#include "elf/elf.h"
#include "common/stdlib.h"
#include "monitor/monitor.h"

int32 load_elf(char* content, uint32* entry_addr) {
  elf32_ehdr_t* elf_header = (elf32_ehdr_t*)content;

  // Verify magic number
  if (elf_header->e_ident[0] != 0x7f) {
    return -1;
  }
  if (elf_header->e_ident[1] != 'E') {
    return -1;
  }
  if (elf_header->e_ident[2] != 'L') {
    return -1;
  }
  if (elf_header->e_ident[3] != 'F') {
    return -1;
  }

  // Load each section.
  elf32_phdr_t* program_header = (elf32_phdr_t*)(content + elf_header->e_phoff);
  for (uint32 i = 0; i < elf_header->e_phnum; i++) {
    if (program_header->p_type == 0) {
      continue;
    }

    //monitor_printf("load section to vaddr %x, offset = %d, size = %d\n",
    //    program_header->p_vaddr, program_header->p_offset, program_header->p_filesz);
    memcpy((void*)program_header->p_vaddr, content + program_header->p_offset,
        program_header->p_filesz);
    program_header = (elf32_phdr_t*)((uint32)program_header + elf_header->e_phentsize);
  }

  *entry_addr = elf_header->e_entry;
  return 0;
}
