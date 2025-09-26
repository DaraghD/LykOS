#include "kstring.h"
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>

void append(kstring *b, char *src, int len) {
  int avail = b->cap - b->len;
  int amount = avail < len ? avail : len;
  for (int i = 0; i < amount; i++) {
    b->buf[b->len + i] = src[i];
  }
  b->len += amount;
  b->error |= amount < len;
}

void append_char(kstring *b, char c) {
  if (b->len >= b->cap) {
    b->error = true;
    return;
  }
  b->buf[b->len] = c;
  b->len += 1;
}
void append_long(kstring *b, long x) {
  char tmp[32];
  char *end = tmp + sizeof(tmp);
  char *beg = end;
  long t = x > 0 ? -x : x;
  do {
    *--beg = '0' - t % 10;
  } while (t /= 10);
  if (x < 0) {
    *--beg = '-';
  }
  append(b, beg, end - beg);
}
void append_int(kstring *b, int x) { append_long(b, (long)x); }

bool kstrncmp(kstring *ks, const char *cmp, int len) {
  if (ks->len < len) {
    return false;
  }
  const char *buf = ks->buf;
  for (int i = 0; i < len; i++) {
    if (buf[i] != cmp[i]) {
      return false;
    }
  }
  return true;
}
