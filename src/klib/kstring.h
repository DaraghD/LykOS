#pragma once
#include <stdbool.h>
#include <stdint.h>

#define KSTRING(buf, cap) {buf, cap, 0, 0}
#define APPEND_STR(buf, str) append(buf, str, sizeof(str)-1)
#define APPEND_STRL(buf, str, long) APPEND_STR(buf,str); append_long(buf, long)
#define QUICK_KSTRING(str) char buf[128]; kstring ks = KSTRING(buf, 128); APPEND_STR(&ks, str); ks;

// TODO: try get these working for vsprintf formatting
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

void append(kstring *b, char *src, int len);
void append_long(kstring *b, long x);
void append_char(kstring *b, char c);
bool kstrncmp(kstring* ks, const char* cmp, int len);
char *itoa(int64_t value, char *str);
char *uitoa(uint64_t value, char* str);
int strncmp(const char *s1, const char *s2, unsigned int n);
