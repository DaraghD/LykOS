#include "arch/x86_64/gdt.h"
#include "arch/x86_64/idt.h"
#include "drivers/serial.h"
#include "graphics/draw.h"
#include "klib/kstring.h"
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

static void hcf(void) {
  for (;;) {
    asm("hlt");
  }
}

void kmain(void) {
  init_graphics();
  limine_framebuffer *framebuffer = get_framebuffer();

  gdt_init();
  asm volatile("sti");
  idt_init();
  uint32_t center = framebuffer->width / 2;
  center -= 100;

  set_draw_scale(1);
  draw_string("Hello Mars, from LykOS", center, 0, BLUE);
  char height_buf[128];
  kstring height_str = KSTRING(height_buf, 128);
  APPEND_STRL(&height_str, "HEIGHT: ", framebuffer->height);
  draw_kstring(height_str, center, 24, RED);

  char width_buf[128];
  kstring width_string = KSTRING(width_buf, 128);
  APPEND_STRL(&width_string, "WIDTH: ", framebuffer->width);
  APPEND_STRL(&width_string, "  pitch: ", framebuffer->pitch);
  draw_kstring(width_string, center, 48, GREEN);

  uint64_t colour = 0x000000;
  uint64_t hue = 0;
  // rainbowww
  asm volatile("int $60");
  for (;;) {
    asm volatile("int $60");
    for (volatile int i = 0; i < 1000000; i++)
      ; // delay
    asm volatile("int $0");
  }
  serial_write("hello after interrupts");
  for (;;) {
    colour = hsv_to_rgb_int(hue, 255, 255);
    // clear_screen(framebuffer, colour);
    hue++;
    if (hue >= 360)
      hue = 0;
  }
  hcf();
}
