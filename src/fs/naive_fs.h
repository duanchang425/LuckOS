#ifndef FS_NAIVE_FS_H
#define FS_NAIVE_FS_H

#include "common/common.h"
#include "fs/vfs.h"

fs_t* get_naive_fs();

struct naive_file_meta {
  char filename[64];
  uint32 size;
  uint32 offset;
};
typedef struct naive_file_meta naive_file_meta_t;


// ****************************************************************************
void init_naive_fs();


#endif
