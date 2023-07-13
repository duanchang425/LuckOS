#ifndef TASK_SCHEDULER_H
#define TASK_SCHEDULER_H

#include "task/thread.h"

// Init scheduler.
void init_scheduler();

// Get current running thread.
tcb_t* get_crt_thread();
thread_node_t* get_crt_thread_node();
pcb_t* get_crt_process();

// If current running thread is kernel main thread.
bool is_kernel_main_thread();

// Add thread to ready task queue and wait for schedule.
void add_thread_to_schedule(struct task_struct* thread);
void add_thread_node_to_schedule(thread_node_t* thread_node);
void add_thread_node_to_schedule_head(thread_node_t* thread_node);

// Call scheduler.
void schedule();

// Yield thread - give up cpu and move current thread to ready queue tail.
void schedule_thread_yield();

// Mark current thread WAITING.
void schedule_mark_thread_block();

void schedule_thread_exit();
void schedule_thread_exit_normal();

// Add process to scheduler
void add_new_process(pcb_t* process);
void add_dead_process(pcb_t* process);

bool multi_task_is_enabled();

void disable_preempt();
void enable_preempt();

#endif
