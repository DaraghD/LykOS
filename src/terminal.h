#pragma once
#include <stdint.h>

extern char scancode_to_ascii[128];
extern uint32_t term_color;

void terminal_init(void);
void terminal_process_input(uint8_t sc);
