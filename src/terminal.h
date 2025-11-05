#pragma once
#include <stdint.h>
#include "klib/kstring.h"
#include "req.h"

extern char scancode_to_ascii[128];
extern u32 term_color;
extern u64 term_xpos;
extern u64 term_ypos;

void terminal_init(void);
void terminal_process_input(u16 sc);
void terminal_newline(void);
void draw_char_term(char c);
void terminal_fstring(const char *format, ...);
void draw_string_term(const char *str);
void terminal_clearscreen(void);
void draw_ascii(void);
void draw_logo(void);
void history_add(const kstring *command);
