#ifndef UTILS_STRING_H
#define UTILS_STRING_H

#include "common/common.h"

// Copy a char* array. Result is allocated on kheap and must be freed by calling destroy_str_array.
char** copy_str_array(uint32 num, char* str_array[]);

// Release a char* array created by copy_str_array.
void destroy_str_array(uint32 num, char* str_array[]);

#endif
