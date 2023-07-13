#ifndef UTILS_ID_POOL_H
#define UTILS_ID_POOL_H

#include "common/common.h"
#include "common/global.h"
#include "sync/yieldlock.h"
#include "utils/bitmap.h"

// This is a thread-safe pool to allocate uint32 id.
struct id_pool {
  int32 size;
  int32 max_size;
  int32 expand_size;
  bitmap_t id_map;
  yieldlock_t lock;
};
typedef struct id_pool id_pool_t;

void id_pool_init(id_pool_t* this, uint32 size, uint32 max_size);
bool id_pool_set_id(id_pool_t* this, uint32 id);
bool id_pool_allocate_id(id_pool_t* this, uint32* id);
void id_pool_free_id(id_pool_t* this, uint32 id);
void id_pool_destroy(id_pool_t* this);


// ****************************************************************************
void id_pool_test();

#endif
