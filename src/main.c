#include "arch/x86_64/gdt.h"
#include "arch/x86_64/idt.h"
#include "arch/x86_64/pic.h"
#include "drivers/ata.h"
#include "drivers/ps2.h"
#include "drivers/serial.h"
#include "graphics/draw.h"
#include "mem/kalloc.h"
#include "mem/mem.h"
#include "req.h"
#include "terminal.h"
#include <cpuid.h>
#include <stdint.h>

static void hcf(void) {
  for (;;) {
    asm("hlt");
  }
}

void kmain(void) {
  gdt_init();
  idt_init();
  pic_remap();
  memmap_init();
  pmm_init();

  graphics_init();
  terminal_init();
  asm volatile("sti");

  alloc_frames(4);

  serial_write_fstring("Allocating 4098 bytes should be 2 pages\n");
  kalloc(4098);

  void *k = kalloc(2000);
  kfree((uint64_t)k);

  uint16_t file_buf[256];
  ata_read_sector(10, file_buf);
  char *bytes = (char *)file_buf;

  serial_writeln("\nREADING FILE : \n");
  for (int i = 0; i < 512; i++) {
    draw_char_term(bytes[i]);
  }

  while (1) {
    keyboard_process();
  }

  hcf();
}
