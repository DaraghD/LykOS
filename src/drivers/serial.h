#pragma once
#include <stddef.h>
#include <stdint.h>

void serial_init(void);
void serial_write_char(char c);
void serial_write(const char *s);
void serial_writeln(const char *s);
void serial_write_fstring(const char *format, ...);
void serial_write_i(uint64_t value);
