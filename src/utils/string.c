#include "common/stdlib.h"
#include "mem/kheap.h"
#include "utils/string.h"

char** copy_str_array(uint32 num, char* str_array[]) {
  if (num == 0 && str_array == nullptr) {
    return nullptr;
  }

  char** result = (char**)kmalloc(num * 4);
  for (uint32 i = 0; i < num; i++) {
    int32 length = 0;
    char c;
    while ((c = str_array[i][length]) != '\0') {
      length++;
    }
    char* str = (char*)kmalloc(length + 1);
    strcpy(str, str_array[i]);
    result[i] = str;
  }
  return result;
}

void destroy_str_array(uint32 num, char* str_array[]) {
  if (num == 0 && str_array == nullptr) {
    return;
  }

  for (uint32 i = 0; i < num; i++) {
    kfree(str_array[i]);
  }
  kfree(str_array);
}