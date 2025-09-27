#pragma once
#include "klib/kstring.h"
#include "vendor/limine.h"
#include <stddef.h>
#include <stdint.h>

#define GREEN 0x00FF00
#define BLUE 0x0000FF
#define BLACK 0x000000
#define RED 0xFF0000

typedef struct limine_framebuffer limine_framebuffer;

void set_draw_scale(uint8_t scale);
void init_graphics(void);
limine_framebuffer *get_framebuffer(void);
void put_pixel(size_t x, size_t y, size_t color);
void draw_char(char c, size_t px, size_t py, uint32_t color);
void draw_kstring(kstring *string, size_t px, size_t py, uint32_t color);
void draw_string(const char *str, size_t px, size_t py, uint32_t color);
void draw_char_scaled(char c, size_t px, size_t py, uint32_t color, size_t scale);
void clear_screen(struct limine_framebuffer *fb_ptr, uint32_t color);
void draw_char_term(char c);
void draw_string_term(const char *str);
uint32_t hsv_to_rgb_int(uint16_t h, uint8_t s, uint8_t v);
void infinite_rainbow(limine_framebuffer *framebuffer);
void debug_graphics(void);
void fill_char(size_t px, size_t py);
void draw_fstring(const char *format, uint32_t color, ...);

extern uint8_t m_scale;
extern uint64_t x_pos;
extern uint64_t y_pos;
