#include "common/stdlib.h"
#include "common/global.h"
#include "monitor/monitor.h"
#include "mem/gdt.h"

extern void load_gdt(gdt_ptr_t*);
extern void refresh_tss();

static gdt_ptr_t gdt_ptr;
static gdt_entry_t gdt_entries[7];

static tss_entry_t tss_entry;

static void refresh_gdt() {
  load_gdt(&gdt_ptr);
}

static void gdt_set_gate(int32 num, uint32 base, uint32 limit, uint8 access, uint8 flags) {
  gdt_entries[num].limit_low = (limit & 0xFFFF);
  gdt_entries[num].base_low = (base & 0xFFFF);
  gdt_entries[num].base_middle = (base >> 16) & 0xFF;
  gdt_entries[num].access = access;
  gdt_entries[num].attributes = (limit >> 16) & 0x0F;
  gdt_entries[num].attributes |= ((flags << 4) & 0xF0);
  gdt_entries[num].base_high = (base >> 24) & 0xFF;
}

static void write_tss(uint32 num, uint16 ss0, uint32 esp0) {
  memset(&tss_entry, 0, sizeof(tss_entry_t));
  tss_entry.ss0 = ss0;
  tss_entry.esp0 = esp0;
  tss_entry.iomap_base = sizeof(tss_entry_t);

  // Here we set the cs, ss, ds, es, fs and gs entries in the TSS. These specify what
  // segments should be loaded when the processor switches to kernel mode. Therefore
  // they are just our normal kernel code/data segments - 0x08 and 0x10 respectively,
  // but with the last two bits set, making 0x0b and 0x13. The setting of these bits
  // sets the RPL (requested privilege level) to 3, meaning that this TSS can be used
  // to switch to kernel mode from ring 3.
  tss_entry.cs = SELECTOR_K_CODE | RPL3;
  tss_entry.ss = tss_entry.ds = tss_entry.es = tss_entry.fs = tss_entry.gs = SELECTOR_K_DATA | RPL3;

  uint32 base = (uint32)&tss_entry;
  uint32 limit = base + sizeof(tss_entry);
  gdt_set_gate(num, base, limit, DESC_P | DESC_DPL_0 | DESC_S_SYS | DESC_TYPE_TSS, 0x0);
}

void init_gdt() {
  gdt_ptr.limit = (sizeof(gdt_entry_t) * 7) - 1;
  gdt_ptr.base = (uint32)&gdt_entries;

  // reserved
  gdt_set_gate(0, 0, 0, 0, 0);

  // kernel code
  gdt_set_gate(1, 0, 0xFFFFF, DESC_P | DESC_DPL_0 | DESC_S_CODE | DESC_TYPE_CODE, FLAG_G_4K | FLAG_D_32);
  // kernel data
  gdt_set_gate(2, 0, 0xFFFFF, DESC_P | DESC_DPL_0 | DESC_S_DATA | DESC_TYPE_DATA, FLAG_G_4K | FLAG_D_32);
  // video: only 8 pages
  gdt_set_gate(3, 0, 7, DESC_P | DESC_DPL_0 | DESC_S_DATA | DESC_TYPE_DATA, FLAG_G_4K | FLAG_D_32);

  // user code
  gdt_set_gate(4, 0, 0xBFFFF, DESC_P | DESC_DPL_3 | DESC_S_CODE | DESC_TYPE_CODE, FLAG_G_4K | FLAG_D_32);
  // user data
  gdt_set_gate(5, 0, 0xBFFFF, DESC_P | DESC_DPL_3 | DESC_S_DATA | DESC_TYPE_DATA, FLAG_G_4K | FLAG_D_32);

  // tss: 
  write_tss(6, 0x10, 0x0);

  refresh_gdt((uint32)&gdt_ptr);
  refresh_tss();
}

void update_tss_esp(uint32 esp) {
  tss_entry.esp0 = esp;
}
