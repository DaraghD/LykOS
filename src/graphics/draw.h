#pragma once
#include "klib/kstring.h"
#include "vendor/limine.h"
#include <stddef.h>
#include <stdint.h>

#define GREEN 0x00FF00
#define BLUE 0x0000FF
#define BLACK 0x000000
#define RED 0xFF0000
#define WHITE 0xFFFFFF

typedef struct limine_framebuffer limine_framebuffer;

extern uint8_t g_scale;
extern uint32_t g_width;
extern uint32_t g_height;

limine_framebuffer *get_framebuffer(void);
void graphics_init(void);
void debug_graphics(void);
void clear_screen(struct limine_framebuffer *fb_ptr, uint32_t color);
void set_draw_scale(uint8_t scale);
void put_pixel(size_t x, size_t y, size_t color);
void draw_char(char c, size_t px, size_t py, uint32_t color);
void fill_char(size_t px, size_t py);
void draw_char_scaled(char c, size_t px, size_t py, uint32_t color, size_t scale);
void draw_kstring(kstring *string, size_t px, size_t py, uint32_t color);
void draw_string(const char *str, size_t px, size_t py, uint32_t color); void infinite_rainbow(limine_framebuffer *framebuffer);
