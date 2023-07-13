#include "driver/hard_disk.h"
#include "fs/naive_fs.h"
#include "monitor/monitor.h"
#include "mem/kheap.h"
#include "common/stdlib.h"
#include "utils/math.h"

fs_t naive_fs;
uint32 file_num;
naive_file_meta_t* file_metas;

fs_t* get_naive_fs() {
  return &naive_fs;
}

static int32 naive_fs_stat_file(char* filename, file_stat_t* stat) {
  for (int i = 0; i < file_num; i++) {
    naive_file_meta_t* meta = file_metas + i;
    if (strcmp(meta->filename, filename) == 0) {
      stat->size = meta->size;
      return 0;
    }
  }
  return -1;
}

static int32 naive_fs_list_dir(char* dir) {
  uint32 size_length[file_num];
  uint32 max_length = 0;
  for (uint32 i = 0; i < file_num; i++) {
    naive_file_meta_t* meta = file_metas + i;
    uint32 size = meta->size;
    uint32 length = 1;
    while ((size /= 10) > 0) {
      length++;
    }
    size_length[i] = length;
    max_length = max(max_length, length);
  }

  for (uint32 i = 0; i < file_num; i++) {
    naive_file_meta_t* meta = file_metas + i;
    monitor_printf("root  ");
    for (uint32 j = 0; j < max_length - size_length[i]; j++) {
      monitor_printf(" ");
    }
    monitor_printf("%d  %s\n", meta->size, meta->filename);
  }
  return -1;
}

static int32 naive_fs_read_data(char* filename, char* buffer, uint32 start, uint32 length) {
  naive_file_meta_t* file_meta = nullptr;
  for (int i = 0; i < file_num; i++) {
    naive_file_meta_t* meta = file_metas + i;
    if (strcmp(meta->filename, filename) == 0) {
      file_meta = meta;
      break;
    }
  }
  if (file_meta == nullptr) {
    return -1;
  }

  uint32 offset = file_meta->offset;
  uint32 size = file_meta->size;
  if (length > size) {
    length = size;
  }

  read_hard_disk((char*)buffer, naive_fs.partition.offset + offset + start, length);
  return length;  
}

static int32 naive_fs_write_data(char* filename, char* buffer, uint32 start, uint32 length) {
}

void init_naive_fs() {
  naive_fs.type = NAIVE;
  naive_fs.partition.offset = 2057 * SECTOR_SIZE;

  naive_fs.stat_file = naive_fs_stat_file;
  naive_fs.read_data = naive_fs_read_data;
  naive_fs.write_data = naive_fs_write_data;
  naive_fs.list_dir = naive_fs_list_dir;

  read_hard_disk((char*)&file_num, 0 + naive_fs.partition.offset, sizeof(uint32));
  //monitor_printf("naive fs found %d files:\n", file_num);

  uint32 meta_size = file_num * sizeof(naive_file_meta_t);
  file_metas = (naive_file_meta_t*)kmalloc(meta_size);
  read_hard_disk((char*)file_metas, 4 + naive_fs.partition.offset, meta_size);
  for (int i = 0; i < file_num; i++) {
    naive_file_meta_t* meta = file_metas + i;
    //monitor_printf(" - %s, offset = %d, size = %d\n", meta->filename, meta->offset, meta->size);
  }
}
