#include "monitor/monitor.h"
#include "common/common.h"
#include "mem/kheap.h"
#include "utils/id_pool.h"
#include "utils/math.h"
#include "utils/debug.h"
#include "mem/kheap.h"

static bool expand(id_pool_t* this);

void id_pool_init(id_pool_t* this, uint32 size, uint32 max_size) {
  this->size = size;
  this->expand_size = size;
  this->max_size = max_size;
  bitmap_init(&this->id_map, nullptr, this->size);
  yieldlock_init(&this->lock);
}

bool id_pool_set_id(id_pool_t* this, uint32 id) {
  bitmap_set_bit(&this->id_map, id);
}

bool id_pool_allocate_id(id_pool_t* this, uint32* id) {
  yieldlock_lock(&this->lock);
  if (!bitmap_allocate_first_free(&this->id_map, id)) {
    if (this->size == this->max_size) {
      yieldlock_unlock(&this->lock);
      return -1;
    } else {
      if (!expand(this)) {
        return false;
      }
      yieldlock_unlock(&this->lock);
      return bitmap_allocate_first_free(&this->id_map, id);
    }
  }
  yieldlock_unlock(&this->lock);
  return true;
}

static bool expand(id_pool_t* this) {
  uint32 new_size = min(this->size + this->expand_size, this->max_size);
  if (new_size == this->size) {
    return false;
  }

  if (!bitmap_expand(&this->id_map, new_size)) {
    return false;
  }
  this->size = new_size;
}

void id_pool_free_id(id_pool_t* this, uint32 id) {
  bitmap_clear_bit(&this->id_map, id);
}

void id_pool_destroy(id_pool_t* this) {
  bitmap_destroy(&this->id_map);
}


// ****************************************************************************
void id_pool_test() {
  monitor_print("id_pool test ...");

  id_pool_t pool;
  id_pool_init(&pool, 32, 128);
  id_pool_set_id(&pool, 0);

  uint32 id;
  for (int i = 1; i < 32; i++) {
    ASSERT(id_pool_allocate_id(&pool, &id) == true);
    ASSERT(id == i);
  }
  ASSERT(pool.size == 32);

  // test expand
  for (int i = 0; i < 32; i++) {
    ASSERT(id_pool_allocate_id(&pool, &id) == true);
    ASSERT(id == i + 32);
  }
  ASSERT(pool.size == 64);

  for (int i = 0; i < 32; i++) {
    ASSERT(id_pool_allocate_id(&pool, &id) == true);
    ASSERT(id == i + 64);
  }
  ASSERT(pool.size == 96);

  // free and allocate again
  for (int i = 10; i < 20; i++) {
    id_pool_free_id(&pool, i);
  }

  for (int i = 10; i < 20; i++) {
    ASSERT(id_pool_allocate_id(&pool, &id) == true);
    ASSERT(id == i);
  }

  id_pool_destroy(&pool);

  monitor_print_with_color("OK\n", COLOR_GREEN);
  kheap_validate_print(1);
  while(1) {}
}
