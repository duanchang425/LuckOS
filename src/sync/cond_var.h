#ifndef SYNC_COND_VAR_H
#define SYNC_COND_VAR_H

#include "common/common.h"
#include "sync/yieldlock.h"
#include "utils/linked_list.h"

// condition variable.
struct cond_var {
  linked_list_t waiting_task_queue;
};
typedef struct cond_var cond_var_t;

typedef bool (*cv_predicator_func)();


// ****************************************************************************
void cond_var_init(cond_var_t* cv);
void cond_var_wait(cond_var_t* cv, yieldlock_t* lock, cv_predicator_func predicator);
void cond_var_notify(cond_var_t* cv);


#endif
