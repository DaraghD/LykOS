#include "gdt.h"
#include "drivers/serial.h"

static inline void disable_interrupts(void) { asm volatile("cli"); }

enum { GDT_COUNT = 7 }; // null, kcode, kdata, ucode, udata, tss_lo, tss_hi
static GDTEntry gdt[GDT_COUNT];
static GDTPtr gdt_ptr;
static TSS tss;

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

  // TODO: cleanup with defines
  gdt[GDT_NULL] = make_entry(0, 0, 0, 0);
  gdt[GDT_CODE] = make_entry(0, 0, 0x9A, 0x20);  // kernel code
  gdt[GDT_DATA] = make_entry(0, 0, 0x92, 0x00);  // kernel data
  gdt[GDT_UCODE] = make_entry(0, 0, 0xFA, 0x20); // user code
  gdt[GDT_UDATA] = make_entry(0, 0, 0xF2, 0x00); // user data

  // TSS setup
  memset(&tss, 0, sizeof(tss));
  tss.iopb_offset = sizeof(tss);
  u64 tss_base = (u64)&tss;
  u32 tss_limit = sizeof(tss) - 1;

  gdt[GDT_TSS].limit_low = tss_limit & 0xFFFF;
  gdt[GDT_TSS].base_low = tss_base & 0xFFFF;
  gdt[GDT_TSS].base_middle = (tss_base >> 16) & 0xFF;
  gdt[GDT_TSS].access = 0x89; // present, 64-bit TSS
  gdt[GDT_TSS].granularity = (tss_limit >> 16) & 0x0F;
  gdt[GDT_TSS].base_high = (tss_base >> 24) & 0xFF;

  u32 tss_base_upper = (tss_base >> 32) & 0xFFFFFFFF;
  u32 zero = 0;
  memcpy(&gdt[GDT_TSS + 1], &tss_base_upper, 4);
  memcpy((u8 *)&gdt[GDT_TSS + 1] + 4, &zero, 4);

  gdt_ptr.limit = sizeof(gdt) - 1;
  gdt_ptr.address = &gdt[0];

  load_gdt(&gdt_ptr);

  asm volatile("ltr %0" : : "r"((u16)GDT_TSS_SELECTOR));

  serial_writeln("GDT and TSS loaded");
}

void tss_set_kernel_stack(u64 rsp0) { tss.rsp0 = rsp0; }
