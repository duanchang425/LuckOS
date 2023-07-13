#include "common/stdio.h"
#include "monitor/monitor.h"
#include "task/thread.h"
#include "task/process.h"
#include "task/scheduler.h"
#include "syscall/syscall.h"
#include "interrupt/interrupt.h"
#include "mem/gdt.h"
#include "mem/kheap.h"
#include "mem/paging.h"
#include "sync/yieldlock.h"
#include "sync/cond_var.h"
#include "utils/linked_list.h"
#include "utils/hash_table.h"
#include "utils/debug.h"

extern void cpu_idle();
extern void context_switch(tcb_t* crt, tcb_t* next);
extern void resume_thread();

// ****************************************************************************
static pcb_t* main_process;
static thread_node_t* main_thread_node;
static thread_node_t* kernel_clean_node;

static thread_node_t* crt_thread_node = nullptr;

// processes map
static hash_table_t processes_map;
static yieldlock_t processes_map_lock;

// dead tasks and processes waiting for clean
static linked_list_t dead_tasks;
static linked_list_t dead_processes;
static yieldlock_t dead_resource_lock;
static cond_var_t dead_resource_cv;

// threads map
static hash_table_t threads_map;
static yieldlock_t threads_map_lock;

// ready task queue
static linked_list_t ready_tasks;

static bool main_thread_in_ready_queue = false;

static bool multi_task_enabled = false;

// *************************************************************************************************
static void kernel_main_thread();
static void kernel_clean_thread();
static void kernel_init_thread();

static bool has_dead_resource();

bool multi_task_is_enabled() {
  return multi_task_enabled;
}

void init_scheduler() {
  disable_interrupt();

  linked_list_init(&ready_tasks);

  hash_table_init(&processes_map);
  yieldlock_init(&processes_map_lock);

  hash_table_init(&threads_map);
  yieldlock_init(&threads_map_lock);

  linked_list_init(&dead_tasks);
  linked_list_init(&dead_processes);
  yieldlock_init(&dead_resource_lock);
  cond_var_init(&dead_resource_cv);

  // Create process 0: kernel main process (cpu idle)
  main_process = create_process("kernel_main_process", /* is_kernel_process = */true);
  tcb_t* main_thread = create_new_kernel_thread(main_process, "kernel main", kernel_main_thread);
  main_thread_node = (thread_node_t*)kmalloc(sizeof(thread_node_t));
  main_thread_node->ptr = main_thread;
  crt_thread_node = main_thread_node;

  // Kick off!
  asm volatile (
   "movl %0, %%esp; \
    jmp resume_thread": : "g" (main_thread->kernel_esp) : "memory");

  // Never should reach here!
  PANIC();
}

// Kernel main thread:
//  - Create the resource clean thread;
//  - Create process 1 (init process) which will later become the first user process;
//  - Becomes the cpu idle thread;
static void kernel_main_thread() {
  // Create kernel clean thread.
  tcb_t* clean_thread = create_new_kernel_thread(main_process, "kernel clean", kernel_clean_thread);
  kernel_clean_node = (thread_node_t*)kmalloc(sizeof(thread_node_t));
  kernel_clean_node->ptr = clean_thread;
  add_thread_node_to_schedule(kernel_clean_node);

  // Create process 1: init process.
  pcb_t* init_process = create_process(nullptr, /* is_kernel_process = */true);
  tcb_t* init_thread = create_new_kernel_thread(init_process, "kernel init", kernel_init_thread);
  add_thread_to_schedule(init_thread);

  // Enable interrupt: multi tasks start running after this line.
  multi_task_enabled = true;
  enable_interrupt();

  // Enter cpu idle.
  while (true) {
    cpu_idle();
  }
}

static void kernel_clean_thread() {
  // thread-1
  while (true) {
    // Wait for dead resource to clean.
    cond_var_wait(&dead_resource_cv, &dead_resource_lock, has_dead_resource);
    linked_list_t dead_tasks_receiver;
    linked_list_t dead_processes_receiver;
    linked_list_move(&dead_tasks_receiver, &dead_tasks);
    linked_list_move(&dead_processes_receiver, &dead_processes);
    yieldlock_unlock(&dead_resource_lock);

    if (dead_tasks_receiver.size > 0) {
      // Clean dead task struct.
      while (dead_tasks_receiver.size > 0) {
        linked_list_node_t* head = dead_tasks_receiver.head;
        linked_list_remove(&dead_tasks_receiver, head);
        tcb_t* thread = (tcb_t*)head->ptr;
        //monitor_printf("clean thread %d\n", thread->id);
        destroy_thread(thread);
        kfree(head);
      }
    }

    if (dead_processes_receiver.size > 0) {
      // Clean dead process struct.
      while (dead_processes_receiver.size > 0) {
        linked_list_node_t* head = dead_processes_receiver.head;
        linked_list_remove(&dead_processes_receiver, head);
        pcb_t* process = (pcb_t*)head->ptr;
        //monitor_printf("clean process %d\n", process->id);
        destroy_process(process);
        kfree(head);
      }
    }

    schedule_thread_yield();
  }
}

static bool has_dead_resource() {
  return dead_processes.size > 0 || dead_tasks.size > 0;
}

static void kernel_init_thread() {
  // thread-2
  //monitor_println("start init process ...");

  // Change this process to user process.
  pcb_t* crt_process = get_crt_thread()->process;
  crt_process->is_kernel_process = false;

  // Load and run init program.
  // thread-3
  process_exec("init", 0, nullptr);
}


