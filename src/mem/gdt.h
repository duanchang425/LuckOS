#ifndef MEM_GDT_H
#define MEM_GDT_H

#include "common/common.h"

// ****************************************************************************
#define FLAG_G_4K  (1 << 3)
#define FLAG_D_32  (1 << 2)

#define DESC_P     (1 << 7)

#define DESC_DPL_0   (0 << 5)
#define DESC_DPL_1   (1 << 5)
#define DESC_DPL_2   (2 << 5)
#define DESC_DPL_3   (3 << 5)

#define DESC_S_CODE   (1 << 4)
#define DESC_S_DATA   (1 << 4)
#define DESC_S_SYS    (0 << 4)

#define DESC_TYPE_CODE  0x8   // r/x non-conforming code segment
#define DESC_TYPE_DATA  0x2   // r/w data segment
#define DESC_TYPE_TSS   0x9

// ****************************************************************************

#define RPL0 0
#define RPL1 1
#define RPL2 2
#define RPL3 3

#define TI_GDT 0
#define TI_LDT 1

#define SELECTOR_K_CODE   ((1 << 3) + (TI_GDT << 2) + RPL0)
#define SELECTOR_K_DATA   ((2 << 3) + (TI_GDT << 2) + RPL0)
#define SELECTOR_K_STACK  SELECTOR_K_DATA
#define SELECTOR_K_GS     ((3 << 3) + (TI_GDT << 2) + RPL0)  // video segment
#define SELECTOR_U_CODE   ((4 << 3) + (TI_GDT << 2) + RPL3)
#define SELECTOR_U_DATA   ((5 << 3) + (TI_GDT << 2) + RPL3)


// ****************************************************************************
struct gdt_ptr {
  uint16 limit;
  uint32 base;
} __attribute__((packed));
typedef struct gdt_ptr gdt_ptr_t;

struct gdt_entry {
  uint16 limit_low;
  uint16 base_low;
  uint8  base_middle;
  uint8  access;
  uint8  attributes;
  uint8  base_high;
} __attribute__((packed));
typedef struct gdt_entry gdt_entry_t;

// TSS entry
struct tss_entry_struct {
  uint32 prev_tss;
  uint32 esp0;
  uint32 ss0;
  uint32 esp1;
  uint32 ss1;
  uint32 esp2;
  uint32 ss2;
  uint32 cr3;
  uint32 eip;
  uint32 eflags;
  uint32 eax;
  uint32 ecx;
  uint32 edx;
  uint32 ebx;
  uint32 esp;
  uint32 ebp;
  uint32 esi;
  uint32 edi;
  uint32 es;
  uint32 cs;
  uint32 ss;
  uint32 ds;
  uint32 fs;
  uint32 gs;
  uint32 ldt;
  uint16 trap;
  uint16 iomap_base;
} __attribute__((packed));
typedef struct tss_entry_struct tss_entry_t;


// ****************************************************************************
void init_gdt();

void update_tss_esp(uint32 esp);

#endif
