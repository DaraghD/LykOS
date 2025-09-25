#include "pic.h"
#include "io.h"

#define ICW1_INIT 0x10
#define ICW1_ICW4 0x01
#define ICW4_8086 0x01
#define PIC1 0x20 /* IO base address for master PIC */
#define PIC2 0xA0 /* IO base address for slave PIC */
#define PIC1_COMMAND PIC1
#define PIC1_DATA (PIC1 + 1)
#define PIC2_COMMAND PIC2
#define PIC2_DATA (PIC2 + 1)

void pic_remap(void) {
  uint8_t a1 = inb(PIC1_DATA); // save masks
  uint8_t a2 = inb(PIC2_DATA);

  outb(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4);
  iowait();
  outb(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);
  iowait();

  outb(PIC1_DATA, 0x20); // master offset
  iowait();
  outb(PIC2_DATA, 0x28); // slave offset
  iowait();

  outb(PIC1_DATA, 4); // tell master about slave on IRQ2
  iowait();
  outb(PIC2_DATA, 2); // tell slave its cascade identity
  iowait();

  outb(PIC1_DATA, ICW4_8086);
  iowait();
  outb(PIC2_DATA, ICW4_8086);
  iowait();

  outb(PIC1_DATA, a1); // restore masks
  iowait();
  outb(PIC2_DATA, a2);
  iowait();

  outb(0x21, 0xFD); // only IRQ1 is unmasked
  outb(0xA1, 0xFF); // all slave IRQs masked
}
