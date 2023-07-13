#include "driver/hard_disk.h"
#include "mem/kheap.h"
#include "monitor/monitor.h"
#include "common/stdlib.h"
#include "utils/math.h"
#include "interrupt/interrupt.h"

static void disk_interrupt_handler() {}

void init_hard_disk() {
  // Ignore disk interrupt.
  register_interrupt_handler(IRQ14_INT_NUM, &disk_interrupt_handler);
  register_interrupt_handler(IRQ15_INT_NUM, &disk_interrupt_handler);
}

extern void read_disk(char* buffer, uint32 start_sector, uint32 sector_num);

static void read_sector(char* buffer, uint32 sector) {
  read_disk(buffer, sector, 1);
}

void read_hard_disk(char* buffer, uint32 start, uint32 length) {
  uint32 end = start + length;

  uint32 start_sector = start / SECTOR_SIZE;
  uint32 end_sector = (end - 1) / SECTOR_SIZE + 1;

  // Do NOT allocate buffer on kernel stack!
  char* sector_buffer = (char*)kmalloc(SECTOR_SIZE);

  for (uint32 i = start_sector; i < end_sector; i++) {
    read_sector(sector_buffer, i);

    uint32 copy_start_addr = max(i * SECTOR_SIZE, start);
    uint32 copy_end_addr = min((i + 1) * SECTOR_SIZE, end);
    uint32 copy_size = copy_end_addr - copy_start_addr;
    memcpy(buffer, sector_buffer + copy_start_addr - i * SECTOR_SIZE, copy_size);
    buffer += (copy_size);
  }

  kfree(sector_buffer);
}

