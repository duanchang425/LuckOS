#include "mem/kheap.h"
#include "monitor/monitor.h"
#include "utils/hash_table.h"
#include "utils/debug.h"

#define INITIAL_BUCKETS_NUM 16
#define LOAD_FACTOR 0.75

static void hash_table_expand(hash_table_t* this);

hash_table_t create_hash_table() {
  hash_table_t map;
  hash_table_init(&map);
  return map;
}

void hash_table_init(hash_table_t* this) {
  this->size = 0;
  this->buckets_num = INITIAL_BUCKETS_NUM;
  this->buckets = (linked_list_t*)kmalloc(this->buckets_num * sizeof(linked_list_t));
  for (uint32 i = 0; i < this->buckets_num; i++) {
    linked_list_init(&this->buckets[i]);
  }
}

void hash_table_clear(hash_table_t* this) {
  for (int32 i = 0; i < this->buckets_num; i++) {
    linked_list_t* bucket = &this->buckets[i];
    linked_list_node_t* kv_node = bucket->head;
    while (kv_node != nullptr) {
      hash_table_kv_t* kv = (hash_table_kv_t*)kv_node->ptr;
      kfree(kv);
      linked_list_node_t* crt_node = kv_node;
      kv_node = crt_node->next;
      linked_list_remove(bucket, crt_node);
      kfree(crt_node);
    }
  }
  this->size = 0;
}

void hash_table_destroy(hash_table_t* this) {
  for (int32 i = 0; i < this->buckets_num; i++) {
    linked_list_t* bucket = &this->buckets[i];
    linked_list_node_t* kv_node = bucket->head;
    while (kv_node != nullptr) {
      hash_table_kv_t* kv = (hash_table_kv_t*)kv_node->ptr;
      kfree(kv->v_ptr);
      kfree(kv);
      linked_list_node_t* crt_node = kv_node;
      kv_node = crt_node->next;
      linked_list_remove(bucket, crt_node);
      kfree(crt_node);
    }
  }
  kfree(this->buckets);
  this->size = 0;
}

static linked_list_node_t* hash_table_bucket_lookup(
    hash_table_t* this, linked_list_t* bucket, uint32 key) {
  linked_list_node_t* kv_node = bucket->head;
  while (kv_node != nullptr) {
    hash_table_kv_t* kv = (hash_table_kv_t*)kv_node->ptr;
    if (kv->key == key) {
      return kv_node;
    }
    kv_node = kv_node->next;
  }
  return nullptr;
}

void* hash_table_get(hash_table_t* this, uint32 key) {
  linked_list_t* bucket = &this->buckets[key % this->buckets_num];
  linked_list_node_t* kv_node = hash_table_bucket_lookup(this, bucket, key);
  if (kv_node != nullptr) {
    hash_table_kv_t* kv = (hash_table_kv_t*)kv_node->ptr;
    return kv->v_ptr;
  }
  return nullptr;
}

bool hash_table_contains(hash_table_t* this, uint32 key) {
  linked_list_t* bucket = &this->buckets[key % this->buckets_num];
  return hash_table_bucket_lookup(this, bucket, key) != nullptr;
}

void* hash_table_put(hash_table_t* this, uint32 key, void* v_ptr) {
  linked_list_t* bucket = &this->buckets[key % this->buckets_num];
  linked_list_node_t* kv_node = hash_table_bucket_lookup(this, bucket, key);
  if (kv_node != nullptr) {
    hash_table_kv_t* kv = (hash_table_kv_t*)kv_node->ptr;
    void* old_value_ptr = kv->v_ptr;
    kv->v_ptr = v_ptr;
    return old_value_ptr;
  }

  // Insert new kv node.
  hash_table_kv_t* new_kv = (hash_table_kv_t*)kmalloc(sizeof(hash_table_kv_t));
  new_kv->key = key;
  new_kv->v_ptr = v_ptr;
  linked_list_append_ele(bucket, new_kv);
  this->size++;

  // If map size exceeds load factor, expand the map.
  if (this->size > this->buckets_num * LOAD_FACTOR) {
    hash_table_expand(this);
  }

  return nullptr;
}

