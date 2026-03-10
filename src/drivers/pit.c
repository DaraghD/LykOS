#include "pit.h"
#include "arch/x86_64/io.h"
#include "drivers/serial.h"

u32 pit_freq_hz = 0;
volatile u64 pit_ticks = 0;

void pit_init(u32 freq_hz) {
  if (!freq_hz)
    freq_hz = PIT_FREQ_HZ;

  u32 divisor = PIT_HZ / freq_hz;
  if (divisor < 1)
    divisor = 1;
  if (divisor > 0xFFFF)
    divisor = 0xFFFF;

  // channel 0, lobyte/hibyte, mode 2 (rate generator), binary
  outb(PIT_CMD, 0x34);
  outb(PIT_CH0_DATA, (u8)(divisor & 0xFF));
  outb(PIT_CH0_DATA, (u8)((divisor >> 8) & 0xFF));

  serial_writeln("PIT programmed");
  pit_freq_hz = freq_hz;
}

void pit_tick(void) { pit_ticks++; }
u64 pit_get_ticks(void) { return pit_ticks; }
