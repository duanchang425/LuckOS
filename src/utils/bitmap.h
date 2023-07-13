#ifndef UTILS_BITMAP_H
#define UTILS_BITMAP_H

#include "common/common.h"
#include "common/global.h"

// argument struct for common isr_handler
typedef struct bit_map {
  uint32* array;
  uint8 alloc_array;
  int array_size;  // size of the array
  int total_bits;
} bitmap_t;

bitmap_t bitmap_create(uint32* array, int total_bits);

void bitmap_init(bitmap_t* this, uint32* array, int total_bits);

void bitmap_set_bit(bitmap_t* this, uint32 bit);

void bitmap_clear_bit(bitmap_t* this, uint32 bit);

bool bitmap_test_bit(bitmap_t* this, uint32 bit);

void bitmap_clear(bitmap_t* this);

bool bitmap_expand(bitmap_t* this, uint32 expand_size);

void bitmap_destroy(bitmap_t* this);

// Returns sucess or not.
// The result bit is stored in argument *bit.
bool bitmap_find_first_free(bitmap_t* this, uint32* bit);
bool bitmap_allocate_first_free(bitmap_t* this, uint32* bit);

#endif
