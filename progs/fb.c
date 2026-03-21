#include "lykosapi.h"

#define RED 0xFF0000
#define GREEN 0x00FF00

void main(int argc, char **argv) {
  u32 *fb = mmap_fb();
  u32 height = 1080;
  u32 width = 1920;
  for (int i = 0; i < 100; i++)
    fb[width * 1000 + i] = GREEN;
  return;
}
