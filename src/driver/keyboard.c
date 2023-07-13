#include "driver/keyboard.h"
#include "driver/keyhelp.h"
#include "common/io.h"
#include "common/stdlib.h"
#include "monitor/monitor.h"
#include "sync/spinlock.h"
#include "task/scheduler.h"
#include "interrupt/interrupt.h"
#include "utils/debug.h"
#include "utils/linked_list.h"

#define KEYBOARD_BUF_SIZE 1024

typedef struct {
  uint8 buffer[KEYBOARD_BUF_SIZE];
  int head;
  int tail;
  int size;
} buffer_queue_t;

static buffer_queue_t queue;

static linked_list_t waiting_tasks;

static spinlock_t keyboard_lock;

static int next_index(int index) {
  return (index + 1) % KEYBOARD_BUF_SIZE;
}

static int enqueue(uint8 scanCode) { 
  if (queue.size == KEYBOARD_BUF_SIZE) {
    return -1;
  }

  // insert the new scancode
  queue.buffer[queue.tail] = scanCode;
  queue.tail = next_index(queue.tail);
  queue.size++;
  return 0;
}

static uint8 dequeue() {
  uint8 code = queue.buffer[queue.head];
  queue.head = next_index(queue.head);
  queue.size--;
  return code;
}

static int32 read_keyboard_char_impl() {
  if (queue.size == 0) {
    return -1;
  }

  int32 augchar = process_scancode((int)dequeue());
  while (!(KH_HASDATA(augchar) && KH_ISMAKE(augchar))) {
    if (queue.size == 0) {
      return -1;
    }
    augchar = process_scancode((int)dequeue());
  }
  return KH_GETCHAR(augchar);
}

int32 read_keyboard_char() {
  spinlock_lock_irqsave(&keyboard_lock);
  int32 c;
  while (1) {
    c = read_keyboard_char_impl();
    if (c != -1) {
      break;
    }
    //monitor_printf("keyboard waiting thread %u\n", get_crt_thread()->id);
    linked_list_append(&waiting_tasks, get_crt_thread_node());
    schedule_mark_thread_block();
    spinlock_unlock_irqrestore(&keyboard_lock);
    schedule_thread_yield();

    spinlock_lock_irqsave(&keyboard_lock);
  }
  spinlock_unlock_irqrestore(&keyboard_lock);
  return c;
}

static void keyboard_interrupt_handler() {
  uint8 scancode = inb(0x60);

  spinlock_lock(&keyboard_lock);
  enqueue(scancode);
  linked_list_t waiting_tasks_get;
  linked_list_move(&waiting_tasks_get, &waiting_tasks);
  spinlock_unlock(&keyboard_lock);

  thread_node_t* node = waiting_tasks_get.head;
  while (node != nullptr) {
    thread_node_t* next_node = node->next;
    linked_list_remove(&waiting_tasks_get, node);
    //monitor_printf("wake up keyboard waiting thread %u\n", ((tcb_t*)node->ptr)->id);
    add_thread_node_to_schedule_head(node);
    //break;
    node = next_node;
  }

  // Activate waiting thread immediately.
  get_crt_thread()->need_reschedule = true;
}

void init_keyboard() {
  spinlock_init(&keyboard_lock);
  linked_list_init(&waiting_tasks);

  // Explicitly set queue memory to avoid page fault for this part of memory.
  memset(&queue, 0, sizeof(buffer_queue_t));
  queue.head = 0;
  queue.tail = 0;
  queue.size = 0;

  register_interrupt_handler(IRQ1_INT_NUM, &keyboard_interrupt_handler);
}
