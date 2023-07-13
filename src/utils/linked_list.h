#ifndef UTILS_LINKED_LIST_H
#define UTILS_LINKED_LIST_H

#include "common/common.h"

struct linked_list_node {
  type_t ptr;
  struct linked_list_node* prev;
  struct linked_list_node* next;
} __attribute__((packed));
typedef struct linked_list_node linked_list_node_t;

struct linked_list {
  linked_list_node_t* head;
  linked_list_node_t* tail;
  uint32 size;
  
} __attribute__((packed));
typedef struct linked_list linked_list_t;


// ****************************************************************************
linked_list_t create_linked_list();

void linked_list_init(linked_list_t* this);

void linked_list_append(linked_list_t* this, linked_list_node_t* new_node);

// insert after node.
void linked_list_insert(linked_list_t* this, linked_list_node_t* n, linked_list_node_t* new_node);

void linked_list_insert_to_head(linked_list_t* this, linked_list_node_t* new_node);

void linked_list_remove(linked_list_t* this, linked_list_node_t* n);

void linked_list_append_ele(linked_list_t* this, type_t ptr);
void linked_list_remove_ele(linked_list_t* this, type_t ptr);

// move all nodes from one linked list to another.
void linked_list_move(linked_list_t* dst, linked_list_t* src);
// concate a list after another.
void linked_list_concate(linked_list_t* dst, linked_list_t* src);


// ******************************** unit tests **********************************
void linked_list_test();

#endif
