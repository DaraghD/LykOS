#include "draw.h"
#include "drivers/ps2.h"
#include "drivers/serial.h"
#include "terminal.h"
#include "vendor/font.h"
#include "vendor/limine.h"
#include <stddef.h>
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

uint32_t width;
uint32_t height;

uint8_t m_scale = 1;
uint64_t x_pos = 0;
uint64_t y_pos = 0;
void set_draw_scale(uint8_t scale) { m_scale = scale; }

void draw_cursor_term(void) {
  uint32_t old_color = term_color;
  term_color = WHITE;
  draw_char('_', x_pos, y_pos, term_color);
  term_color = old_color;
}

// draws character using global x_pos and y_pos positions
// probably should be moved into terminal code
void draw_char_term(char c) {
  if (c == '\n') {
    y_pos += 8 * m_scale;
    x_pos = 0;
  }
  draw_char(c, x_pos, y_pos, term_color);
  // advance the cursor
  x_pos += 8 * m_scale;
  // if next position is greater than the width
  // takes into account how big the START of the next character will be
  if (x_pos > (get_framebuffer()->width) - (7 * m_scale)) {
    y_pos += 8 * m_scale;
    x_pos = 0;
  }
}

// TODO: move this and draw_char_term to terminal code
void draw_string_term(const char *str) {
  while (*str) {
    if (*str == '\n') {
      x_pos = 0;
      y_pos += 8 * m_scale;
    } else {
      draw_char_term(*str);
    }
    str++;
  }
}

void init_graphics(void) {
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
  width = get_framebuffer()->width;
  height = get_framebuffer()->height;
}

limine_framebuffer *get_framebuffer(void) {
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

void fill_char(size_t px, size_t py) {
  size_t scale = m_scale;
  for (size_t row = 0; row < 8; row++) {
    for (size_t col = 0; col < 8; col++) {
      // Draw a scale x scale block instead of a single pixel
      for (size_t dy = 0; dy < scale; dy++) {
        for (size_t dx = 0; dx < scale; dx++) {
          put_pixel(px + col * scale + dx, py + row * scale + dy, BLACK);
        }
      }
    }
  }
}

void draw_string(const char *str, size_t px, size_t py, uint32_t color) {
  while (*str) {
    if (px > (width - (7 * m_scale))) {
      px = 0;
      py += 8 * m_scale;
    }
    if (*str == '\n') {
      py += 8 * m_scale;
      px = 0;
    } else {
      draw_char_scaled(*str, px, py, color, m_scale);
      px += 8 * m_scale;
    }
    str++;
  }
}

void draw_kstring(kstring *string, size_t px, size_t py, uint32_t color) {
  draw_string(string->buf, px, py, color);
}

void clear_screen(limine_framebuffer *fb_ptr, uint32_t color) {
  uint32_t *buffer = (uint32_t *)fb_ptr->address;
  size_t total_pixel = fb_ptr->height * fb_ptr->width;
  for (size_t i = 0; i < total_pixel; i++)
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

static uint64_t hue = 0;
void infinite_rainbow(limine_framebuffer *framebuffer) {
  uint64_t colour;
  // rainbowww
  for (;;) {
    colour = hsv_to_rgb_int(hue, 255, 255);
    clear_screen(framebuffer, colour);
    hue++;
    if (hue >= 360)
      hue = 0;
    return;
  }
}

void debug_graphics(void) {
  limine_framebuffer *framebuffer = get_framebuffer();
  uint32_t center = framebuffer->width / 2;
  center -= 100;

  set_draw_scale(3);
  draw_string("Hello Mars, from LykOS", center, 0, BLUE);
  char height_buf[128];
  kstring height_str = KSTRING(height_buf, 128);
  APPEND_STRL(&height_str, "HEIGHT: ", framebuffer->height);
  draw_kstring(&height_str, center, 24, RED);

  char width_buf[128];
  kstring width_string = KSTRING(width_buf, 128);
  APPEND_STRL(&width_string, "WIDTH: ", framebuffer->width);
  APPEND_STRL(&width_string, " pitch: ", framebuffer->pitch);
  draw_kstring(&width_string, center, 48, GREEN);
}

void draw_fstring(const char *format, ...) {
  va_list args;
  va_start(args, format);

  while (*format) {
    if (format[0] == '{' && strncmp(format, "{uint}", 6) == 0) {
      uint64_t value = va_arg(args, uint64_t);
      char buf[21];
      uitoa(value, buf);
      draw_string_term(buf);
      format += 6;
    } else if (format[0] == '{' && strncmp(format, "{int}", 5) == 0) {
      int value = va_arg(args, int);
      char buf[1000];
      itoa(value, buf);
      draw_string_term(buf);
      format += 5;
    } else if (format[0] == '{' && strncmp(format, "{char}", 6) == 0) {
      char c = (char)va_arg(args, int);
      draw_char_term(c);
      format += 6;
    } else {
      draw_char_term(*format);
      format++;
    }
  }
  va_end(args);
}
