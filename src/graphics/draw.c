#include "draw.h"
#include "drivers/serial.h"
#include "vendor/font.h"
#include "vendor/limine.h"
#include <stdint.h>

// Finally, define the start and end markers for the Limine requests.
// These can also be moved anywhere, to any .c file, as seen fit.
__attribute__((used,
               section(".limine_requests_"
                       "start"))) static volatile LIMINE_REQUESTS_START_MARKER;

__attribute__((
    used,
    section(
        ".limine_requests_end"))) static volatile LIMINE_REQUESTS_END_MARKER;

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

static void hcf(void) {
  for (;;) {
    asm("hlt");
  }
}

uint8_t m_scale = 1;
uint64_t x_pos = 0;
uint64_t y_pos = 0;
void set_draw_scale(uint8_t scale) { m_scale = scale; }

void draw_char_term(char c, uint32_t color) {
  draw_char(c, x_pos, y_pos, color);
  // advance the cursor
  x_pos += 8 * m_scale;
  // if next position is greater than the width
  // takes into account how big the START of the next character will be
  if (x_pos > (get_framebuffer()->width) - (7 * m_scale)) {
    y_pos += 8 * m_scale;
    x_pos = 0;
  }
}

void draw_string_term(const char *str, uint32_t color) {
  size_t orig_x = x_pos;
  while (*str) {
    if (*str == '\n') {
      x_pos = orig_x;
    } else {
      draw_char_term(*str, color);
    }
    str++;
  }
}

void init_graphics() {
  // Ensure the bootloader actually understands our base revision (see spec).
  if (LIMINE_BASE_REVISION_SUPPORTED == false) {
    serial_write("Limine base revision not supported");
    hcf();
  }

  // Ensure we got a framebuffer.
  if (framebuffer_request.response == NULL ||
      framebuffer_request.response->framebuffer_count < 1) {
    serial_write("Failed to receive framebuffer");
    hcf();
  }
}

limine_framebuffer *get_framebuffer() {
  return framebuffer_request.response->framebuffers[0];
}

void put_pixel(size_t x, size_t y, size_t color) {
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
  draw_char_scaled(c, px, py, color, m_scale);
}

void draw_string(const char *str, size_t px, size_t py, uint32_t color) {
  size_t orig_x = px;
  while (*str) {
    if (*str == '\n') {
      px = orig_x;
      py += 8 * m_scale; // bug
    } else {
      draw_char_scaled(*str, px, py, color, m_scale);
      px += 8 * m_scale;
    }
    str++;
  }
}

void draw_kstring(kstring string, size_t px, size_t py, uint32_t color) {
  draw_string(string.buf, px, py, color);
}

void clear_screen(limine_framebuffer *fb_ptr, uint32_t color) {
  uint32_t *buffer = (uint32_t *)fb_ptr->address;
  size_t total_pixel = fb_ptr->height * fb_ptr->width;
  for (size_t i = total_pixel / 2; i < total_pixel; i++)
    buffer[i] = color;
}

void draw_char_scaled(char c, size_t px, size_t py, uint32_t color,
                      size_t scale) {
  if (c < 0)
    return;

  for (size_t row = 0; row < 8; row++) {
    uint8_t bits = font8x8_basic[(size_t)c][row];
    for (size_t col = 0; col < 8; col++) {
      if (bits & (1 << col)) {
        // Draw a scale x scale block instead of a single pixel
        for (size_t dy = 0; dy < scale; dy++) {
          for (size_t dx = 0; dx < scale; dx++) {
            put_pixel(px + col * scale + dx, py + row * scale + dy, color);
          }
        }
      }
    }
  }
}

uint32_t hsv_to_rgb_int(uint16_t h, uint8_t s, uint8_t v) {
  // h: 0–359, s/v: 0–255
  uint8_t region = h / 60; // sector 0–5
  uint16_t remainder = (h % 60) * 255 / 60;

  uint16_t p = (v * (255 - s)) / 255;
  uint16_t q = (v * (255 - (s * remainder) / 255)) / 255;
  uint16_t t = (v * (255 - (s * (255 - remainder)) / 255)) / 255;

  uint8_t r, g, b;
  switch (region) {
  case 0:
    r = v;
    g = t;
    b = p;
    break;
  case 1:
    r = q;
    g = v;
    b = p;
    break;
  case 2:
    r = p;
    g = v;
    b = t;
    break;
  case 3:
    r = p;
    g = q;
    b = v;
    break;
  case 4:
    r = t;
    g = p;
    b = v;
    break;
  case 5:
    r = v;
    g = p;
    b = q;
    break;
  default:
    r = 0;
    g = 0;
    b = 0;
    break;
  }
  return (r << 16) | (g << 8) | b;
}
