#include "mem.h"
#include "drivers/serial.h"
#include "mem/kalloc.h"
#include "req.h"
#include "terminal.h"
#include "vendor/limine.h"
#include <stdbool.h>
#include <stdint.h>

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
__attribute__((
    used,
    section(".limine_requests"))) static volatile struct limine_hhdm_request
    limine_hhdm_request = {.id = LIMINE_HHDM_REQUEST, .revision = 0};

static u64 total_usable = 0;
static u64 reclaimable = 0;
static u64 bad = 0;
static u64 reserved = 0;
static u64 framebuf = 0;
static u64 nvs = 0;
static u64 exe = 0;
static u64 highest_addr = 0;

u64 hhdm_offset;

void memmap_init(void) {
  struct limine_memmap_response *response = limine_memmap_request.response;
  if (!response) {
    serial_writeln("No memory map from Limine!");
    return;
  }

  hhdm_offset = limine_hhdm_request.response->offset;
  serial_write_fstring("Memory map entries: {int}\n", response->entry_count);

  for (u64 i = 0; i < response->entry_count; i++) {
    struct limine_memmap_entry *entry = response->entries[i];
    if (entry->base > highest_addr)
      highest_addr = entry->base;

    char *is_usable = "True";
    if (entry->type != LIMINE_MEMMAP_USABLE)
      is_usable = "False";

    serial_write_fstring(
        "Region {uint}: base={uint} length={uint} usable={str}\n", i,
        entry->base, entry->length, is_usable);

    // draw_fstring("Region {uint}: base={uint} length={uint} type={uint}\n", i,
    // entry->base, TO_MB(entry->length), entry->type);

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
}

// bitmap will have 1 frame per bit
static u8 frame_bitmap[MAX_FRAMES / 8];

static inline void set_bit(u64 frame) {
  frame_bitmap[frame / 8] |= (1 << (frame % 8));
}

static inline void clear_bit(u64 frame) {
  frame_bitmap[frame / 8] &= ~(1 << (frame % 8));
}

static inline int test_bit(u64 frame) {
  return frame_bitmap[frame / 8] & (1 << (frame % 8));
}

void mark_frames_free(u64 start, u64 count) {
  for (u64 i = 0; i < count; i++) {
    clear_bit(start + i); // 0 = free
  }
}

void mark_frames_used(u64 start, u64 count) {
  for (u64 i = 0; i < count; i++) {
    set_bit(start + i); // 1 = used
  }
}

void pmm_init(void) {
  memset(frame_bitmap, 0xFF, sizeof(frame_bitmap));
  struct limine_memmap_response *response = limine_memmap_request.response;
  for (u64 i = 0; i < response->entry_count; i++) {
    struct limine_memmap_entry *entry = response->entries[i];
    if (entry->type == LIMINE_MEMMAP_USABLE) {
      serial_write_fstring("Entry is usable at base {uint} \n", entry->base);
      u64 start = entry->base / FRAME_SIZE;
      u64 count = entry->length / FRAME_SIZE;
      serial_write_fstring("Marking {uint} frames free at {uint}\n", count,
                           start);
      mark_frames_free(start, count);
    }
  }
}

int64_t find_first_free_frame(void) {
  for (int64_t i = 1; i < MAX_FRAMES / 8; i++) { // avoid first frame
    if (frame_bitmap[i] != 0xFF) {               // has to have one non zero bit
      for (int b = 0; b < 8; b++) {
        if (!(frame_bitmap[i] & (1 << b))) {
          return i * 8 + b;
        }
      }
    }
  }
  return -1; // none
}

// can null
void *alloc_frames(u64 frame_amount) {
  u64 avail_contigous_frames = 0;
  u64 start_idx = 0;
  for (int64_t i = 1; i < MAX_FRAMES / 8; i++) { // avoid first frame
    // TODO:possible optimisation here using amount sizes
    if (frame_bitmap[i] != 0xFF) { // has to have one non zero bit
      for (int b = 0; b < 8; b++) {
        if (!(frame_bitmap[i] & (1 << b))) {
          // we have found a free frame
          if (start_idx == 0)
            start_idx = i * 8 + b;
          avail_contigous_frames++;

          if (avail_contigous_frames == frame_amount) {
            mark_frames_used(start_idx, frame_amount);
            serial_write_fstring("Allocating idx:{uint}, frames {uint}\n",
                                 start_idx, avail_contigous_frames);
            return (void *)((start_idx * FRAME_SIZE) +
                            limine_hhdm_request.response->offset);
          }
        } else { // frame isnt free, reset counters
          avail_contigous_frames = 0;
          start_idx = 0;
        }
      }
    }
  }
  return NULL;
}

// can null
void *pmm_alloc_frame(void) {
  int64_t idx = find_first_free_frame();
  serial_write_fstring("Allocating frame {uint}\n", idx);
  serial_write_fstring(
      "Frame base: {uint}\n Frame virt: {uint}\n", idx * FRAME_SIZE,
      (idx * FRAME_SIZE) + limine_hhdm_request.response->offset);

  if (idx == -1)
    return NULL;
  set_bit(idx);
  return (void *)((idx * FRAME_SIZE) + limine_hhdm_request.response->offset);
}

void pmm_free_frame(void) { return; }

//
// debug
//
void print_memory_stats(void) {
  u64 free_frames = 0;
  for (int64_t i = 1; i < MAX_FRAMES / 8; i++) { // avoid first frame
    if (frame_bitmap[i] != 0xFF) {
      for (int b = 0; b < 8; b++) {
        if (!(frame_bitmap[i] & (1 << b))) {
          free_frames += 1;
        }
      }
    }
  }
  u64 free_frames_MB = TO_MB(free_frames * FRAME_SIZE);
  terminal_fstring("Free memory (MB): {uint}\n", free_frames_MB);
  terminal_fstring("Free pages: {uint}\n", free_frames);
  print_allocation_stats();
}

void print_paging(void) {
  struct limine_paging_mode_response *response =
      limine_paging_mode_request.response;

  terminal_fstring("Kernel offset {uint}",
                   limine_hhdm_request.response->offset);
  terminal_fstring("Kernel offset {hex}", limine_hhdm_request.response->offset);
  terminal_fstring("\nPAGING\n");
  terminal_fstring("Mode : {uint}\n", response->mode);
  terminal_fstring("Revision: {uint}\n", response->revision);
  terminal_fstring("Max Mode: {uint}\n", limine_paging_mode_request.max_mode);
  terminal_fstring("Min Mode: {uint}\n", limine_paging_mode_request.min_mode);
  terminal_fstring("Mode: {uint}\n", limine_paging_mode_request.mode);
}

void print_memmap(void) {
  terminal_fstring("\n Total usable~      (MB):{uint}\n", TO_MB(total_usable));
  terminal_fstring("\n Total reclaimable~ (KB):{uint}\n", TO_KB(reclaimable));
  terminal_fstring("\n Total bad~         (KB):{uint}\n", TO_KB(bad));
  terminal_fstring("\n Total reserved~    (MB):{uint}\n", TO_MB(reserved));
  terminal_fstring("\n Total exe~         (KB):{uint}\n", TO_KB(exe));
  terminal_fstring("\n Total framebuffer~ (MB):{uint}\n", TO_MB(framebuf));
  terminal_fstring("\n Total nvs?~        (KB):{uint}\n", TO_KB(nvs));
  terminal_fstring("\n Bitmap size        (MB):{uint}\n",
                   TO_MB(highest_addr / (8 * FRAME_SIZE)));
}
