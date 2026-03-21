#include "lykosapi.h"
#include "arch/x86_64/idt.h"
#include "drivers/serial.h"
#include "graphics/draw.h"
#include "mem/mem.h"
#include "proc/task.h"
#include "terminal.h"

i64 sys_exit(interrupt_frame *frame) {
  // TODO: use frame for reason why sys exit e.g success failure etc
  (void)frame;
  task_exit();
  return 1;
}

i64 sys_sleep(interrupt_frame *frame) {
  u64 ms = frame->rdi;
  serial_fstring("Sleeping by syscall for {uint} ms\n", ms);
  task_sleep(ms);
  return 1;
}

i64 sys_write(interrupt_frame *frame) {
  char *str = (char *)frame->rdi;
  terminal_fstring("{str}", str);
  serial_writeln(str);
  return 1;
}

i64 map_fb(interrupt_frame *frame) {
  (void)frame;
  u64 fb_vaddr = 0xFB000000;
  limine_framebuffer fb = *get_framebuffer();
  u64 fb_phys_addr = virt_to_phys(fb.address);
  u64 fb_size = fb.width * fb.height * sizeof(u32);

  Task *t = &tasks[current_task];
  u64 proc_cr3 = t->user_cr3;

  // TODO: look into bigger pages, for now just using smallest
  u64 pages_needed = ALIGN_UP(fb_size, PAGE_SIZE) / PAGE_SIZE;

  u64 flags = PTE_PRESENT | PTE_WRITE | PTE_USER | PTE_WRITE_THROUGH;
  for (uint64_t i = 0; i < pages_needed; i++) {
    uint64_t offset = i * PAGE_SIZE;
    serial_fstring("map fb {uint}\n", i);
    map_page(phys_to_virt(proc_cr3), fb_vaddr + offset, fb_phys_addr + offset,
             flags);
  }

  return fb_vaddr;
}
