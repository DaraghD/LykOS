#include "widget.h"
#include "graphics/draw.h"
#include "klib/time.h"
#include "proc/task.h"
#include "req.h"
#include "terminal.h"

void pit_spinner_tick(void) {
  u8 spinner_char = 0;
  while (1) {
    char c = spinner_chars[spinner_char];
    spinner_char++;
    if (spinner_char > 3)
      spinner_char = 0;
    fill_char(136, 0, BLACK);
    draw_char_scaled(c, 136, 0, MARS_RED, 2);
    task_sleep(300);
  }
}

u64 clock_base_x = 0;
u64 clock_base_y = 0;

void draw_clock_char(char c) {
  draw_char_scaled(c, clock_base_x, clock_base_y, CRIMSON, 2);
  clock_base_x += 16;
  if (clock_base_x == (16 * 8))
    clock_base_x = 0;
}

void format_2digits(u16 value, char out[2]) {
  out[0] = '0' + (value / 10);
  out[1] = '0' + (value % 10);
}

void draw_clock(void) {
  while (1) {
    for (u32 i = 0; i < 8 * 16;
         i++) { // hack, maybe start terminal drawing lower
      fill_char(clock_base_x + i, clock_base_y, BLACK);
    }

    char hours[2];
    char minutes[2];
    char seconds[2];
    u64 temp = term_color;
    term_color = CYAN;
    format_2digits(get_hours() % 100, hours);
    format_2digits(clock_minutes(), minutes);
    format_2digits(clock_seconds(), seconds);

    draw_clock_char(hours[0]);
    draw_clock_char(hours[1]);

    draw_clock_char(':');

    draw_clock_char(minutes[0]);
    draw_clock_char(minutes[1]);

    draw_clock_char(':');

    draw_clock_char(seconds[0]);
    draw_clock_char(seconds[1]);
    term_color = temp;
    task_sleep(1000);
  }
}
