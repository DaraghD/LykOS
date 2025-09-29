#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef struct {
  uint64_t len;
  void* addr;
} mem_block;

void print_memmap(void);
void print_paging(void);
void init_memmap(void);
void* alloc_heap(void);
