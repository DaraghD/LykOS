#include "serial.h"
#include "arch/x86_64/io.h"
#include "klib/kstring.h"
#include <stdint.h>

#define COM1 0x3F8 // Base I/O port for COM1

// Initialise the first serial port (COM1)
void serial_init(void) {
  outb(COM1 + 1, 0x00); // Disable interrupts
  outb(COM1 + 3, 0x80); // Enable DLAB (set baud rate divisor)
  outb(COM1 + 0, 0x03); // Divisor low byte (38400 baud)
  outb(COM1 + 1, 0x00); // Divisor high byte
  outb(COM1 + 3, 0x03); // 8 bits, no parity, one stop bit
  outb(COM1 + 2, 0xC7); // Enable FIFO, clear them, 14-byte threshold
  outb(COM1 + 4, 0x0B); // IRQs enabled, RTS/DSR set
}

// Check if the transmit holding register is empty
static int serial_is_transmit_empty(void) { return inb(COM1 + 5) & 0x20; }

// Write a single character to COM1
void serial_write_char(char c) {
  while (serial_is_transmit_empty() == 0)
    ; // wait until ready
  outb(COM1, c);
}

// Write a null-terminated string to COM1
void serial_write(const char *s) {
  while (*s) {
    if (*s == '\n') {
      serial_write_char('\r');
    }
    serial_write_char(*s++);
  }
}

void serial_writeln(const char *s) {
  serial_write(s);
  serial_write("\r\n");
}

void serial_write_fstring(const char *format, ...) {
  va_list args;
  va_start(args, format);

  while (*format) {
    if (format[0] == '{' && strncmp(format, "{uint}", 6) == 0) {
      uint64_t value = va_arg(args, uint64_t);
      char buf[21];

      uitoa(value, buf);
      serial_write(buf);
      format += 6;
    } else if (format[0] == '{' && strncmp(format, "{int}", 5) == 0) {

      int64_t value = va_arg(args, int);
      char buf[32];
      itoa(value, buf);
      serial_write(buf);
      format += 5;

    } else if (format[0] == '{' && strncmp(format, "{char}", 6) == 0) {
      char c = (char)va_arg(args, int); // promoted to int
      serial_write_char(c);
      format += 6;
    } else {
      serial_write_char(*format);
      format++;
    }
  }
  va_end(args);
}
