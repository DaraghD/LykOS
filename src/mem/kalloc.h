#pragma once
#include <stddef.h>
#include <stdint.h>

void kfree(uint64_t ptr);
void *kalloc(uint64_t size);
void *krealloc(void *ptr, size_t size);
void debug_freelist(void);
void print_allocation_stats(void);
