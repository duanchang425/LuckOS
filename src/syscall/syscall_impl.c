#include "common/stdlib.h"
#include "monitor/monitor.h"
#include "interrupt/interrupt.h"
#include "mem/paging.h"
#include "mem/kheap.h"
#include "task/thread.h"
#include "fs/vfs.h"
#include "fs/file.h"
#include "driver/keyboard.h"
#include "task/process.h"
#include "task/scheduler.h"
#include "syscall/syscall_impl.h"
#include "utils/debug.h"

static int32 syscall_exit_impl(int32 exit_code) {
  process_exit(exit_code);
}

static int32 syscall_fork_impl() {
  return process_fork();
}

static int32 syscall_exec_impl(char* path, uint32 argc, char* argv[]) {
  return process_exec(path, argc, argv);
}

static int32 syscall_yield_impl() {
  return 0;
}

static int32 syscall_read_impl(char* filename, char* buffer, uint32 offset, uint32 size) {
  return read_file(filename, buffer, offset, size);
}

static int32 syscall_write_impl(char* filename, char* buffer, uint32 offset, uint32 size) {
  return -1;
}

static int32 syscall_stat_impl(char* filename, file_stat_t* stat) {
  return stat_file(filename, stat);
}

static int32 syscall_listdir_impl(char* dir) {
  return list_dir(dir);
}

static int32 syscall_print_impl(char* str, void* args_ptr) {
  monitor_printf_args(str, args_ptr);
  return 0;
}

static int32 syscall_wait_impl(uint32 pid, uint32* status) {
  return process_wait(pid, status);
}

static int32 syscall_thread_exit_impl() {
  schedule_thread_exit();
  return 0;
}

static int32 syscall_read_char_impl() {
  return read_keyboard_char();
}

static int32 syscall_move_cursor_impl(int32 delta_x, int32 delta_y) {
  monitor_move_cursor(delta_x, delta_y);
  return 0;
}

int32 syscall_handler(isr_params_t isr_params) {
  // syscall num saved in eax.
  // args list: ecx, edx, ebx, esi, edi
  uint32 syscall_num = isr_params.eax;

  switch (syscall_num) {
    case SYSCALL_EXIT_NUM:
      return syscall_exit_impl((int32)isr_params.ecx);
    case SYSCALL_FORK_NUM:
      return syscall_fork_impl();
    case SYSCALL_EXEC_NUM:
      return syscall_exec_impl((char*)isr_params.ecx, isr_params.edx, (char**)isr_params.ebx);
    case SYSCALL_YIELD_NUM:
      return syscall_yield_impl();
    case SYSCALL_READ_NUM:
      return syscall_read_impl((char*)isr_params.ecx, (char*)isr_params.edx,
          (uint32)isr_params.ebx, (uint32)isr_params.esi);
    case SYSCALL_WRITE_NUM:
      return syscall_write_impl((char*)isr_params.ecx, (char*)isr_params.edx,
          (uint32)isr_params.ebx, (uint32)isr_params.esi);
    case SYSCALL_STAT_NUM:
      return syscall_stat_impl((char*)isr_params.ecx, (file_stat_t*)isr_params.edx);
    case SYSCALL_LISTDIR_NUM:
      return syscall_listdir_impl((char*)isr_params.ecx);
    case SYSCALL_PRINT_NUM:
      return syscall_print_impl((char*)isr_params.ecx, (void*)isr_params.edx);
    case SYSCALL_WAIT_NUM:
      return syscall_wait_impl((uint32)isr_params.ecx, (uint32*)isr_params.edx);
    case SYSCALL_THREAD_EXIT_NUM:
      return syscall_thread_exit_impl();
    case SYSCALL_READ_CHAR_NUM:
      return syscall_read_char_impl();
    case SYSCALL_MOVE_CURSOR_NUM:
      return syscall_move_cursor_impl((int32)isr_params.ecx, (int32)isr_params.edx);
    default:
      PANIC();
  }
}
