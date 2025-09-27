#pragma once

#include <stdint.h>

#define GDT_NULL 0
#define GDT_CODE 1
#define GDT_DATA 2

#define GDT_CODE_SELECTOR 0x08
#define GDT_DATA_SELECTOR 0x10

typedef struct {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t  base_middle;
    uint8_t  access;
    uint8_t  granularity;
    uint8_t  base_high;
} __attribute__((packed)) GDTEntry;

typedef struct{
    uint16_t limit; // size of GDT - 1
    GDTEntry* address; // ptr to GDT itself
} __attribute__((packed)) GDTPtr;


// Extern assembly stub to load the GDT
extern void load_gdt(GDTPtr *gdtp);

// Kernel GDT init
void gdt_init(void);
