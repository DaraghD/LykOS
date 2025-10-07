#include "arena.h"
#include "drivers/serial.h"
#include "mem/kalloc.h"
#include "req.h"

void *arena_alloc(arena *a, ptrdiff_t size, ptrdiff_t align, ptrdiff_t count) {
  ptrdiff_t padding = -(uintptr_t)a->beg & (align - 1);
  ptrdiff_t available = a->end - a->beg - padding;
  if (available < 0 || count > available / size) {
    serial_write_fstring("Couldn't allocate arena");
    return NULL;
  }
  void *p = a->beg + padding;
  a->beg += padding + count * size;
  return memset(p, 0, count * size);
}

arena arena_init(ptrdiff_t cap) {
  arena a = {0};
  serial_write_fstring("\nAllocating for arena {uint}\n", cap);
  a.beg = kalloc(cap);
  a.end = a.beg ? a.beg + cap : 0;
  return a;
}
