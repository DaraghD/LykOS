#include "drivers/serial.h"
#include "klib/kstring.h"
#include "vendor/font.h"
#include "vendor/limine.h"
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

// Set the base revision to 3, this is recommended as this is the latest
// base revision described by the Limine boot protocol specification.
// See specification for further info.

__attribute__((
    used, section(".limine_requests"))) static volatile LIMINE_BASE_REVISION(3);

// The Limine requests can be placed anywhere, but it is important that
// the compiler does not optimise them away, so, usually, they should
// be made volatile or equivalent, _and_ they should be accessed at least
// once or marked as used with the "used" attribute as done here.

__attribute__((
    used,
    section(
        ".limine_requests"))) static volatile struct limine_framebuffer_request
    framebuffer_request = {.id = LIMINE_FRAMEBUFFER_REQUEST, .revision = 0};

// Finally, define the start and end markers for the Limine requests.
// These can also be moved anywhere, to any .c file, as seen fit.

__attribute__((used,
               section(".limine_requests_"
                       "start"))) static volatile LIMINE_REQUESTS_START_MARKER;

__attribute__((
    used,
    section(
        ".limine_requests_end"))) static volatile LIMINE_REQUESTS_END_MARKER;

// GCC and Clang reserve the right to generate calls to the following
// 4 functions even if they are not directly called.
// Implement them as the C specification mandates.
// DO NOT remove or rename these functions, or stuff will eventually break!
// They CAN be moved to a different .c file.

void *memcpy(void *restrict dest, const void *restrict src, size_t n) {
  uint8_t *restrict pdest = (uint8_t *restrict)dest;
  const uint8_t *restrict psrc = (const uint8_t *restrict)src;

  for (size_t i = 0; i < n; i++) {
    pdest[i] = psrc[i];
  }

  return dest;
}

void *memset(void *s, int c, size_t n) {
  uint8_t *p = (uint8_t *)s;

  for (size_t i = 0; i < n; i++) {
    p[i] = (uint8_t)c;
  }

  return s;
}

void *memmove(void *dest, const void *src, size_t n) {
  uint8_t *pdest = (uint8_t *)dest;
  const uint8_t *psrc = (const uint8_t *)src;

  if (src > dest) {
    for (size_t i = 0; i < n; i++) {
      pdest[i] = psrc[i];
    }
  } else if (src < dest) {
    for (size_t i = n; i > 0; i--) {
      pdest[i - 1] = psrc[i - 1];
    }
  }

  return dest;
}

int memcmp(const void *s1, const void *s2, size_t n) {
  const uint8_t *p1 = (const uint8_t *)s1;
  const uint8_t *p2 = (const uint8_t *)s2;

  for (size_t i = 0; i < n; i++) {
    if (p1[i] != p2[i]) {
      return p1[i] < p2[i] ? -1 : 1;
    }
  }

  return 0;
}

static void hcf(void) {
  for (;;) {
    asm("hlt");
  }
}

typedef struct limine_framebuffer limine_framebuffer;

static inline void put_pixel(size_t x, size_t y, size_t color) {
  limine_framebuffer *framebuffer =
      framebuffer_request.response->framebuffers[0];
  size_t fb_width = framebuffer->width;
  size_t fb_height = framebuffer->height;
  volatile uint32_t *fb_ptr = framebuffer->address;

  if (x >= fb_width || y >= fb_height)
    return;
  fb_ptr[y * fb_width + x] = color;
}

void draw_char(char c, size_t px, size_t py, uint32_t color) {
  if (c < 0)
    return;
  for (size_t row = 0; row < 8; row++) {
    uint8_t bits = font8x8_basic[(size_t)c][row];
    for (size_t col = 0; col < 8; col++) {
      if (bits & (1 << col)) {
        put_pixel(px + col, py + row, color);
      }
    }
  }
}

void draw_string(const char *str, size_t px, size_t py, uint32_t color) {
  size_t orig_x = px;
  while (*str) {
    if (*str == '\n') {
      px = orig_x;
      py += 8;
    } else {
      draw_char(*str, px, py, color);
      px += 8;
    }
    str++;
  }
}
void draw_kstring(kstring string, size_t px, size_t py, uint32_t color) {
  draw_string(string.buf, px, py, color);
}

void clear_screen(limine_framebuffer *fb_ptr, uint32_t color) {
  volatile uint32_t *buffer = (volatile uint32_t *)fb_ptr->address;
  size_t total_pixel = fb_ptr->height * fb_ptr->width;
  for (size_t i = 0; i < total_pixel; i++)
    buffer[i] = color;
}
void keyboard_loop() {}

// The following will be our kernel's entry point.
// If renaming kmain() to something else, make sure to change the
// linker script accordingly.
void kmain(void) {
  // Ensure the bootloader actually understands our base revision (see spec).
  if (LIMINE_BASE_REVISION_SUPPORTED == false) {
    hcf();
  }

  // Ensure we got a framebuffer.
  if (framebuffer_request.response == NULL ||
      framebuffer_request.response->framebuffer_count < 1) {
    hcf();
  }

  // Fetch the first framebuffer.
  limine_framebuffer *framebuffer =
      framebuffer_request.response->framebuffers[0];

#define GREEN 0x00FF000

  draw_string("Hello Mars, from Lykos", 0, 0, GREEN);

  char height_buf[128];
  kstring height_str = KSTRING(height_buf, 128);
  APPEND_STRL(&height_str, "HEIGHT: ", framebuffer->height);
  draw_kstring(height_str, 0, 8, GREEN);

  char width_buf[128];
  kstring width_string = KSTRING(width_buf, 128);
  APPEND_STRL(&width_string, "WIDTH: ", framebuffer->width);
  draw_kstring(width_string, 0, 16, GREEN);

  serial_write(width_string.buf);
  serial_write(height_str.buf);

  // main loop -> replace with multi-tasking or some scheduler idk
  keyboard_loop();

  hcf();
}
