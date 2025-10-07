#pragma once
#include <stdint.h>

extern char scancode_to_ascii[128];
extern uint32_t term_color;
extern uint64_t term_xpos;
extern uint64_t term_ypos;

void terminal_init(void);
void terminal_process_input(uint16_t sc);
void terminal_newline(void);
void terminal_fstring(const char *format, ...);
void terminal_clearscreen(void);
