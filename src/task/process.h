#ifndef TASK_PROCESS_H
#define TASK_PROCESS_H

#include "task/thread.h"
#include "mem/paging.h"
#include "sync/mutex.h"
#include "sync/yieldlock.h"
#include "utils/bitmap.h"
#include "utils/linked_list.h"
#include "utils/hash_table.h"

#define USER_STACK_TOP   0xBFC00000  // 0xC0000000 - 4MB
#define USER_STACK_SIZE  65536       // 64KB
#define USER_PRCOESS_THREDS_MAX  4096

enum process_status {
  PROCESS_NORMAL,
  PROCESS_EXIT,
  PROCESS_EXIT_ZOMBIE
};

struct process_struct {
  uint32 id;
  char name[32];

  struct process_struct* parent;

  enum process_status status;

  // tid -> threads
  hash_table_t threads;

  // allocate user space thread for threads
  bitmap_t user_thread_stack_indexes;

  // is kernel process?
  uint8 is_kernel_process;

  // exit code
  int32 exit_code;

  // children processes
  hash_table_t children_processes;

  // exit children processes
  hash_table_t exit_children_processes;

  // child pid that is waiting
  uint32 waiting_child_pid;

  // waiting thread
  struct linked_list_node* waiting_thread_node;

  // page directory
  page_directory_t page_dir;
  yieldlock_t page_dir_lock;

  // lock to protect this struct
  yieldlock_t lock;
};
typedef struct process_struct pcb_t;


// ****************************************************************************
void init_process_manager();

struct process_struct* create_process(char* name, uint8 is_kernel_process);

struct task_struct* create_new_kernel_thread(pcb_t* process, char* name, void* function);
struct task_struct* create_new_user_thread(
    pcb_t* process, char* name, void* user_function, uint32 argc, char** argv);

void add_process_thread(pcb_t* process, struct task_struct* thread);
void remove_process_thread(pcb_t* process, struct task_struct* thread);

void add_child_process(pcb_t* parent, pcb_t* child);

void release_process_resources(pcb_t* process);
void destroy_process(pcb_t* process);

// syscalls implementation
int32 process_fork();
int32 process_exec(char* path, uint32 argc, char* argv[]);
int32 process_wait(uint32 pid, uint32* status);
void process_exit(int32 exit_code);

#endif
