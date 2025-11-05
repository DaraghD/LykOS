#include "gdt.h"
#include "drivers/serial.h"

static inline void disable_interrupts(void) { asm volatile("cli"); }

enum { GDT_COUNT = 3 };
static GDTEntry gdt[GDT_COUNT];
static GDTPtr gdt_ptr;

static GDTEntry make_entry(u32 base, u32 limit, u8 access, u8 flags) {
  // base and limit ignored in 64-bit;
  GDTEntry e;
  e.limit_low = limit & 0xFFFF;
  e.base_low = base & 0xFFFF;
  e.base_middle = (base >> 16) & 0xFF;
  e.access = access;
  e.granularity = ((limit >> 16) & 0x0F) | (flags & 0xF0);
  e.base_high = (base >> 24) & 0xFF;
  return e;
}

void gdt_init(void) {
  serial_writeln("Initialising GDT");
  disable_interrupts();
  // use macros to make it more clear whats being defined here
  gdt[GDT_NULL] = make_entry(0, 0, 0, 0);
  gdt[GDT_CODE] = make_entry(0, 0, 0x9A, 0x20);
  gdt[GDT_DATA] = make_entry(0, 0, 0x92, 0x00);

  gdt_ptr.limit = sizeof(gdt) - 1;
  gdt_ptr.address = &gdt[0];

  serial_write("Loading GDT");
  load_gdt(&gdt_ptr);
}
