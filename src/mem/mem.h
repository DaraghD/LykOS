#pragma once

#include "req.h"
#include <stdbool.h>
#include <stdint.h>

#define KB 1024
#define MB (1024 * KB)
#define GB (1024 * MB)

#define TO_KB(bytes) ((bytes) / KB)
#define TO_MB(bytes) ((bytes) / MB)
#define TO_GB(bytes) ((bytes) / GB)

#define FRAME_SIZE (4 * KB)
// 80GB of memory~
// Should dynamically calculate and store in memory
#define MAX_FRAMES (1024 * 1024 * 20)
extern u64 hhdm_offset;

void print_memmap(void);
void print_paging(void);
void memmap_init(void);
void *pmm_alloc_frame(void);
void pmm_free_frame(void);
void pmm_init(void);
void print_memory_stats(void);
void *alloc_frames(u64 amount);
