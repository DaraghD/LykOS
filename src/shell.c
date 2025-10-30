#include "shell.h"
#include "arch/x86_64/util.h"
#include "drivers/fs/vfs.h"
#include "drivers/serial.h"
#include "graphics/draw.h"
#include "klib/kstring.h"
#include "mem/kalloc.h"
#include "mem/mem.h"
#include "req.h"
#include "terminal.h"
#include <stdbool.h>
#include <stdint.h>

// max length of a path is 256
char cwd_buf[256];
kstring cwd = KSTRING(cwd_buf, 256);

static int8_t change_dir(kstring *path) {
  if (path->len > cwd.cap) {
    serial_write_fstring("Path length {uint} is bigger than {uint}\n",
                         path->len, cwd.cap);
    return -1;
  }

  memcpy(cwd.buf, path->buf, path->len);
  cwd.len = path->len;
  return 0;
}

static void print_cwd(void) {
  serial_write_fstring("{str}\n", cwd.buf);
  terminal_fstring("{str}\n", cwd.buf);
}

void shell_init(void) {
  char init_path_buf[256];
  kstring init_path = KSTRING(init_path_buf, 256);
  APPEND_STR(&init_path, "/");
  if (init_path.error)
    serial_write_fstring("Error setting initial path\n");

  if (change_dir(&init_path) != 0)
    serial_write_fstring("Error setting initial path\n");
}

void shell_execute(kstring *line) {
  uint32_t temp_color = term_color;
  term_color = WHITE;
  term_ypos += 8 * g_scale;
  term_xpos = 0;

  if (kstrncmp(line, "ls", 2))
    list_files(&cwd);

  else if (kstrncmp(line, "cat", 3)) {
    // TODO: make this also apppend cwd to it
    //  e.g cat abc, cat + space is 4
    //  full len is 7, 7-4 = 3, len(abc) is 3
    uint16_t path_len = line->len - 4;
    char buf[path_len + 1];
    kstring path = KSTRING(buf, path_len);
    for (uint16_t i = 0; i < path_len; i++) {
      path.buf[i] = line->buf[i + 4];
      append_char(&path, line->buf[i + 4]);
    }
    path.buf[path_len + 1] = '\0';
    cat_file(&path);
  }

  else if (kstrncmp(line, "ascii", 5))
    draw_ascii();

  else if (kstrncmp(line, "logo", 4))
    draw_logo();

  else if (kstrncmp(line, "exit", 4))
    qemu_shutdown();

  else if (kstrncmp(line, "pwd", 3))
    print_cwd();

  else if (kstrncmp(line, "rainbow", 7))
    infinite_rainbow(get_framebuffer());

  else if (kstrncmp(line, "red", 3))
    term_color = RED;

  else if (kstrncmp(line, "blue", 4))
    term_color = BLUE;

  else if (kstrncmp(line, "green", 5))
    term_color = GREEN;

  else if (kstrncmp(line, "mmap", 4))
    print_memmap();

  else if (kstrncmp(line, "allocstats", 4))
    print_allocation_stats();

  else if (kstrncmp(line, "paging", 6))
    print_paging();

  else if (kstrncmp(line, "memstats", 8))
    print_memory_stats();

  else if (kstrncmp(line, "dbgalloc", 8))
    pmm_alloc_frame();

  else if (kstrncmp(line, "clear", 5))
    terminal_clearscreen();

  else if (kstrncmp(line, "freelist", 8))
    debug_freelist();

  else {
	  terminal_fstring("Unknown command\n");
	  goto done;
  }

  history_add(line);

done:
  term_color = temp_color;
  terminal_newline();
  return;
}
