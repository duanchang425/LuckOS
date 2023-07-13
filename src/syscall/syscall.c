#include "common/common.h"
#include "syscall/syscall.h"

extern void trigger_syscall_exit(int32 exit_code);
extern int32 trigger_syscall_fork();
extern int32 trigger_syscall_exec(char* path, uint32 argc, char* argv[]);
extern void trigger_syscall_yield();
extern int32 trigger_syscall_read(char* filename, char* buffer, uint32 offset, uint32 size);
extern int32 trigger_syscall_write(char* filename, char* buffer, uint32 offset, uint32 size);
extern int32 trigger_syscall_stat(char* filename, file_stat_t* stat);
extern int32 trigger_syscall_listdir(char* dir);
extern void trigger_syscall_print(char* str, void* args);
extern int32 trigger_syscall_wait(uint32 pid, uint32* status);
extern int32 trigger_syscall_thread_exit();
extern int32 trigger_syscall_read_char();
extern void trigger_syscall_move_cursor(int32 delta_x, int32 delta_y);


void exit(int32 exit_code) {
  return trigger_syscall_exit(exit_code);
}

int32 fork() {
  return trigger_syscall_fork();
}

int32 exec(char* path, uint32 argc, char* argv[]) {
  return trigger_syscall_exec(path, argc, argv);
}

void yield() {
  trigger_syscall_yield();
}

int32 read(char* filename, char* buffer, uint32 offset, uint32 size) {
  return trigger_syscall_read(filename, buffer, offset, size);
}

int32 write(char* filename, char* buffer, uint32 offset, uint32 size) {
  return trigger_syscall_write(filename, buffer, offset, size);
}

int32 stat(char* filename, file_stat_t* stat) {
  return trigger_syscall_stat(filename, stat);
}

int32 listdir(char* dir) {
  return trigger_syscall_listdir(dir);
}

void print(char* str, void* args) {
  return trigger_syscall_print(str, args);
}

int32 wait(uint32 pid, uint32* status) {
  return trigger_syscall_wait(pid, status);
}

void thread_exit() {
  trigger_syscall_thread_exit();
}

int32 read_char() {
  return trigger_syscall_read_char();
}

void move_cursor(int32 delta_x, int32 delta_y) {
  trigger_syscall_move_cursor(delta_x, delta_y);
}
