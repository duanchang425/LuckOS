#ifndef MEM_KHEAP_H
#define MEM_KHEAP_H

#include "common/common.h"
#include "utils/ordered_array.h"

#define KHEAP_START          0xC0C00000
#define KHEAP_MIN_SIZE       0x300000
#define KHEAP_MAX            0xE0000000

#define KHEAP_INDEX_NUM      0x20000
#define KHEAP_MAGIC          0x123060AB

// 9 bytes
struct kheap_block_header {
  uint32 magic;
  uint8 is_hole;
  uint32 size;
} __attribute__((packed));
typedef struct kheap_block_header kheap_block_header_t;

// 8 bytes
struct kheap_block_footer {
  uint32 magic;
  kheap_block_header_t *header;
} __attribute__((packed));
typedef struct kheap_block_footer kheap_block_footer_t;

typedef struct kernel_heap {
  ordered_array_t index;
  uint32 start_address;
  uint32 end_address;
  uint32 size;
  uint32 max_address;
  uint8 supervisor;
  uint8 readonly;
} kheap_t;


// ****************************************************************************
void init_kheap();

kheap_t create_kheap(uint32 start, uint32 end, uint32 max, uint8 supervisor, uint8 readonly);

void* kmalloc(uint32 size);

void* kmalloc_aligned(uint32 size);

void kfree(void *p);

uint32 kheap_validate_print(uint8 print);


// ******************************** unit tests **********************************
void kheap_test();

void kheap_killer();

#endif
