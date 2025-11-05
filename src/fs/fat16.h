#pragma once
#include "klib/kstring.h"
#include "req.h"
#include <stdint.h>

#define ATTR_DIRECTORY 0x10
// #pragma pack(push, 1)
typedef struct {
  u8 jump[3];
  u8 oem[8];
  u16 bytes_per_sector;
  u8 sectors_per_cluster;
  u16 reserved_sectors;
  u8 fat_count;
  u16 root_entries;
  u16 total_sectors_short;
  u8 media;
  u16 fat_size_sectors;
  u16 sectors_per_track;
  u16 heads;
  u32 hidden_sectors;
  u32 total_sectors_long;
  // FAT16-specific
  u8 drive_number;
  u8 reserved1;
  u8 boot_signature;
  u32 volume_id;
  u8 volume_label[11];
  u8 fs_type[8];
} __attribute__((packed)) fat16_bpb_t;

typedef struct {
  u8 name[8];
  u8 ext[3];
  u8 attr;
  u8 reserved;
  u8 create_time_tenths;
  u16 create_time;
  u16 create_date;
  u16 last_access_date;
  u16 ignore;
  u16 last_write_time;
  u16 last_write_date;
  u16 first_cluster;
  u32 file_size;
} __attribute__((packed)) fat16_dir_entry_t;
// #pragma pack(pop)

typedef struct {
  fat16_dir_entry_t *entries;
  u32 count;
} dir_listing_t;

void fat16_init(void);
u16 fat16_get_next_cluster(u16 cluster);
u32 fat16_cluster_to_lba(u16 cluster);
void fat16_read_file(fat16_dir_entry_t *entry, u8 *buffer);
fat16_dir_entry_t *fat16_find_file(const char *name);
u32 fat16_get_entries(fat16_dir_entry_t* files);
u32 fat16_get_entries_at_entry(fat16_dir_entry_t location, fat16_dir_entry_t* files);
bool fat16_list_path(kstring *path, dir_listing_t* out_listing);
void fat16_format83_name(const char*name, char formatted[11]);

