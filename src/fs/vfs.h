#ifndef FS_FS_H
#define FS_FS_H

#include "common/common.h"
#include "fs/file.h"

enum fs_type {
  NAIVE,
  EXT4
};

struct disk_partition {
  uint32 offset;
};
typedef struct disk_partition disk_partition_t;

typedef int32 (*stat_file_func)(char* filename, file_stat_t* stat);
typedef int32 (*list_dir_func)(char* dir);
typedef int32 (*read_data_func)(char* filename, char* buffer, uint32 start, uint32 length);
typedef int32 (*write_data_func)(char* filename, char* buffer, uint32 start, uint32 length);

struct file_system {
  enum fs_type type;
  disk_partition_t partition;

  // functions
  stat_file_func stat_file;
  list_dir_func list_dir;
  read_data_func read_data;
  write_data_func write_data;
};
typedef struct file_system fs_t;


// ****************************************************************************
void init_file_system();

int32 stat_file(char* filename, file_stat_t* stat);
int32 list_dir(char* dir);
int32 read_file(char* filename, char* buffer, uint32 start, uint32 length);
int32 write_file(char* filename, char* buffer, uint32 start, uint32 length);


#endif
