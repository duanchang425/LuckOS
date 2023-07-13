#include "common/stdio.h"
#include "syscall/syscall.h"

extern void* get_ebp();

void printf(char* str, ...) {
  void* ebp = get_ebp();
  void* arg_ptr = ebp + 12;
  print(str, arg_ptr);
}
