#pragma once
#include "klib/kstring.h"
#include "mem/arena.h"
#include <stdint.h>

#define ATTR_DIRECTORY 0x10
// #pragma pack(push, 1)
typedef struct {
  uint8_t jump[3];
  uint8_t oem[8];
  uint16_t bytes_per_sector;
  uint8_t sectors_per_cluster;
  uint16_t reserved_sectors;
  uint8_t fat_count;
  uint16_t root_entries;
  uint16_t total_sectors_short;
  uint8_t media;
  uint16_t fat_size_sectors;
  uint16_t sectors_per_track;
  uint16_t heads;
  uint32_t hidden_sectors;
  uint32_t total_sectors_long;
  // FAT16-specific
  uint8_t drive_number;
  uint8_t reserved1;
  uint8_t boot_signature;
  uint32_t volume_id;
  uint8_t volume_label[11];
  uint8_t fs_type[8];
} __attribute__((packed)) fat16_bpb_t;

typedef struct {
  uint8_t name[8];
  uint8_t ext[3];
  uint8_t attr;
  uint8_t reserved;
  uint8_t create_time_tenths;
  uint16_t create_time;
  uint16_t create_date;
  uint16_t last_access_date;
  uint16_t ignore;
  uint16_t last_write_time;
  uint16_t last_write_date;
  uint16_t first_cluster;
  uint32_t file_size;
} __attribute__((packed)) fat16_dir_entry_t;
// #pragma pack(pop)

typedef struct {
  fat16_dir_entry_t *entries;
  uint32_t count;
} dir_listing_t;

void fat16_init(void);
uint16_t fat16_get_next_cluster(uint16_t cluster);
uint32_t fat16_cluster_to_lba(uint16_t cluster);
void fat16_read_file(fat16_dir_entry_t *entry, uint8_t *buffer);
fat16_dir_entry_t *fat16_find_file(const char *name);
uint32_t fat16_get_entries(fat16_dir_entry_t* files);
void free_dir_listing(dir_listing_t *listing);
bool fat16_list_path(kstring *path, dir_listing_t* out_listing);
void fat16_format83_name(const char*name, char formatted[11]);

