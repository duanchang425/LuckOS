#include "monitor/monitor.h"
#include "common/common.h"
#include "utils/bitmap.h"
#include "mem/kheap.h"

#define INDEX_FROM_BIT(a) (a/32)
#define OFFSET_FROM_BIT(a) (a%32)

bitmap_t bitmap_create(uint32* array, int total_bits) {
  bitmap_t ret;
  bitmap_init(&ret, array, total_bits);
  return ret;
}

void bitmap_init(bitmap_t* this, uint32* array, int total_bits) {
  this->total_bits = total_bits;
  this->array_size = total_bits / 32;
  if (array == nullptr) {
    array = (uint32*)kmalloc(this->array_size * 4);
    //monitor_printf("alloc bitmap array %x\n", array);
    this->alloc_array = true;
  } else {
    this->alloc_array = false;
  }

  for (int32 i = 0; i < this->array_size; i++) {
    array[i] = 0;
  }
  this->array = array;
}

void bitmap_set_bit(bitmap_t* this, uint32 bit) {
  uint32 idx = INDEX_FROM_BIT(bit);
  uint32 off = OFFSET_FROM_BIT(bit);
  this->array[idx] |= (0x1 << off);
}

void bitmap_clear_bit(bitmap_t* this, uint32 bit) {
  uint32 idx = INDEX_FROM_BIT(bit);
  uint32 off = OFFSET_FROM_BIT(bit);
  this->array[idx] &= ~(0x1 << off);
}

bool bitmap_test_bit(bitmap_t* this, uint32 bit) {
  uint32 idx = INDEX_FROM_BIT(bit);
  uint32 off = OFFSET_FROM_BIT(bit);
  return (this->array[idx] & (0x1 << off)) != 0;
}

bool bitmap_find_first_free(bitmap_t* this, uint32* bit) {
  uint32 i, j;
  for (i = 0; i < this->array_size; i++) {
    uint32 ele = this->array[i];
    if (ele != 0xFFFFFFFF) {
      for (j = 0; j < 32; j++) {
        if (!(ele & (0x1 << j))) {
          *bit = i * 32 + j;
          return true;
        }
      }
    }
  }

  return false;
}

bool bitmap_allocate_first_free(bitmap_t* this, uint32* bit) {
  bool success = bitmap_find_first_free(this, bit);
  if (!success) {
    return false;
  }

  bitmap_set_bit(this, *bit);
  return true;
}

void bitmap_clear(bitmap_t* this) {
  for (uint32 i = 0; i < this->array_size; i++) {
    this->array[i] = 0;
  }
}

bool bitmap_expand(bitmap_t* this, uint32 expand_size) {
  uint32 new_size = expand_size;
  uint32 new_array_size = new_size / 32;
  uint32* array = (uint32*)kmalloc(new_array_size * 4);
  for (int32 i = 0; i < new_array_size; i++) {
    array[i] = 0;
  }
  for (int32 i = 0; i < this->array_size; i++) {
    array[i] = this->array[i];
  }

  if (this->alloc_array) {
    kfree(this->array);
  }
  this->total_bits = new_size;
  this->array_size = new_array_size;
  this->alloc_array = true;
  this->array = array;
  return true;
}

void bitmap_destroy(bitmap_t* this) {
  if (this->alloc_array) {
    //monitor_printf("destroyed user_thread_stack_indexes %x\n", this->array);
    kfree(this->array);
  }
}
