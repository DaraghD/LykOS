#ifndef KSTRING_H
#define KSTRING_H

#include <stdbool.h>

#define KSTRING(buf, cap) {buf, cap, 0, 0}
#define APPEND_STR(buf, str) append(buf, str, sizeof(str)-1)
#define APPEND_STRL(buf, str, long) APPEND_STR(buf,str); append_long(buf, long)
#define QUICK_KSTRING(str) char buf[128]; kstring ks = KSTRING(buf, 128); APPEND_STR(&ks, str); ks;

typedef struct {
  char *buf;
  int cap;
  int len;
  bool error;
} kstring;

void append(kstring *b, char *src, int len);
void append_long(kstring *b, long x);

#endif
