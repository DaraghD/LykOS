#include "arch/x86_64/gdt.h"
#include "arch/x86_64/idt.h"
#include "arch/x86_64/pic.h"
#include "drivers/ps2.h"
#include "drivers/serial.h"
#include "graphics/draw.h"
#include "mem/kalloc.h"
#include "mem/mem.h"
#include "terminal.h"
#include <cpuid.h>
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

    fill_char(get_framebuffer()->width - (8 * g_scale), 0);
    draw_char(frame, get_framebuffer()->width - (8 * g_scale), 0, WHITE);
    rotation++;
    if (rotation > 3)
      rotation = 0;
  }
  tick++;
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

  void *vp = pmm_alloc_frame();
  *(uint64_t *)vp = 10;

  alloc_frames(4);
  serial_write_fstring("Allocating 4098 bytes should be 2 pages\n");
  kalloc(4098);

  serial_write_fstring("Allocating 4088 bytes + header should be 1 page\n");

  kalloc(4088);
  void *k = kalloc(2000);
  kfree((uint64_t)k);
  kalloc(1000);
  void *v = kalloc(9000);

  kfree((uint64_t)v);

  while (1) {
    keyboard_process();
  }

  hcf();
}
