#pragma once

#include "req.h"
#include <stdint.h>

#define GDT_NULL 0
#define GDT_CODE 1
#define GDT_DATA 2

#define GDT_CODE_SELECTOR 0x08
#define GDT_DATA_SELECTOR 0x10
#define GDT_UCODE_SELECTOR 0x1B
#define GDT_UDATA_SELECTOR 0x23
#define GDT_TSS_SELECTOR   0x28

#define GDT_UCODE 3
#define GDT_UDATA 4
#define GDT_TSS   5

typedef struct {
    u16 limit_low;
    u16 base_low;
    u8  base_middle;
    u8  access;
    u8  granularity;
    u8  base_high;
} __attribute__((packed)) GDTEntry;

typedef struct{
    u16 limit; // size of GDT - 1
    GDTEntry* address; // ptr to GDT itself
} __attribute__((packed)) GDTPtr;

// the CPU uses this to find the kernel stack
// when an interrupt fires while in ring 3
typedef struct {
    u32 reserved0;
    u64 rsp0;       // kernel stack pointer
    u64 rsp1;
    u64 rsp2;
    u64 reserved1;
    u64 ist1;
    u64 ist2;
    u64 ist3;
    u64 ist4;
    u64 ist5;
    u64 ist6;
    u64 ist7;
    u64 reserved2;
    u16 reserved3;
    u16 iopb_offset;
} __attribute__((packed)) TSS;

// Extern assembly stub to load the GDT
extern void load_gdt(GDTPtr *gdtp);

// Kernel GDT init
void gdt_init(void);

void tss_set_kernel_stack(u64 rsp0);
