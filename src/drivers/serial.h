#pragma once

void serial_init(void);
void serial_write_char(char c);
void serial_write(const char *s);
void serial_writeln(const char *s);
