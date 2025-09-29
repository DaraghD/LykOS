#include "mem.h"
#include "drivers/serial.h"
#include "graphics/draw.h"
#include "vendor/limine.h"
#include <stdbool.h>
#include <stdint.h>

#define KB 1024
#define MB (1024 * KB)
#define GB (1024 * MB)

#define TO_KB(bytes) ((bytes) / KB)
#define TO_MB(bytes) ((bytes) / MB)
#define TO_GB(bytes) ((bytes) / GB)

#define PAGE_SIZE (KB * 4)

__attribute__((
    used,
    section(".limine_requests"))) static volatile struct limine_memmap_request
    limine_memmap_request = {.id = LIMINE_MEMMAP_REQUEST, .revision = 0};

__attribute__((
    used,
    section(
        ".limine_requests"))) static volatile struct limine_paging_mode_request
    limine_paging_mode_request = {.id = LIMINE_PAGING_MODE_REQUEST,
                                  .revision = 0};

static uint64_t total_usable = 0;
static uint64_t reclaimable = 0;
static uint64_t bad = 0;
static uint64_t reserved = 0;
static uint64_t framebuf = 0;
static uint64_t nvs = 0;
static uint64_t exe = 0;
static bool memmap_init = false;
static uint64_t highest_addr = 0;

void init_memmap(void) {
  struct limine_memmap_response *response = limine_memmap_request.response;
  if (!response) {
    serial_writeln("No memory map from Limine!");
    return;
  }

  serial_write_fstring("Memory map entries: {int}\n", response->entry_count);

  for (uint64_t i = 0; i < response->entry_count; i++) {
    struct limine_memmap_entry *entry = response->entries[i];
    if (entry->base > highest_addr)
      highest_addr = entry->base;

    serial_write_fstring(
        "Region {uint}: base={uint} length={uint} type={uint}\n", i,
        entry->base, entry->length, entry->type);

    // draw_fstring("Region {uint}: base={uint} length={uint} type={uint}\n", i,
    //             entry->base, TO_MB(entry->length), entry->type);

    switch (entry->type) {
    case (LIMINE_MEMMAP_USABLE):
      total_usable += entry->length;
      break;
    case (LIMINE_MEMMAP_RESERVED):
      reserved += entry->length;
      break;
    case (LIMINE_MEMMAP_BAD_MEMORY):
      bad += entry->length;
      break;
    case (LIMINE_MEMMAP_FRAMEBUFFER):
      framebuf += entry->length;
      break;
    case (LIMINE_MEMMAP_ACPI_NVS):
      nvs += entry->length;
      break;
    case (LIMINE_MEMMAP_EXECUTABLE_AND_MODULES):
      exe += entry->length;
      break;
    case (LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE):
      reclaimable += entry->length;
      break;
    case (LIMINE_MEMMAP_ACPI_RECLAIMABLE):
      reclaimable += entry->length;
      break;
    }
  }
  memmap_init = true;
}

void print_paging(void) {
  struct limine_paging_mode_response *response =
      limine_paging_mode_request.response;

  draw_fstring("\nPAGING\n");
  draw_fstring("Mode : {uint}\n", response->mode);
  draw_fstring("Revision: {uint}\n", response->revision);
  draw_fstring("Max Mode: {uint}\n", limine_paging_mode_request.max_mode);
  draw_fstring("Min Mode: {uint}\n", limine_paging_mode_request.min_mode);
  draw_fstring("Mode: {uint}\n", limine_paging_mode_request.mode);
}

void print_memmap(void) {
  if (!memmap_init) {
    serial_writeln("Memmap not initialised, can't print");
    return;
  }
  draw_fstring("\n Total usable~      (MB):{uint}\n", TO_MB(total_usable));
  draw_fstring("\n Total reclaimable~ (KB):{uint}\n", TO_KB(reclaimable));
  draw_fstring("\n Total bad~         (KB):{uint}\n", TO_KB(bad));
  draw_fstring("\n Total reserved~    (MB):{uint}\n", TO_MB(reserved));
  draw_fstring("\n Total exe~         (KB):{uint}\n", TO_KB(exe));
  draw_fstring("\n Total framebuffer~ (MB):{uint}\n", TO_MB(framebuf));
  draw_fstring("\n Total nvs?~        (KB):{uint}\n", TO_KB(nvs));
  draw_fstring("\n Bitmap size        (MB):{uint}\n",
               TO_MB(highest_addr / (8 * PAGE_SIZE)));
}

#define FRAME_SIZE (4 * KB)
// supports 4GB of memory
#define MAX_FRAMES (1024 * 1024)

// bitmap will have 1 frame per bit
static uint8_t frame_bitmap[MAX_FRAMES / 8];

static inline void set_bit(uint64_t frame) {
  frame_bitmap[frame / 8] |= (1 << (frame % 8));
}

static inline void clear_bit(uint64_t frame) {
  frame_bitmap[frame / 8] &= ~(1 << (frame % 8));
}

static inline int test_bit(uint64_t frame) {
  return frame_bitmap[frame / 8] & (1 << (frame % 8));
}

void mark_frames_free(uint64_t start, uint64_t count) {
  for (uint64_t i = 0; i < count; i++) {
    clear_bit(start + i); // 0 = free
  }
}

void mark_frames_used(uint64_t start, uint64_t count) {
  for (uint64_t i = 0; i < count; i++) {
    set_bit(start + i); // 1 = used
  }
}
void pmm_init(struct limine_memmap_response *response) {
  for (uint64_t i = 0; i < response->entry_count; i++) {
    struct limine_memmap_entry *entry = response->entries[i];
    if (entry->type == LIMINE_MEMMAP_USABLE) {
      uint64_t start = entry->base / FRAME_SIZE;
      uint64_t count = entry->length / FRAME_SIZE;
      mark_frames_free(start, count);
    }
  }
}

int64_t find_first_free_frame(void) {
  for (int64_t i = 0; i < MAX_FRAMES / 8; i++) {
    if (frame_bitmap[i] != 0xFF) { // not full
      for (int b = 0; b < 8; b++) {
        if (!(frame_bitmap[i] & (1 << b))) {
          return i * 8 + b;
        }
      }
    }
  }
  return -1; // none
}

// Can return NULL
void *pmm_alloc_frame(void) {
  int64_t idx = find_first_free_frame();
  if (idx == -1)
    return NULL;
  set_bit(idx);
  return (void *)(idx * FRAME_SIZE);
}

void *alloc_heap(void) {
  void *heap = pmm_alloc_frame();
  uint64_t *my_num = (uint64_t *)heap;
  *my_num = 100;

  return my_num;
}
