#include "kalloc.h"
#include "drivers/serial.h"
#include "mem.h"
#include "req.h"
#include "terminal.h"
#include <stdint.h>
#include <stdlib.h>
#include <strings.h>

#define FREELIST_COALESCE_SIZE 100

typedef struct mem_header {
  uint64_t size;
  struct mem_header *next;
  struct mem_header *prev;
} mem_header;

static mem_header *free_list_head = NULL;
static uint64_t freelist_count = 0;

static uint64_t debug_freelist_size = 0;
static uint64_t debug_total_allocated = 0;

// returns NULL if no suitable locations
// returns amount of unused bytes in *unused_out
static void *freelist_alloc(uint64_t size, uint64_t *unused_out) {
  mem_header *current = free_list_head;
  while (current) {
    // first fit approach- TODO: look into other approaches
    if (current->size >= size) {
      void *addr = (void *)current;

      if (current->prev)
        current->prev->next = current->next;
      else
        free_list_head = current->next;

      if (current->next)
        current->next->prev = current->prev;

      serial_write_fstring(
          "Allocated from freelist requested {uint}, got {uint}\n", size,
          current->size);
      *unused_out = current->size - size;
      freelist_count--;
      debug_freelist_size -= current->size - size;
      memset(addr, 0xAA, size);
      return addr;
    }
    current = current->next;
  }

  serial_write_fstring("Failed to allocate from freelist requested {uint}\n",
                       size);
  return NULL;
}

static void add_to_freelist(uint64_t addr, uint64_t size) {
  if (size <= sizeof(mem_header)) {
    return;
  }
  mem_header *hdr = (mem_header *)addr;
  // size doesnt subtract mem_header as the metadata is no longer needed when
  // allocating
  hdr->size = size;
  hdr->next = free_list_head;
  hdr->prev = NULL;

  if (free_list_head)
    free_list_head->prev = hdr;

  free_list_head = hdr;
  serial_write_fstring(
      "Added to freelist block: block_addr={hex}, size={uint}\n",
      (uint64_t)hdr, hdr->size);
  debug_freelist_size += hdr->size;
  freelist_count++;
}

// returns NULL if allocation fails
void *kalloc(uint64_t size) {
  serial_write_fstring("Called with size {uint}\n", size);
  // allocate size + a u64 to keep track of size of the allocation
  uint64_t total = size + sizeof(uint64_t);

  void *mem = NULL;
  uint64_t unused = 0;

  mem = freelist_alloc(total, &unused);

  if (mem == NULL) {
    uint64_t frames = (total + FRAME_SIZE - 1) / FRAME_SIZE;
    unused = frames * FRAME_SIZE - total;
    mem = alloc_frames(frames);
  }

  if (mem == NULL) {
    serial_write_fstring("Allocating {uint} failed from freelist and pages\n",
                         total);
    return NULL;
  }
  uint64_t unused_addr = (uint64_t)mem + total;

  serial_write_fstring("\n UNUSED MEMORY FROM PAGES : {uint}\n", unused);
  serial_write_fstring("Allocation starts at {hex}\n", (uint64_t)mem);
  serial_write_fstring("Unused starts at {hex}\n", (uint64_t)unused_addr);

  // add unused memory to the freelist
  add_to_freelist(unused_addr, unused);

  // TODO: coalesce freelist
  if (freelist_count >= FREELIST_COALESCE_SIZE) {
  }

  debug_total_allocated += size;
  // store size at start of mem
  *(uint64_t *)mem = size;
  return (void *)((uint64_t)mem + sizeof(uint64_t));
}

void kfree(uint64_t ptr) {
  // read the size placed by kmalloc before the ptr
  uint64_t size2 = ((uint64_t *)ptr)[-1];
  uint64_t *size_addr = (uint64_t *)(ptr - sizeof(uint64_t));
  uint64_t size = *size_addr;
  serial_write_fstring("KFree called with {uint}, size of allocation {uint}\n",
                       ptr, size);
  serial_write_fstring("size2={uint} size={uint}", size2, size);

  add_to_freelist((uint64_t)size_addr, size + sizeof(uint64_t));
  debug_total_allocated -= size;
}

// debug

void debug_freelist(void) {
  serial_write_fstring("\n FREELIST DEBUG \n");
  int index = 0;
  mem_header *current = free_list_head;
  while (current) {
    serial_write_fstring("IDX={uint} SIZE={uint} ADDR={hex}\n", index,
                         current->size, current);
    terminal_fstring("\nIDX={uint} SIZE={uint} ADDR={hex}\n", index,
                     current->size, current);
    current = current->next;
    index++;
  }

  terminal_fstring("Items in freelist {uint}, size of freelist KB :{uint}\n",
                   freelist_count, TO_KB(debug_freelist_size));
  serial_write_fstring(
      "Items in freelist {uint}, size of freelist KB :{uint}\n", freelist_count,
      TO_KB(debug_freelist_size));
}

void print_allocation_stats(void) {
  terminal_fstring("Total memory allocated :(KB) {uint}\n",
                   TO_KB(debug_total_allocated));

  serial_write_fstring("Total memory allocated :(KB) {uint}\n",
                       TO_KB(debug_total_allocated));
}
