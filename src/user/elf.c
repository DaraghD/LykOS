#include "elf.h"
#include "drivers/serial.h"
#include "mem/kalloc.h"
#include "mem/mem.h"
#include "req.h"
#include <stdbool.h>

// load elf into virt addr space
bool load_elf(Task *t, void *file, loaded_elf *out, u64 *pml4) {
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

    // align start down to page boundary
    u64 seg_start = ph->p_vaddr & ~(FRAME_SIZE - 1);
    u64 seg_end = ph->p_vaddr + ph->p_memsz;
    u64 pages_needed = (seg_end - seg_start + FRAME_SIZE - 1) / FRAME_SIZE;

    for (u64 p = 0; p < pages_needed; p++) {
      void *frame = pmm_alloc_frame(); // virtual addr
      memset(frame, 0, FRAME_SIZE);

      u64 phys = virt_to_phys(frame);
      u64 virt = seg_start + (p * FRAME_SIZE);

      serial_fstring("p_vaddr = {uint}\n", ph->p_vaddr);
      map_page(pml4, virt, phys, PTE_PRESENT | PTE_WRITE | PTE_USER);

      // figure out overlap between this page and the file
      u64 page_start = virt;
      u64 page_end = virt + FRAME_SIZE;

      u64 file_start = ph->p_vaddr;
      u64 file_end = ph->p_vaddr + ph->p_filesz;

      if (page_start < file_end && page_end > file_start) {
        u64 start = (page_start > file_start) ? page_start : file_start;
        u64 end = (page_end < file_end) ? page_end : file_end;

        memcpy((u8 *)frame + (start - page_start),
               (u8 *)file + ph->p_offset + (start - file_start), end - start);
      }
    }

    out->segments[out->segment_count++] = (void *)ph->p_vaddr;
    add_vma(t, seg_start, seg_start + (pages_needed * FRAME_SIZE));

    serial_fstring(
        "Mapped segment {uint}: vaddr=0x{hex}, memsz={uint}, pages={uint}\n", i,
        ph->p_vaddr, ph->p_memsz, pages_needed);
  }

  kfree(file);
  return true;
}
