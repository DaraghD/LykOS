#include "idt.h"
#include "drivers/serial.h"
#include <stdbool.h>
#include <stdint.h>

static IDTEntry idt[256];
static IDTPointer idt_ptr;

#define RING_0_64_PRESENT_GATE 0x8E
#define KERNEL_CODE_SEGMENT 0x08
// handler is a function pointer
void set_idt_gate(uint32_t index, uint64_t handler_ptr, uint16_t selector,
                  uint8_t flags) {

  idt[index].offset_low = handler_ptr & 0xFFFF;
  idt[index].selector = selector; // code segment selector to GDT, usually 0x08
  idt[index].ist = 0;
  idt[index].type_attr = flags;
  idt[index].offset_mid = (handler_ptr >> 16) & 0xFFFF;
  idt[index].offset_high = (handler_ptr >> 32) & 0xFFFFFFFF;
  idt[index].zero = 0;
}

static inline bool are_interrupts_enabled() {
  unsigned long flags;
  asm volatile("pushf\n\t"
               "pop %0"
               : "=g"(flags));
  return flags & (1 << 9);
}
void idt_init() {
  idt_ptr.limit = (sizeof(IDTEntry) * 256) - 1;
  idt_ptr.base = (uint64_t)&idt;

  set_idt_gate(0, (uint64_t)isr_divide_error, KERNEL_CODE_SEGMENT,
               RING_0_64_PRESENT_GATE);

  set_idt_gate(60, (uint64_t)test_isr, KERNEL_CODE_SEGMENT,
               RING_0_64_PRESENT_GATE);

  asm volatile("lidt %0" : : "m"(idt_ptr));
  if (are_interrupts_enabled()) {
    serial_write("Interrupts enabled");
  } else {
    serial_write("Interrutps not enabled");
  }
}

__attribute__((interrupt)) void isr_divide_error(interrupt_frame *frame) {
  (void)frame;
  serial_write("Divide by zero!\n");
}

__attribute__((interrupt)) void test_isr(interrupt_frame *frame) {
  (void)frame;
  serial_write("Test interrupt");
}
