#pragma once
#include <stddef.h>

void *memset(void *s, int c, size_t n);
void *memcpy(void *restrict dest, const void *restrict src, size_t n);
int memcmp(const void *s1, const void *s2, size_t n);
