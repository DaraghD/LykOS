#pragma once

#include "req.h"
#include <stdint.h>

#define PT_LOAD 1
#define EI_MAG0 0
#define EI_MAG1 1
#define EI_MAG2 2
#define EI_MAG3 3
#define EI_CLASS 4
#define ELFCLASS64 2
#define EM_X86_64 62
#define ET_EXEC 2

typedef struct {
  unsigned char e_ident[16]; // Magic number + other info
  u16 e_type;                // Object file type (executable, relocatable, etc.)
  u16 e_machine;             // Architecture (should be 62 for x86-64)
  u32 e_version;             // ELF version
  u64 e_entry;               // Entry point virtual address
  u64 e_phoff;               // Program header table offset (in bytes)
  u64 e_shoff;     // Section header table offset (we can ignore this for now)
  u32 e_flags;     // Processor-specific flags
  u16 e_ehsize;    // ELF header size
  u16 e_phentsize; // Size of one program header
  u16 e_phnum;     // Number of program headers
  u16 e_shentsize; // Size of one section header
  u16 e_shnum;     // Number of section header
  u16 e_shstrndx;  // Section header string table index
} __attribute__((packed)) Elf64_Ehdr;

typedef struct {
  u32 p_type;   // Segment type
  u32 p_flags;  // Segment flags
  u64 p_offset; // Offset in file
  u64 p_vaddr;  // Virtual address in memory
  u64 p_paddr;  // Physical address (ignore for now)
  u64 p_filesz; // Size of segment in file
  u64 p_memsz;  // Size of segment in memory
  u64 p_align;  // Alignment
} __attribute__((packed)) Elf64_Phdr;

void exec_elf(void *file);
