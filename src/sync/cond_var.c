#include "sync/cond_var.h"
#include "task/scheduler.h"

extern uint32 atomic_exchange(volatile uint32* dst, uint32 src);

void cond_var_init(cond_var_t* cv) {
  linked_list_init(&cv->waiting_task_queue);
}

void cond_var_wait(cond_var_t* cv, yieldlock_t* lock, cv_predicator_func predicator) {
  yieldlock_lock(lock);
  while (predicator != nullptr && predicator() == false) {
    // Add current thread to wait queue.
    thread_node_t* thread_node = get_crt_thread_node();
    linked_list_append(&cv->waiting_task_queue, thread_node);
    schedule_mark_thread_block();
    yieldlock_unlock(lock);
    schedule_thread_yield();

    // Waken up, and test condition predicator again.
    yieldlock_lock(lock);
  }
  yieldlock_unlock(lock);
}

void cond_var_notify(cond_var_t* cv) {
  if (cv->waiting_task_queue.size != 0) {
    // Wake up waiting thread.
    thread_node_t* head = cv->waiting_task_queue.head;
    linked_list_remove(&cv->waiting_task_queue, head);
    add_thread_node_to_schedule(head);
  }
}