static void hash_table_expand(hash_table_t* this) {
  uint32 new_buckets_num = this->buckets_num * 2;
  linked_list_t* new_buckets = (linked_list_t*)kmalloc(new_buckets_num * sizeof(linked_list_t));
  for (int32 i = 0; i < new_buckets_num; i++) {
    linked_list_init(&new_buckets[i]);
  }

  // Place kv nodes into new buckets
  for (int32 i = 0; i < this->buckets_num; i++) {
    linked_list_t* bucket = &this->buckets[i];
    linked_list_node_t* kv_node = bucket->head;
    while (kv_node != nullptr) {
      hash_table_kv_t* kv = (hash_table_kv_t*)kv_node->ptr;
      linked_list_node_t* crt_node = kv_node;
      kv_node = crt_node->next;
      linked_list_remove(bucket, crt_node);

      uint32 key = kv->key;
      linked_list_append(&new_buckets[key % new_buckets_num], crt_node);
    }
  }

  kfree(this->buckets);

  this->buckets = new_buckets;
  this->buckets_num = new_buckets_num;
}

void* hash_table_remove(hash_table_t* this, uint32 key) {
  linked_list_t* bucket = &this->buckets[key % this->buckets_num];
  linked_list_node_t* kv_node = hash_table_bucket_lookup(this, bucket, key);
  if (kv_node != nullptr) {
    linked_list_remove(bucket, kv_node);
    hash_table_kv_t* kv = (hash_table_kv_t*)kv_node->ptr;
    void* value = kv->v_ptr;
    kfree(kv);
    kfree(kv_node);
    this->size--;

    // TODO: shrink buckets if needed?
    return value;
  }
  return nullptr;
}

void hash_table_print(hash_table_t* this) {
  monitor_printf("**************** hash table *****************\n");
  monitor_printf("// size = %d\n", this->size);
  for (uint32 i = 0; i < this->buckets_num; i++) {
    linked_list_t* bucket = &this->buckets[i];
    linked_list_node_t* kv_node = bucket->head;
    while (kv_node != nullptr) {
      hash_table_kv_t* kv = (hash_table_kv_t*)kv_node->ptr;
      monitor_printf("key = %d, v_ptr = %x\n", kv->key, kv->v_ptr);
      kv_node = kv_node->next;
    }
  }
  monitor_printf("*********************************************\n");
}

hash_table_interator_t hash_table_create_iterator(hash_table_t* map) {
  hash_table_interator_t iter;
  iter.map = map;
  iter.bucket_index = -1;
  iter.node = nullptr;
  iter.index = -1;
  return iter;
}

bool hash_table_iterator_has_next(hash_table_interator_t* iter) {
  return iter->index < (int32)iter->map->size - 1;
}

hash_table_kv_t* hash_table_iterator_next(hash_table_interator_t* iter) {
  if (iter->index >= iter->map->size - 1) {
    iter->index = iter->map->size;
    iter->bucket_index = iter->map->buckets_num;
    iter->node = nullptr;
    return nullptr;
  }

  if (iter->node != nullptr && iter->node->next != nullptr) {
    iter->node = iter->node->next;
    iter->index++;
    return (hash_table_kv_t*)iter->node->ptr;
  }

  iter->bucket_index++;
  while (iter->bucket_index < iter->map->buckets_num) {
    linked_list_t* bucket = &iter->map->buckets[iter->bucket_index];
    linked_list_node_t* head = bucket->head;
    if (head != nullptr) {
      iter->node = head;
      iter->index++;
      return (hash_table_kv_t*)iter->node->ptr;
    }
    iter->bucket_index++;
  }

  iter->node = nullptr;
  iter->index++;
}


