#include "common/common.h"
#include "common/stdio.h"
#include "syscall/syscall.h"
#include "fs/file.h"

int main(uint32 argc, char* argv[]) {
  for (int i = 1; i < argc; i++) {
    printf(argv[i]);
    printf(" ");
  }
  printf("\n");
  return 0;
}
