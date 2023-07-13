#include "fs/vfs.h"
#include "fs/naive_fs.h"

// ***************************** root fs APIs *********************************
static fs_t* get_fs(char* path) {
  // This is the only fs we have mount :)
  return get_naive_fs();
}

void init_file_system() {
  init_naive_fs();
}

int32 stat_file(char* filename, file_stat_t* stat) {
  fs_t* fs = get_fs(filename);
  return fs->stat_file(filename, stat);
}

int32 list_dir(char* dir) {
  fs_t* fs = get_fs(dir);
  return fs->list_dir(dir);
}

int32 read_file(char* filename, char* buffer, uint32 start, uint32 length) {
  fs_t* fs = get_fs(filename);
  return fs->read_data(filename, buffer, start, length);
}

int32 write_file(char* filename, char* buffer, uint32 start, uint32 length) {
  fs_t* fs = get_fs(filename);
  return fs->write_data(filename, buffer, start, length);
}