// ****************************** unit test ***********************************
void hash_table_test() {
  hash_table_t map = create_hash_table();

  // Insert (1, 10)
  uint32* v1 = (uint32*)kmalloc(sizeof(uint32));
  *v1 = 10;
  hash_table_put(&map, 1, v1);
  ASSERT(map.size == 1);
  ASSERT(hash_table_contains(&map, 1));
  ASSERT(*((uint32*)hash_table_get(&map, 1)) == *v1);

  // Insert (2, 20)
  uint32* v2 = (uint32*)kmalloc(sizeof(uint32));
  *v2 = 20;
  hash_table_put(&map, 2, v2);
  ASSERT(map.size == 2);
  ASSERT(hash_table_contains(&map, 2));
  ASSERT(*((uint32*)hash_table_get(&map, 2)) == *v2);

  // Insert (3, 29)
  uint32* v3_x = (uint32*)kmalloc(sizeof(uint32));
  *v3_x = 30 - 1;
  hash_table_put(&map, 3, v3_x);
  ASSERT(map.size == 3);
  ASSERT(hash_table_contains(&map, 3));
  ASSERT(*((uint32*)hash_table_get(&map, 3)) == *v3_x);

  // Insert (3, 30)
  uint32* v3 = (uint32*)kmalloc(sizeof(uint32));
  *v3 = 30;
  v3_x = (uint32*)hash_table_put(&map, 3, v3);
  ASSERT(map.size == 3);
  ASSERT(hash_table_contains(&map, 3));
  ASSERT(*((uint32*)hash_table_get(&map, 3)) == *v3);
  ASSERT(*v3_x == 29);
  kfree(v3_x);

  // Test iterator
  hash_table_interator_t iter = hash_table_create_iterator(&map);
  ASSERT(iter.index == -1);
  int32 iter_total_num = 0;
  int32 key = 1;
  while (hash_table_iterator_has_next(&iter)) {
    hash_table_kv_t* kv_node = hash_table_iterator_next(&iter);
    ASSERT(kv_node->key == key);
    ASSERT(*(uint32*)kv_node->v_ptr == key * 10);
    iter_total_num++;
    key++;
  }
  monitor_printf("num = %d\n", iter_total_num);
  ASSERT(iter_total_num == 3);
  ASSERT(iter.index == 2);

  hash_table_kv_t* kv_node = hash_table_iterator_next(&iter);
  ASSERT(kv_node == nullptr);
  ASSERT(iter.node == nullptr);
  ASSERT(iter.index == 3);

  // Remove (2, 20)
  v2 = (uint32*)hash_table_remove(&map, 2);
  ASSERT(map.size == 2);
  ASSERT(false == hash_table_contains(&map, 2));
  ASSERT(hash_table_get(&map, 2) == nullptr);
  ASSERT(*v2 == 20);
  kfree(v2);

  // Test expand:
  //  - insert 12 elements without triggering expansion;
  //  - insert 13th element and expand;
  for (uint32 i = 0; i < 10; i++) {
    uint32 key = i + 4;
    uint32* v = (uint32*)kmalloc(sizeof(uint32));
    *v = key * 10;
    hash_table_put(&map, key, v);
    ASSERT(map.size == 3 + i);
    ASSERT(hash_table_contains(&map, key));
    ASSERT(*((uint32*)hash_table_get(&map, key)) == *v);    
  }
  ASSERT(map.buckets_num == INITIAL_BUCKETS_NUM);

  uint32* vv = (uint32*)kmalloc(sizeof(uint32));
  *vv = 1000;
  hash_table_put(&map, 100, vv);
  ASSERT(map.size == 13);
  ASSERT(hash_table_contains(&map, 100));
  ASSERT(*((uint32*)hash_table_get(&map, 100)) == *vv);
  ASSERT(map.buckets_num == INITIAL_BUCKETS_NUM * 2);

  ASSERT(hash_table_contains(&map, 1));
  ASSERT(*((uint32*)hash_table_get(&map, 1)) == 10);
  ASSERT(hash_table_contains(&map, 3));
  ASSERT(*((uint32*)hash_table_get(&map, 3)) == 30);
  for (uint32 i = 0; i < 10; i++) {
    uint32 key = i + 4;
    ASSERT(*((uint32*)hash_table_get(&map, key)) == (key * 10));
  }
  ASSERT(*((uint32*)hash_table_get(&map, 100)) == 1000);

  // End
  hash_table_destroy(&map);
  kheap_validate_print(1);

  monitor_printf("hash table test ");
  monitor_print_with_color("OK\n", COLOR_GREEN);
}
