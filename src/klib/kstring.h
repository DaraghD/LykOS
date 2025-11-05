#pragma once
#include <stdint.h>
#include <stdbool.h>

#define KSTRING(buf, cap) {buf, cap, 0, 0}
#define APPEND_STR(buf, str) append(buf, str, sizeof(str)-1)
#define APPEND_STRL(buf, str, long) APPEND_STR(buf,str); append_long(buf, long)

#define va_list  __builtin_va_list
#define va_start __builtin_va_start
#define va_end   __builtin_va_end
#define va_arg   __builtin_va_arg

typedef struct {
  char *buf;
  int cap;
  int len;
  bool error;
} kstring;


typedef enum 
{
  TERMINAL,
  SERIAL,
} io_type;

void append(kstring *b, char *src, int len);
void append_long(kstring *b, long x);
void append_char(kstring *b, char c);
bool kstrncmp(kstring* ks, const char* cmp, int len);
int strncmp(const char *s1, const char *s2, unsigned int n);
int strcmp(const char *s1, const char *s2);
int strlen(const char *s);
void write_fstring(io_type io, const char *format, va_list args); 
void to_upper(kstring* ks);
