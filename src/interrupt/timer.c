#include "common/io.h"
#include "monitor/monitor.h"
#include "interrupt/interrupt.h"
#include "interrupt/timer.h"
#include "task/thread.h"
#include "task/scheduler.h"

uint32 tick = 0;

uint32 getTick() {
  return tick;
}

static void timer_callback(isr_params_t regs) {
  if (tick % TIMER_FREQUENCY == 0) {
    //monitor_printf("second: %d\n", tick / TIMER_FREQUENCY);
  }
  tick++;

  // Check current thread time slice.
  tcb_t* crt_thread = get_crt_thread();
  crt_thread->ticks++;
  if (crt_thread->ticks >= crt_thread->priority) {
    crt_thread->need_reschedule = true;
  }
}

void init_timer(uint32 frequency) {
  // register our timer callback.
  register_interrupt_handler(IRQ0_INT_NUM, &timer_callback);

  // the divisor must be small enough to fit into 16-bits.
  uint32 divisor = 1193180 / frequency;

  // send the command byte.
  outb(0x43, 0x36);

  uint8 l = (uint8)(divisor & 0xFF);
  uint8 h = (uint8)((divisor>>8) & 0xFF);

  // Send the frequency divisor.
  outb(0x40, l);
  outb(0x40, h);
}
