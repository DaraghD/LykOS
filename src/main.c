#include "arch/x86_64/gdt.h"
#include "arch/x86_64/idt.h"
#include "arch/x86_64/pic.h"
#include "drivers/ps2.h"
#include "graphics/draw.h"
#include "mem/mem.h"
#include "terminal.h"
#include <stdint.h>

static void hcf(void) {
  for (;;) {
    asm("hlt");
  }
}
#define CPUID_FEAT_EDX_APIC (1 << 9)

static int rotation = 0;
static uint64_t tick = 0;

static void xddd(void) {
  if ((tick % 10000000) == 0) {
    tick = 0;
    char frames[] = {'|', '/', '-', '\\'};
    int frame = frames[rotation % 4];

    fill_char(get_framebuffer()->width - (8 * m_scale), 0);
    draw_char(frame, get_framebuffer()->width - (8 * m_scale), 0, WHITE);
    rotation++;
    if (rotation > 3)
      rotation = 0;
  }
  tick++;
}

void kmain(void) {
  init_graphics();
  gdt_init();
  idt_init();
  pic_remap();
  init_memmap();
  asm volatile("sti");
  terminal_init();
  uint64_t *my_num = alloc_heap();
  draw_fstring("My num: {uint}", *my_num);
  // print_memmap();
  while (1) {
    keyboard_process();
    xddd();
  }
  hcf();
}
