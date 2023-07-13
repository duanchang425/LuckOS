#include "common/common.h"
#include "common/stdio.h"
#include "syscall/syscall.h"

int main(uint32 argc, char* argv[]) {
  printf("start user app: hello\n");
  printf("argc = %d\n", argc);
  for (uint32 i = 0; i < argc; i++) {
    printf("argv[%d] = %s\n", i, argv[i]);
  }

  return 0;
}
