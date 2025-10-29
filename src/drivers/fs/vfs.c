#include "vfs.h"
#include "drivers/fs/fat16.h"
#include "drivers/serial.h"
#include "klib/kstring.h"
#include "mem/kalloc.h"
#include "terminal.h"
#include <stdint.h>

static void normalise(const uint8_t old[11], char new[13]) {
  int pos = 0;
  for (int i = 0; i < 8; i++) {
    if (old[i] == ' ')
      break;
    new[pos++] = old[i];
  }

  if (old[8] != ' ') {
    new[pos++] = '.';
    for (int i = 8; i < 11; i++) {
      if (old[i] == ' ')
        break;
      new[pos++] = old[i];
    }
  }

  new[pos] = '\0';
}

void list_files(kstring *path) {
  fat16_dir_entry_t *files = kalloc(100 * sizeof(fat16_dir_entry_t));
  uint32_t file_count = fat16_get_entries(files);
  if (file_count < 1)
    terminal_fstring("\nNo files found!");
  for (uint32_t i = 0; i < file_count; i++) {
    char name[13];
    normalise(files[i].name, name);
    if (files[i].attr & 0x10) {
      terminal_fstring("DIR:  {str}\n", name);
    } else {
      terminal_fstring("FILE: {str}\n", name);
    }
  }
  kfree((uint64_t)files);
  return;
}

// command is the full command not just the file path
void cat_file(kstring *path) {
  serial_write_fstring("\ntrying to cat: {kstr}\n", path);
  fat16_dir_entry_t *files = kalloc(100 * sizeof(fat16_dir_entry_t));
  uint32_t file_count = fat16_get_entries(files);

  if (file_count < 1)
    terminal_fstring("No file {kstr} found\n", path);

  for (uint32_t i = 0; i < file_count; i++) {
    fat16_dir_entry_t file = files[i];
    bool is_file = !(file.attr & 0x10);
    char file_name[13];

    normalise(file.name, file_name);
    if (is_file && (strncmp(file_name, path->buf, path->len) == 0)) {
      terminal_fstring("File found! :{str}\n", file_name);
      char *file_buf = kalloc(file.file_size);
      fat16_read_file(&file, (uint8_t *)file_buf);

      for (uint32_t i = 0; i < file.file_size; i++) {
        terminal_fstring("{char}", file_buf[i]);
      }
      kfree((uint64_t)file_buf);
    }
  }
  kfree((uint64_t)files);
}
