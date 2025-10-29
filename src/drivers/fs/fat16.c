#include "fat16.h"
#include "drivers/ata.h"
#include "drivers/serial.h"
#include "mem/arena.h"
#include "mem/kalloc.h"
#include "req.h"
#include <stdint.h>

static fat16_bpb_t bpb;
static fat16_dir_entry_t root_dir[512];
static uint8_t *fat_table;
static uint32_t root_start;
static uint32_t root_sectors;

void fat16_init(void) {
  // read BPB from sector 0
  uint8_t sector[512];
  ata_read_sector(0, sector);
  memcpy(&bpb, sector, sizeof(fat16_bpb_t));

  uint32_t fat_size_bytes = bpb.fat_size_sectors * bpb.bytes_per_sector;
  serial_write_fstring("Allocating {uint} bytes for fat", fat_size_bytes);
  fat_table = kalloc(fat_size_bytes);

  // read fat1 into fat_table
  for (uint32_t i = 0; i < bpb.fat_size_sectors; i++)
    ata_read_sector(bpb.reserved_sectors + i,
                    fat_table + i * bpb.bytes_per_sector);

  // read root directory
  root_start = bpb.reserved_sectors + bpb.fat_count * bpb.fat_size_sectors;
  root_sectors = (bpb.root_entries * 32 + (bpb.bytes_per_sector - 1)) /
                 bpb.bytes_per_sector;
  for (uint32_t i = 0; i < root_sectors; i++) {
    ata_read_sector(root_start + i, sector);
    for (uint32_t j = 0; j < bpb.bytes_per_sector / 32; j++) {
      uint32_t entry_index = i * (bpb.bytes_per_sector / 32) + j;
      if (entry_index >= bpb.root_entries)
        break;
      memcpy(&root_dir[entry_index], sector + j * 32, 32);
    }
  }
  serial_write_fstring("Reserved: {uint}\n", bpb.reserved_sectors);
  serial_write_fstring("FAT count: {uint}\n", bpb.fat_count);
  serial_write_fstring("FAT size: {uint}\n", bpb.fat_size_sectors);
  serial_write_fstring("Root entries: {uint}\n", bpb.root_entries);
  serial_write_fstring("Total sectors: {uint}\n", bpb.total_sectors_short);
  serial_write_fstring("Bytes per sector: {uint}\n", bpb.bytes_per_sector);
  serial_write_fstring("Sectors per cluster: {uint}\n",
                       bpb.sectors_per_cluster);
}

uint32_t fat16_cluster_to_lba(uint16_t cluster) {
  uint32_t first_data_sector =
      bpb.reserved_sectors + (bpb.fat_count * bpb.fat_size_sectors) +
      ((bpb.root_entries * 32 + (bpb.bytes_per_sector - 1)) /
       bpb.bytes_per_sector);
  return first_data_sector + (cluster - 2) * bpb.sectors_per_cluster;
}

// buffer must be big enough for entry->file_size
// TODO: should buffer be char or uint8_t?
void fat16_read_file(fat16_dir_entry_t *entry, uint8_t *buffer) {

  serial_write_fstring("About to read file:\n");
  serial_write_fstring("  First cluster: {uint}\n", entry->first_cluster);
  serial_write_fstring("  File size: {uint}\n", entry->file_size);
  serial_write_fstring("  Name: {str}\n", entry->name);
  serial_write_fstring("  Calculated LBA: {uint}\n",
                       fat16_cluster_to_lba(entry->first_cluster));

  uint16_t cluster = entry->first_cluster;
  uint32_t bytes_left = entry->file_size;
  uint8_t sector_buffer[512];

  while (cluster < 0xFFF8 && bytes_left > 0) {
    uint32_t lba = fat16_cluster_to_lba(cluster);

    // read ALL sectors in this cluster
    for (uint32_t i = 0; i < bpb.sectors_per_cluster && bytes_left > 0; i++) {
      ata_read_sector(lba + i, sector_buffer);
      // serial_write_fstring(
      // "Read LBA {uint}, first bytes: {uint} {uint} {uint} {uint}\n",
      // lba + i, sector_buffer[0], sector_buffer[1], sector_buffer[2],
      // sector_buffer[3]);

      uint32_t copy_size = (bytes_left > 512) ? 512 : bytes_left;
      memcpy(buffer, sector_buffer, copy_size);
      buffer += copy_size;
      bytes_left -= copy_size;
    }

    // Get next cluster from FAT
    cluster = *((uint16_t *)(fat_table + cluster * 2));
  }
}

