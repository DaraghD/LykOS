#include "idt.h"
#include "drivers/ps2.h"
#include "drivers/serial.h"
#include "terminal.h"
#include <stdbool.h>
#include <stdint.h>

static IDTEntry idt[256];
static IDTPointer idt_ptr;

#define RING_0_64_PRESENT_GATE 0x8E
#define KERNEL_CODE_SEGMENT 0x08

void set_idt_gate(u32 index, u64 handler_ptr, u16 selector,
                  u8 flags) {

  idt[index].offset_low = handler_ptr & 0xFFFF;
  idt[index].selector = selector; // code segment selector to GDT, usually 0x08
  idt[index].ist = 0;
  idt[index].type_attr = flags;
  idt[index].offset_mid = (handler_ptr >> 16) & 0xFFFF;
  idt[index].offset_high = (handler_ptr >> 32) & 0xFFFFFFFF;
  idt[index].zero = 0;
}

static inline bool are_interrupts_enabled(void) {
  unsigned long flags;
  asm volatile("pushf\n\t"
               "pop %0"
               : "=g"(flags));
  return flags & (1 << 9);
}
void idt_init(void) {
  serial_writeln("Initialising interrupt descriptor table");
  idt_ptr.limit = (sizeof(IDTEntry) * 256) - 1;
  idt_ptr.base = (u64)&idt;

  set_idt_gate(0, (u64)isr_divide_error, KERNEL_CODE_SEGMENT,
               RING_0_64_PRESENT_GATE);

  set_idt_gate(60, (u64)test_isr, KERNEL_CODE_SEGMENT,
               RING_0_64_PRESENT_GATE);

  set_idt_gate(0x21, (u64)isr_ps2_keyboard, KERNEL_CODE_SEGMENT,
               RING_0_64_PRESENT_GATE);

  asm volatile("lidt %0" : : "m"(idt_ptr));
  if (are_interrupts_enabled()) {
    serial_writeln("Interrupts enabled");
  } else {
    serial_writeln("Interrutps not enabled");
  }
}

__attribute__((interrupt)) void isr_divide_error(interrupt_frame *frame) {
  (void)frame;
  serial_writeln("Divide by zero!");
}

__attribute__((interrupt)) void test_isr(interrupt_frame *frame) {
  (void)frame;
  serial_writeln("Test interrupt");
  terminal_fstring("Interrupt 60 called\n");
}
