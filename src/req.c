#include "req.h"
#include <stddef.h>
#include <stdint.h>

// GCC and Clang reserve the right to generate calls to the following
// 4 functions even if they are not directly called.
// Implement them as the C specification mandates.
// DO NOT remove or rename these functions, or stuff will eventually break!
// They CAN be moved to a different .c file.

void *memcpy(void *restrict dest, const void *restrict src, size_t n) {
  u8 *restrict pdest = (u8 *restrict)dest;
  const u8 *restrict psrc = (const u8 *restrict)src;

  for (size_t i = 0; i < n; i++) {
    pdest[i] = psrc[i];
  }

  return dest;
}

void *memset(void *s, int c, size_t n) {
  u8 *p = (u8 *)s;

  for (size_t i = 0; i < n; i++) {
    p[i] = (u8)c;
  }

  return s;
}

void *memmove(void *dest, const void *src, size_t n) {
  u8 *pdest = (u8 *)dest;
  const u8 *psrc = (const u8 *)src;

  if (src > dest) {
    for (size_t i = 0; i < n; i++) {
      pdest[i] = psrc[i];
    }
  } else if (src < dest) {
    for (size_t i = n; i > 0; i--) {
      pdest[i - 1] = psrc[i - 1];
    }
  }

  return dest;
}

int memcmp(const void *s1, const void *s2, size_t n) {
  const u8 *p1 = (const u8 *)s1;
  const u8 *p2 = (const u8 *)s2;

  for (size_t i = 0; i < n; i++) {
    if (p1[i] != p2[i]) {
      return p1[i] < p2[i] ? -1 : 1;
    }
  }

  return 0;
}
