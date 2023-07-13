#ifndef DRIVER_HARD_DISK_H
#define DRIVER_HARD_DISK_H

#include "common/common.h"

#define SECTOR_SIZE  512

void init_hard_disk();

void read_hard_disk(char* buffer, uint32 start, uint32 length);


#endif
