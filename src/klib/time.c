#include "time.h"
#include "drivers/serial.h"
#include "terminal.h"

// give ticks get out a formated time

u64 ticks_total_seconds(u64 ticks) { return ticks / PIT_FREQ_HZ; }

u64 ticks_to_seconds(u64 ticks) { return ticks_total_seconds(ticks) % 60; }

u64 ticks_to_minutes(u64 ticks) {
  return (ticks_total_seconds(ticks) / 60) % 60;
}

u64 ticks_to_hours(u64 ticks) { return ticks_total_seconds(ticks) / 3600; }

// hh:mm:ss
void ticks_to_time(u64 ticks, char out[9]) {
  serial_write_fstring("START TICK {uint}\n", ticks);
  serial_write_fstring("CURRENT TICK {uint}\n", pit_get_ticks());

  u64 uptime = pit_get_ticks() - ticks;
  serial_write_fstring("UPTIME {uint}\n", uptime);

  out[0] = '0' + (ticks_to_hours(uptime) / 10);
  out[1] = '0' + (ticks_to_hours(uptime) % 10);
  out[2] = ':';

  out[3] = '0' + (ticks_to_minutes(uptime) / 10);
  out[4] = '0' + (ticks_to_minutes(uptime) % 10);
  out[5] = ':';

  out[6] = '0' + (ticks_to_seconds(uptime) / 10);
  out[7] = '0' + (ticks_to_seconds(uptime) % 10);
  out[8] = '\0';
}

u64 get_seconds(void) { return pit_get_ticks() / PIT_FREQ_HZ; }
u64 get_minutes(void) { return get_seconds() / 60; }
u64 get_hours(void) { return get_minutes() / 60; }

u64 clock_seconds(void) { return get_seconds() % 60; }
u64 clock_minutes(void) { return get_minutes() % 60; }
void draw_uptime(void) {
  terminal_fstring("Uptime: {uint}:{uint}:{uint}\n", get_hours(),
                   clock_minutes(), clock_seconds());
}
