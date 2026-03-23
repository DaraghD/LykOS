#pragma once

#include "proc/task.h"
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
#define PAGE_SIZE FRAME_SIZE
#define ALIGN_UP(x, align) (((x) + (align) - 1) & ~((align) - 1))
// 80GB of memory~
// Should dynamically calculate and store in memory
#define MAX_FRAMES (1024 * 1024 * 20)

// Page table entry flags
#define PTE_PRESENT         (1ULL << 0)
#define PTE_WRITE           (1ULL << 1)
#define PTE_USER            (1ULL << 2)
#define PTE_WRITE_THROUGH   (1ULL << 3)
#define PTE_CACHE_DISABLE   (1ULL << 4)
#define PTE_NX              (1ULL << 63)

// Bit mask to extract the physical addr from a page table entry
// Bits 12-51 hold the addr, rest are flags
#define PTE_ADDR_MASK 0x000FFFFFFFFFF000ULL

extern u64 hhdm_offset;
extern u64 *kernel_pml4;

void print_memmap(void);
void print_paging(void);
void memmap_init(void);
void *pmm_alloc_frame(void);
void pmm_free_frame(void);
void pmm_init(void);
void print_memory_stats(void);
void *alloc_frames(u64 amount);
u64 *create_address_space(void);
void map_page(u64 *pml4, u64 virt_addr, u64 phys_addr, u64 flags);
void switch_address_space(u64 *pml4);
void debug_cr3(void);
void add_vma(Task *t, u64 start, u64 end);
void debug_vmas(Task *t);
u64 find_free_region(Task *t, u64 size);
static inline u64 virt_to_phys(void *virt) { return (u64)virt - hhdm_offset; }

static inline void *phys_to_virt(u64 phys) {
  return (void *)(phys + hhdm_offset);
}

