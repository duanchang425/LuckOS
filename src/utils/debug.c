#include "monitor/monitor.h"
#include "interrupt/interrupt.h"
#include "utils/debug.h"

void oh_panic(const char* file, const char* func, int line) {
  disable_interrupt();
  monitor_print_with_color("PANIC!", COLOR_LIGHT_RED);
  monitor_printf(" %s, %s(), line %d\n", file, func, line);
  while (1) {}
}
