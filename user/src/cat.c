#include "common/common.h"
#include "common/stdio.h"
#include "syscall/syscall.h"
#include "fs/file.h"

int main(uint32 argc, char* argv[]) {
  if (argc != 2) {
    printf("Usage: cat filename\n");
    return -1;
  }

  char* path = argv[1];

  file_stat_t file_stat;
  if (stat(path, &file_stat) != 0) {
    printf("Could not find file \"%s\"\n", path);
    return -1;
  }

  uint32 size = file_stat.size;
  char read_buffer[size + 1];
  if (read(path, read_buffer, 0, size) != size) {
    printf("Failed to read file \"%s\"\n", path);
    return -1;
  }
  read_buffer[size] = '\0';

  printf("%s", read_buffer);
  return 0;
}
