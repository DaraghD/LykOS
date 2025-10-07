#include "kalloc.h"
#include "drivers/serial.h"
#include "mem.h"
#include "terminal.h"
#include <stdint.h>
#include <stdlib.h>
#include <strings.h>

#define FREELIST_COALESCE_SIZE 100

typedef struct mem_header {
  uint64_t size;
  uint64_t addr;
  struct mem_header *next;
  struct mem_header *prev;
} mem_header;

static mem_header *free_list_head = NULL;
static uint64_t freelist_size = 0;

// returns NULL if no suitable locations
// returns amount of unused bytes in *unused_out
static void *freelist_alloc(uint64_t size, uint64_t *unused_out) {
  mem_header *current = free_list_head;
  while (current) {
    // first fit approach- TODO: look into other approaches
    if (current->size >= size) {
      void *addr = (void *)current->addr;

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
      freelist_size--;
      return addr;
    }
    current = current->next;
  }

  serial_write_fstring("Failed to allocate from freelist requested {uint}\n",
                       size);
  return NULL;
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
  serial_write_fstring("Allocation starts at {uint}\n", (uint64_t)mem);
  serial_write_fstring("Unused starts at {uint}\n", (uint64_t)unused_addr);

  // add unused memory to the freelist
  if (unused >= sizeof(mem_header)) {
    mem_header *hdr = (mem_header *)unused_addr;
    hdr->size = unused - sizeof(mem_header);
    hdr->addr = unused_addr + sizeof(mem_header);
    hdr->next = free_list_head;
    hdr->prev = NULL;

    if (free_list_head)
      free_list_head->prev = hdr;

    free_list_head = hdr;

    serial_write_fstring("Added to freelist block: addr={uint}, size={uint}\n",
                         hdr->addr, hdr->size);
    freelist_size++;
  }

  // TODO: coalesce freelist
  if (freelist_size >= FREELIST_COALESCE_SIZE) {
  }

  // store size at start of mem
  *(uint64_t *)mem = size;
  return (void *)((uint64_t)mem + sizeof(uint64_t));
}

void kfree(uint64_t ptr) {
  // read the size placed by kmalloc before the ptr
  uint64_t size = *(uint64_t *)(ptr - sizeof(uint64_t));
  serial_write_fstring("KFree called with {uint}, size of allocation {uint}\n",
                       ptr, size);
  // TODO: add back to freelist
}

void debug_freelist(void) {
  serial_write_fstring("\n FREELIST DEBUG \n");
  int index = 0;
  mem_header *current = free_list_head;
  while (current) {
    serial_write_fstring("IDX={uint} SIZE={uint} ADDR={uint}\n", index,
                         current->size, current->addr);
    terminal_fstring("\nIDX={uint} SIZE={uint} ADDR={uint}\n", index,
                     current->size, current->addr);
    current = current->next;
    index++;
  }
}
