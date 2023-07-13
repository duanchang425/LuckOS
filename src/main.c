#include "monitor/monitor.h"
#include "interrupt/timer.h"
#include "interrupt/interrupt.h"
#include "mem/gdt.h"
#include "mem/paging.h"
#include "mem/kheap.h"
#include "task/thread.h"
#include "task/process.h"
#include "task/scheduler.h"
#include "fs/vfs.h"
#include "driver/hard_disk.h"
#include "driver/keyboard.h"
#include "utils/debug.h"
#include "utils/id_pool.h"

char* welcome = " welcome to scroll kernel\n";

static void print_welcome() {
  monitor_print_with_color("#", COLOR_GREEN);
  monitor_printf(welcome);
}

int main() {
  init_gdt();

  monitor_init();
  monitor_clear();
  print_welcome();

  init_idt();
  init_timer(TIMER_FREQUENCY);

  init_paging();
  init_kheap();
  init_paging_stage2();

  init_hard_disk();
  init_file_system();

  init_keyboard();

  init_task_manager();
  init_process_manager();
  init_scheduler();

  // Never should reach here.
  PANIC();
}