// *************************************************************************************************
tcb_t* get_crt_thread() {
  if (crt_thread_node == nullptr) {
    return nullptr;
  }
  return (tcb_t*)(crt_thread_node->ptr);
}

thread_node_t* get_crt_thread_node() {
  return crt_thread_node;
}

pcb_t* get_crt_process() {
  tcb_t* thread = get_crt_thread();
  if (thread == nullptr) {
    return nullptr;
  }
  return thread->process;
}

bool is_kernel_main_thread() {
  return crt_thread_node == main_thread_node;
}

void add_new_process(pcb_t* process) {
  yieldlock_lock(&processes_map_lock);
  hash_table_put(&processes_map, process->id, process);
  yieldlock_unlock(&processes_map_lock);
}

void add_dead_process(pcb_t* process) {
  yieldlock_lock(&dead_resource_lock);
  linked_list_append_ele(&dead_processes, process);
  cond_var_notify(&dead_resource_cv);
  yieldlock_unlock(&dead_resource_lock);
}

void add_dead_task(tcb_t* thread) {
  yieldlock_lock(&dead_resource_lock);
  linked_list_append_ele(&dead_tasks, thread);
  cond_var_notify(&dead_resource_cv);
  yieldlock_unlock(&dead_resource_lock);
}

static void process_switch(pcb_t* process) {
  if (process == nullptr) {
    process = main_process;
  }
  reload_page_directory(&process->page_dir);
}

// Note: interrupt must be DISABLED before entering this function.
static void do_context_switch() {
  //monitor_printf("ready_tasks num = %d\n", ready_tasks.size);
  tcb_t* old_thread = get_crt_thread();

  thread_node_t* head = ready_tasks.head;
  linked_list_remove(&ready_tasks, head);
  tcb_t* next_thread = (tcb_t*)head->ptr;

  // Switch out current running thread.
  if (old_thread->status == TASK_RUNNING && crt_thread_node != main_thread_node) {
    old_thread->status = TASK_READY;
    linked_list_append(&ready_tasks, crt_thread_node);
  }
  old_thread->ticks = 0;
  old_thread->need_reschedule = false;

  // Switch in next thread.
  next_thread->status = TASK_RUNNING;
  crt_thread_node = head;
  if (head == main_thread_node) {
    main_thread_in_ready_queue = 0;
  }

  // Setup env for next thread (and maybe a different process)
  update_tss_esp(next_thread->kernel_stack + KERNEL_STACK_SIZE);
  if (old_thread->process != next_thread->process) {
    process_switch(next_thread->process);
  }

  //monitor_printf("thread %u switch to thread %u\n", old_thread->id, next_thread->id);
  context_switch(old_thread, next_thread);
}

void schedule() {
  disable_interrupt();

  tcb_t* crt_thread = get_crt_thread();
  if (crt_thread->preempt_count > 0) {
    // preemption is disabled.
    return;
  }
  bool need_context_switch = false;
  if (ready_tasks.size > 0) {
    need_context_switch = crt_thread->need_reschedule;
  }

  if (need_context_switch) {
    //monitor_printf("context_switch yes, %d ready tasks\n", ready_tasks.size);
    do_context_switch();
  } else {
    //monitor_println("context_switch no");
    enable_interrupt();
  }
}

void add_thread_to_schedule(tcb_t* thread) {
  thread_node_t* node = (thread_node_t*)kmalloc(sizeof(thread_node_t));
  node->ptr = (void*)thread;
  add_thread_node_to_schedule(node);
}

void add_thread_node_to_schedule(thread_node_t* thread_node) {
  disable_interrupt();
  tcb_t* thread = (tcb_t*)thread_node->ptr;
  if (thread->status != TASK_DEAD) {
    thread->status = TASK_READY;
  }
  linked_list_append(&ready_tasks, thread_node);
  enable_interrupt();
}

void add_thread_node_to_schedule_head(thread_node_t* thread_node) {
  disable_interrupt();
  tcb_t* thread = (tcb_t*)thread_node->ptr;
  thread->status = TASK_READY;
  linked_list_insert_to_head(&ready_tasks, thread_node);
  enable_interrupt();
}

void schedule_thread_yield() {
  disable_interrupt();

  //monitor_printf("thread %d yield\n", get_crt_thread()->id);

  // If no ready task in queue, wake up kernel main (cpu idle) thread.
  if (ready_tasks.size == 0) {
    linked_list_append(&ready_tasks, main_thread_node);
    main_thread_in_ready_queue = 1;
  }

  do_context_switch();
}

void schedule_mark_thread_block() {
  tcb_t* thread = (tcb_t*)crt_thread_node->ptr;
  thread->status = TASK_WAITING;
}

void schedule_thread_exit() {
  // Detach this thread from its process.
  tcb_t* thread = get_crt_thread();
  pcb_t* process = thread->process;
  if (process != nullptr) {
    remove_process_thread(process, thread);
  }

  thread->status = TASK_DEAD;
  add_dead_task(thread);

  // Mark this thread TASK_dead.
  disable_interrupt();
  do_context_switch();
}

void schedule_thread_exit_normal() {
  //tcb_t* thread = get_crt_thread();
  //pcb_t* process = thread->process;
  //monitor_printf("process %d thread %d exit\n", process->id, thread->id);
  thread_exit();
}

void disable_preempt() {
  get_crt_thread()->preempt_count += 1;
}

void enable_preempt() {
  get_crt_thread()->preempt_count -= 1;
}
