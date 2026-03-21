#pragma once
#include "klib/kstring.h"
#include "req.h"
#include "vendor/limine.h"
#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>

#define GREEN 		 0x00FF00
#define BLUE  		 0x0000FF
#define BLACK 		 0x000000
#define RED   		 0xFF0000
#define WHITE 		 0xFFFFFF
#define YELLOW       0xFFFF00
#define CYAN         0x00FFFF
#define MAGENTA      0xFF00FF
#define ORANGE       0xFF8800
#define PURPLE       0x8800FF
#define PINK         0xFF66AA
#define GREY         0x888888
#define DARK_GREY    0x444444
#define LIME         0x88FF00
#define TEAL         0x008888
#define RUST         0xB7410E
#define CRIMSON      0xDC143C
#define BURNT_ORANGE 0xCC5500
#define TERRACOTTA   0xE2725B
#define MARS_RED     0xAD3A2D

typedef struct limine_framebuffer limine_framebuffer;

extern u8 g_scale;
extern u32 g_width;
extern u32 g_height;
extern bool is_graphics_init;

limine_framebuffer *get_framebuffer(void);
void graphics_init(void);
void debug_graphics(void);
void clear_screen(struct limine_framebuffer *fb_ptr, u32 color);
void set_draw_scale(u8 scale);
void put_pixel(size_t x, size_t y, u32 color);
void draw_char(char c, size_t px, size_t py, u32 color);
void fill_char(size_t px, size_t py, u32 color);
void draw_char_scaled(char c, size_t px, size_t py, u32 color,
                      size_t scale);
void draw_kstring(kstring *string, size_t px, size_t py, u32 color);
void draw_string(const char *str, size_t px, size_t py, u32 color);
void infinite_rainbow(limine_framebuffer *framebuffer);
