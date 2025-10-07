#pragma once
#include <stdint.h>

void kfree(uint64_t ptr);
void *kalloc(uint64_t size);
void debug_freelist(void);
