#include "elf.h"
#include "drivers/serial.h"
#include "mem/kalloc.h"
#include "mem/mem.h"
#include "req.h"
#include <stdbool.h>

// load elf into virt addr space
bool load_elf(void *file, loaded_elf *out, u64 *pml4) {
  Elf64_Ehdr *hdr = (Elf64_Ehdr *)file;
  Elf64_Phdr *phdrs = (Elf64_Phdr *)((u8 *)file + hdr->e_phoff);

  if (hdr->e_ident[EI_MAG0] != 0x7F || hdr->e_ident[EI_MAG1] != 'E' ||
      hdr->e_ident[EI_MAG2] != 'L' || hdr->e_ident[EI_MAG3] != 'F') {
    serial_fstring("Not a valid ELF file\n");
    return false;
  }

  out->entry_addr = hdr->e_entry;
  out->segment_count = 0;

  serial_fstring("e_entry = {uint}\n", hdr->e_entry);
  serial_fstring("e_phoff = {uint}\n", hdr->e_phoff);
  serial_fstring("e_phnum = {uint}\n", hdr->e_phnum);

  for (int i = 0; i < hdr->e_phnum; i++) {
    Elf64_Phdr *ph = &phdrs[i];
    if (ph->p_type != PT_LOAD)
      continue;

    u64 pages_needed = (ph->p_memsz + FRAME_SIZE - 1) / FRAME_SIZE;

    for (u64 p = 0; p < pages_needed; p++) {
      void *frame = pmm_alloc_frame(); // virtual addr
      memset(frame, 0, FRAME_SIZE);

      u64 phys = virt_to_phys(frame);
      u64 virt = ph->p_vaddr + (p * FRAME_SIZE);

      serial_fstring("p_vaddr = {uint}\n", ph->p_vaddr);
      map_page(pml4, virt, phys, PTE_PRESENT | PTE_WRITE | PTE_USER);

      u64 offset_in_segment = p * FRAME_SIZE;
      if (offset_in_segment < ph->p_filesz) {
        u64 copy_size = ph->p_filesz - offset_in_segment;
        if (copy_size > FRAME_SIZE)
          copy_size = FRAME_SIZE;
        memcpy(frame, (u8 *)file + ph->p_offset + offset_in_segment, copy_size);
      }
    }

    out->segments[out->segment_count++] = (void *)ph->p_vaddr;

    serial_fstring(
        "Mapped segment {uint}: vaddr=0x{hex}, memsz={uint}, pages={uint}\n", i,
        ph->p_vaddr, ph->p_memsz, pages_needed);
  }

  kfree(file);
  return true;
}
