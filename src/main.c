#include "arch/x86_64/gdt.h"
#include "arch/x86_64/idt.h"
#include "arch/x86_64/pic.h"
#include "drivers/ps2.h"
#include "drivers/serial.h"
#include "fs/fat16.h"
#include "fs/vfs.h"
#include "graphics/draw.h"
#include "mem/mem.h"
#include "terminal.h"
#include "vendor/stb_ds.h"
#include <stdint.h>

static void hcf(void) {
  for (;;) {
    asm("hlt");
  }
}

void kmain(void) {
  gdt_init();
  idt_init();
  pic_remap();
  memmap_init();
  pmm_init();
  graphics_init();
  asm volatile("sti");
  fat16_init(); // assumes fat on disk
  // fat16_list_files();

  terminal_init();
  // serial_write_fstring("Found file: cluster {uint}, size
  // {uint},name={str}\n",
  //                      entry.first_cluster, entry.file_size, entry.name);

  // fat16_dir_entry_t fat_entry = *fat16_find_file("FATREAD    ");
  // fat16_dir_entry_t targa_entry = *fat16_find_file("LYKOS2  TGA");

  // void *k = kalloc(999999);
  // kfree((u64)k);
  //
  const char *name = "test.txt";
  char buf[11];
  fat16_format83_name(name, buf);

  // allocs
  // for (int i = 0; i < 1000; i++) {
  //   void *x = kalloc(i);
  //   kfree((u64)x);
  // }
  kstring path = {.buf = "/directory/test/abc",
                  .len = strlen("/directory/test/abc"),
                  .cap = strlen("/directory/test/abc") + 1};
  kstring *parts = vfs_split_path(&path);
  for (int i = 0; i < arrlen(parts); i++) {
    serial_write_fstring("{str}\n", parts[i].buf);
    terminal_fstring("{str}\n", parts[i].buf);
  }

  u32 i = 0;
  while (1) {
    keyboard_process();
  }

  hcf();
}
