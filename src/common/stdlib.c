#include "common/stdlib.h"

extern void* get_ebp();

void memset(void* addr, uint8 value, int num) {
  for (int i = 0; i < num; i++) {
    *((uint8*)addr + i) = value;
  }
}

void memcpy(void* dst, void* src, int num) {
  for (int i = 0; i < num; i++) {
    *((uint8*)(dst + i)) = *((uint8*)(src + i));
  }
}

int32 strcpy(char* dst, char* src) {
  int32 num = 0;
  while (1) {
    char c = *src;
    *dst = c;
    if (c == '\0') {
      break;
    }
    dst++;
    src++;
    num++;
  }
  return num;
}

int32 strcmp(char* str1, char* str2) {
  int i = 0;
  char c;
  while ((c = str1[i]) != '\0') {
    char c2 = str2[i];
    if (c2 == '\0') {
      return 1;
    }
    if (c != c2) {
      return c < c2 ? -1 : 1;
    }
    i++;
  }
  return str2[i] == '\0' ? 0 : 1;
}

int32 strlen(char* str) {
  int i = 0;
  char c;
  while ((c = str[i]) != '\0') {
    i++;
  }
  return i;
}

int32 int2str(char* dst, int32 num) {
  uint32 start = 0;
  if (num < 0 ) {
    dst[start++] = '-';
    num = -num;
  }

  char buf[16];
  buf[15] = '\0';
  uint32 i = 14;
  while (num > 0) {
    uint32 remain = num % 10;
    buf[i--] = '0' + remain;
    num = num / 10;
  }

  strcpy(dst + start, buf + i + 1);
  return start + 14 - i;
}

int32 int2hex(char* dst, int32 num) {
  uint32 start = 0;
  if (num < 0 ) {
    dst[start++] = '-';
    num = -num;
  }
  dst[start++] = '0';
  dst[start++] = 'x';

  char buf[16];
  buf[15] = '\0';
  uint32 i = 14;
  while (num > 0) {
    uint32 remain = num % 16;
    if (remain <= 9) {
      buf[i--] = '0' + remain;
    } else {
      buf[i--] = 'a' + (remain - 10);
    }
    num = num / 16;
  }

  strcpy(dst + start, buf + i + 1);
  return start + 14 - i;
}

void sprintf(char* dst, char* str, ...) {
  void* ebp = get_ebp();
  void* arg_ptr = ebp + 16;

  int i = 0;
  int write_index = 0;
  while (1) {
    char c = str[i];
    if (c == '\0') {
      break;
    }

    if (c == '%') {
      i++;
      char next = str[i];
      if (c == '\0') {
        break;
      }

      if (next == 'd') {
        int32 int_arg = *((int32*)arg_ptr);
        char buf[16];
        int2str(buf, int_arg);
        write_index += strcpy(dst + write_index, buf);
        arg_ptr += 4;
      } else if (next == 'u') {
        uint32 int_arg = *((uint32*)arg_ptr);
        char buf[16];
        int2str(buf, (int32)int_arg);
        write_index += strcpy(dst + write_index, buf);
        arg_ptr += 4;
      } else if (next == 'x') {
        int32 int_arg = *((int32*)arg_ptr);
        char buf[16];
        int2hex(buf, int_arg);
        write_index += strcpy(dst + write_index, buf);
        arg_ptr += 4;
      } else if (next == 's') {
        char* str_arg = *((char**)arg_ptr);
        write_index += strcpy(dst + write_index, str_arg);
        arg_ptr += 4;
      }
    } else {
      dst[write_index++] = c;
    }

    i++;
  }

  dst[write_index] = '\0';
}
