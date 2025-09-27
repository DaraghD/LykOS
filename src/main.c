#include "arch/x86_64/gdt.h"
#include "arch/x86_64/idt.h"
#include "arch/x86_64/pic.h"
#include "drivers/ps2.h"
#include "graphics/draw.h"
#include "terminal.h"
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <cpuid.h>

static void hcf(void) {
  for (;;) {
    asm("hlt");
  }
}
#define CPUID_FEAT_EDX_APIC (1 << 9)

void kmain(void) {
  init_graphics();
  gdt_init();
  idt_init();
  pic_remap();
  asm volatile("sti");
  // infinite_rainbow(get_framebuffer());
  // debug_graphics();
  terminal_init();
  while (1) {
    keyboard_process();
  }
  hcf();
}
