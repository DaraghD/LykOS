#include "kstring.h"
#include "drivers/serial.h"
#include "mem/kalloc.h"
#include "req.h"
#include "terminal.h"
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
static char *itoa(int64_t value, char *str) {
  char *p = str;
  char *p1 = str;
  char tmp;

  // handle 0 explicitly
  if (value == 0) {
    *p++ = '0';
    *p = '\0';
    return str;
  }

  // handle negatives
  uint64_t sign = value < 0;
  if (sign)
    value = -value;

  // convert digits
  while (value > 0) {
    *p++ = '0' + (value % 10);
    value /= 10;
  }
  if (sign)
    *p++ = '-';

  *p = '\0';

  // reverse the string in-place
  for (--p; p1 < p; ++p1, --p) {
    tmp = *p1;
    *p1 = *p;
    *p = tmp;
  }

  return str;
}
static char *uitoa(uint64_t value, char *str) {
  char *p = str;
  char *p1 = str;
  char tmp;

  // handle 0 explicitly
  if (value == 0) {
    *p++ = '0';
    *p = '\0';
    return str;
  }

  // convert digits (no sign check needed for unsigned type)
  while (value > 0) {
    *p++ = '0' + (value % 10);
    value /= 10;
  }

  *p = '\0';

  // reverse the string in-place
  // p points one past the last digit (i.e., at the null terminator)
  for (--p; p1 < p; ++p1, --p) {
    tmp = *p1;
    *p1 = *p;
    *p = tmp;
  }

  return str;
}

int strncmp(const char *s1, const char *s2, unsigned int n) {
  unsigned int i = 0;
  while (i < n && s1[i] && s2[i]) {
    if (s1[i] != s2[i]) {
      return s1[i] - s2[i];
    }
    i++;
  }

  if (i < n) {
    return s1[i] - s2[i];
  }

  return 0;
}

int strcmp(const char *s1, const char *s2) {
  int i = 0;
  while (s1[i] && s2[i]) {
    if (s1[i] != s2[i]) {
      return s1[i] - s2[i];
    }
    i++;
  }
  return 0;
}

static char *intohex(uint64_t n, char *str, size_t width, bool trim_zero) {
  if (width > 16)
    width = 16; // max for 64-bit

  // fill digits from end
  for (size_t i = width; i > 0; i--) {
    str[i - 1] = "0123456789ABCDEF"[n & 0xF];
    n >>= 4;
  }
  str[width] = '\0';

  if (trim_zero) {
    // find first non-zero digit
    size_t start = 0;
    while (str[start] == '0' && start < width - 1)
      start++;
    // shift string to beginning
    if (start > 0) {
      size_t j = 0;
      while (str[start] != '\0')
        str[j++] = str[start++];
      str[j] = '\0';
    }
  }

  return str;
}

typedef void (*write_string_fptr)(const char *str);
typedef void (*write_char_fptr)(char c);

void write_fstring(io_type io, const char *format, va_list args) {
  write_string_fptr write_string = &serial_write;
  write_char_fptr write_char = &serial_write_char;
  switch (io) {
  case TERMINAL:
    write_string = &draw_string_term;
    write_char = &draw_char_term;
    break;
  case SERIAL:
    write_string = &serial_write;
    write_char = &serial_write_char;
    break;
  }

  while (*format) {
    if (format[0] == '{' && strncmp(format, "{uint}", 6) == 0) {
      uint64_t value = va_arg(args, uint64_t);
      char buf[65];
      uitoa(value, buf);
      write_string(buf);
      format += 6;
    } else if (format[0] == '{' && strncmp(format, "{int}", 5) == 0) {
      int value = va_arg(args, int);
      char buf[65];
      itoa(value, buf);
      write_string(buf);
      format += 5;
    } else if (format[0] == '{' && strncmp(format, "{hex8}", 6) == 0) {
      int value = va_arg(args, int);
      char buf[17];
      intohex(value, buf, 8, true);
      write_string(buf);
      format += 5;
    } else if (format[0] == '{' && strncmp(format, "{hex4}", 6) == 0) {
      int value = va_arg(args, int);
      char buf[17];
      intohex(value, buf, 4, true);
      write_string(buf);
      format += 5;
    } else if (format[0] == '{' && strncmp(format, "{hex}", 5) == 0) {
      int value = va_arg(args, int);
      char buf[17];
      intohex(value, buf, 16, true);
      write_string(buf);
      format += 5;
    } else if (format[0] == '{' && strncmp(format, "{char}", 6) == 0) {
      char c = (char)va_arg(args, int);
      write_string(&c);
      format += 6;
    } else if (format[0] == '{' && strncmp(format, "{str}", 5) == 0) {
      const char *s = va_arg(args, const char *);
      write_string(s);
      format += 5;
    } else if (format[0] == '{' && strncmp(format, "{kstr}", 6) == 0) {
      kstring *ks = va_arg(args, kstring *);
      //? only works if memory is zerod?
      char *buf = kalloc(ks->len + 1);
      memcpy(buf, ks->buf, ks->len);
      buf[ks->len] = '\0';
      // char buf[256];
      // for (int i = 0; i < ks->len; i++) {
      //   buf[i] = ks->buf[i];
      // }
      buf[ks->len] = '\0';
      write_string(buf);
      kfree((uint64_t)buf);
      format += 6;
    } else {
      write_char(*format);
      format++;
    }
  }
}

int strlen(const char *s) {
  size_t len = 0;
  while (s[len] != '\0') {
    len++;
  }
  return len;
}
