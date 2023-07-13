#include "syscall/syscall.h"
#include "sys/common.h"

extern int main(uint32, char**);

void _start(uint32 argc, char** argv) {
  int exit_code = main(argc, argv);
  exit(exit_code);
}