// doesnt check directories / just root
fat16_dir_entry_t *fat16_find_file(const char *name) {
  char formatted_name[11];
  fat16_format83_name(name, formatted_name);
  for (uint32_t i = 0; i < bpb.root_entries; i++) {
    if (root_dir[i].name[0] == 0x00)
      continue; // empty entry
    if (root_dir[i].name[0] == 0xE5)
      continue;
    if ((root_dir[i].attr & 0x0F) == 0x0F)
      continue; // skip long names
    if (memcmp(root_dir[i].name, formatted_name, 11) == 0)
      return &root_dir[i];
  }
  serial_writeln("FILE NOT FOUND");
  return NULL; // file not found
}

void fat16_format83_name(const char *name, char formatted[11]) {
  for (int i = 0; i < 11; i++)
    formatted[i] = ' ';

  // find dot
  const char *dot = NULL;
  for (int i = 0; name[i]; i++) {
    if (name[i] == '.')
      dot = &name[i];
  }

  int name_len = dot ? (dot - name) : strlen(name);
  int ext_len = dot ? strlen(dot + 1) : 0;

  for (int i = 0; i < name_len && i < 8; i++) {
    char c = name[i];
    if (c >= 'a' && c <= 'z')
      c -= ('a' - 'A');
    formatted[i] = c;
  }

  for (int i = 0; i < ext_len && i < 3; i++) {
    char c = dot[1 + i];
    if (c >= 'a' && c <= 'z')
      c -= ('a' - 'A');
    formatted[8 + i] = c;
  }
}

uint32_t fat16_get_entries(fat16_dir_entry_t *files) {
  uint32_t count = 0;
  serial_write_fstring("bpb entries root{uint}\n", bpb.root_entries);
  for (uint32_t i = 0; i < bpb.root_entries; i++) {
    if (root_dir[i].name[0] == 0x00)
      break;  // end if (root_dir[i].attr == 0x0F)
    continue; // skip LFN
    serial_write_fstring("Entry {str}\n", root_dir[i].name);
  }

  for (int j = 0; j < 512; j += 1) {
    serial_write_fstring("Reading root dir at {uint}", j);
    fat16_dir_entry_t *entry = &root_dir[j];
    if (entry->name[0] == '\0')
      break; // end of entries
    if (entry->name[0] == 0xE5)
      continue; // deleted
    if (entry->attr == 0x0F) {
      continue; // long filename
    }
    files[count++] = *entry;

    char name[9], ext[4];
    memcpy(name, entry->name, 8);
    memcpy(ext, entry->ext, 3);
    name[8] = '\0';
    ext[3] = '\0';
    for (int k = 7; k >= 0 && name[k] == ' '; k--)
      name[k] = '\0';
    for (int k = 2; k >= 0 && ext[k] == ' '; k--)
      ext[k] = '\0';

    serial_write_fstring("{str}.{str}\n", name, ext);
  }
  return count;
}

// absolute path starting with /
void create_file(const char *name, const char *path) {
  char fat_83_name[11];
  fat16_format83_name(name, fat_83_name);
}
// VFS Implementation
// file_ fat16_open_file(const char *file_path, enum OPEN_MODE) {
//   // create file on disk at file_path
//   file_ f;
//   return f;
// }
//
// uint8_t fat16_close_file(const char *file_path) {
//   // create file on disk at file_path
//   return 1;
// }
// file_system fat16_filesystem(void) {
//   file_system fs;
//   fs.open = &fat16_open_file;
//   fs.close = &fat16_close_file;
//   return fs;
// }
