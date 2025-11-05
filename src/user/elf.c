#include "elf.h"
#include "drivers/serial.h"
#include "mem/kalloc.h"
#include "req.h"
#include "terminal.h"

void exec_elf(void *file) {
  Elf64_Ehdr *hdr = (Elf64_Ehdr *)file;
  Elf64_Phdr *phdrs = (Elf64_Phdr *)((u8 *)file + hdr->e_phoff);
  serial_write_fstring("executing ELF file...\n");

  if (hdr->e_ident[EI_MAG0] != 0x7F || hdr->e_ident[EI_MAG1] != 'E' ||
      hdr->e_ident[EI_MAG2] != 'L' || hdr->e_ident[EI_MAG3] != 'F') {
    serial_write_fstring("Not a valid ELF file\n");
    terminal_fstring("Not a valid ELF file\n");
    return;
  }

  u64 entry_addr = 0;

  for (int i = 0; i < hdr->e_phnum; i++) {
    Elf64_Phdr *ph = &phdrs[i];

    if (ph->p_type != PT_LOAD)
      continue;

    void *segment = kalloc(ph->p_memsz);
    memcpy(segment, (u8 *)file + ph->p_offset, ph->p_filesz);

    // zero the .bss memory
    if (ph->p_memsz > ph->p_filesz)
      memset((u8 *)segment + ph->p_filesz, 0, ph->p_memsz - ph->p_filesz);

    // calculate address
    if (hdr->e_entry >= ph->p_vaddr &&
        hdr->e_entry < ph->p_vaddr + ph->p_memsz) {
      u64 delta = (u64)segment - ph->p_vaddr;
      entry_addr = hdr->e_entry + delta;
    }

    serial_write_fstring(
        "Loaded segment {uint}: p_vaddr=0x{uint}, p_memsz={uint}\n", i,
        ph->p_vaddr, ph->p_memsz);

    //  TODO: Process structrues for freeing the segments safely
    // kfree((u64)segment);
  }

  if (entry_addr == 0) {
    serial_write_fstring("Error: entry point not in any PT_LOAD segment\n");
    return;
  }

  serial_write_fstring("Jumping to entry point at 0x{hex}\n", entry_addr);
  ((void (*)())entry_addr)();
}
