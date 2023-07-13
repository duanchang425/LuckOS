#include "interrupt/interrupt.h"
#include "sync/spinlock.h"
#include "task/thread.h"
#include "task/scheduler.h"
#include "utils/debug.h"

extern uint32 atomic_exchange(volatile uint32* dst, uint32 src);
extern uint32 get_eflags();

void spinlock_init(spinlock_t* splock) {
  splock->hold = LOCKED_NO;
  splock->interrupt_mask = 0;
}

// This lock should NOT be used for interrupt handling.
void spinlock_lock(spinlock_t *splock) {
  // Disable preempt on local cpu.
  disable_preempt();

  // For uni-processor, disabling preeempt is sufficient to to ensure mutual exclusive.
  // For multi-processor however, competing for CAS is still needed.
  #ifndef SINGLE_PROCESSOR
  while (atomic_exchange(&splock->hold , LOCKED_YES) != LOCKED_NO) {}
  #endif
}

// This lock disables local interrupt, and can be used for interrupt handling.
void spinlock_lock_irqsave(spinlock_t *splock) {
  // First disable preempt.
  disable_preempt();

  // Now disable local interrupt and save interrupt flag bit.
  uint32 eflags = get_eflags();
  disable_interrupt();
  splock->interrupt_mask = (eflags & (1 << 9));

  // For multi-processor, competing for CAS is still needed.
  #ifndef SINGLE_PROCESSOR
  while (atomic_exchange(&splock->hold , LOCKED_YES) != LOCKED_NO) {}
  #endif
}

void spinlock_unlock(spinlock_t *splock) {
  #ifndef SINGLE_PROCESSOR
  splock->hold = LOCKED_NO;
  #endif

  enable_preempt();
}

void spinlock_unlock_irqrestore(spinlock_t *splock) {
  #ifndef SINGLE_PROCESSOR
  splock->hold = LOCKED_NO;
  #endif

  enable_preempt();

  // Restore interrupt flag bit - If it's previously enabled before locking, re-enable it again.
  if (splock->interrupt_mask) {
    enable_interrupt();
  }
}
