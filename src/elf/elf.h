#ifndef ELF_ELF_H
#define ELF_ELF_H

#include "common/common.h"

struct elf32_ehdr {
  uint8  e_ident[16];    // Magic number
  uint16 e_type;         // Object file type
  uint16 e_machine;      // Architecture
  uint32 e_version;      // Object file version
  uint32 e_entry;        // Entry point virtual address
  uint32 e_phoff;        // Program header table file offset
  uint32 e_shoff;        // Section header table file offset
  uint32 e_flags;        // Processor-specific flags
  uint16 e_ehsize;       // ELF header size in bytes
  uint16 e_phentsize;    // Program header table entry size
  uint16 e_phnum;        // Program header table entry count
  uint16 e_shentsize;    // Section header table entry size
  uint16 e_shnum;        // Section header table entry count
  uint16 e_shstrndx;     // Section header string table index
};
typedef struct elf32_ehdr elf32_ehdr_t;

struct elf32_phdr {
  uint32 p_type;
  uint32 p_offset;
  uint32 p_vaddr;
  uint32 p_paddr;
  uint32 p_filesz;
  uint32 p_memsz;
  uint32 p_flags;
  uint32 p_align;
};
typedef struct elf32_phdr elf32_phdr_t;


// ****************************************************************************
int32 load_elf(char* content, uint32* entry_addr);

#endif
