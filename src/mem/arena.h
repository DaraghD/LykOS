#pragma once

#include <stddef.h>
#include <stdint.h>

#define new(a, t, n) (t *)arena_alloc(a, sizeof(t), _Alignof(t), n)

typedef struct {
  char *beg;
  char *end;
} arena;

void *arena_alloc(arena *a, ptrdiff_t size, ptrdiff_t align, ptrdiff_t count);
arena arena_init(ptrdiff_t cap);
