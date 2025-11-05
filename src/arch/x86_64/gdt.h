#pragma once

#include "req.h"
#include <stdint.h>

#define GDT_NULL 0
#define GDT_CODE 1
#define GDT_DATA 2

#define GDT_CODE_SELECTOR 0x08
#define GDT_DATA_SELECTOR 0x10

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


// Extern assembly stub to load the GDT
extern void load_gdt(GDTPtr *gdtp);

// Kernel GDT init
void gdt_init(void);
