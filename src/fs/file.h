#ifndef FS_FILE_H
#define FS_FILE_H

#include "common/common.h"

struct file_stat {
  uint32 size;
  uint8 acl;
};
typedef struct file_stat file_stat_t;

#endif
