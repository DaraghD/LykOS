#pragma once
#include <stdint.h>

extern char scancode_to_ascii[128];
extern uint32_t term_color;
extern uint64_t term_xpos;
extern uint64_t term_ypos;

void terminal_init(void);
void terminal_process_input(uint16_t sc);
void terminal_newline(void);
void draw_char_term(char c);
void terminal_fstring(const char *format, ...);
void draw_string_term(const char *str);
void terminal_clearscreen(void);
void draw_ascii(void);
void draw_logo(void);
