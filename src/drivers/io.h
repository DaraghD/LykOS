#ifndef IO_H
#define IO_H

#include <stdint.h>

// Write a byte/word/dword to an I/O port
void outb(uint16_t port, uint8_t val);
void outw(uint16_t port, uint16_t val);
void outl(uint16_t port, uint32_t val);

// Read a byte/word/dword from an I/O port
uint8_t inb(uint16_t port);
uint16_t inw(uint16_t port);
uint32_t inl(uint16_t port);

#endif

