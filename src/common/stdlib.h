#ifndef COMMON_STDLIB_H
#define COMMON_STDLIB_H

#include "common/common.h"

void memset(void* addr, uint8 value, int num);
void memcpy(void* dst, void* src, int num);

int32 strcpy(char* dst, char* src);
int32 strcmp(char* str1, char* str2);
int32 strlen(char* str);

int32 int2str(char* dst, int32 num);
int32 int2hex(char* dst, int32 num);

void sprintf(char* dst, char* str, ...);

#endif
