#ifndef COMMON_IO_H
#define COMMON_IO_H

#include "common/common.h"

void outb(uint16 port, uint8 value);
uint8 inb(uint16 port);
uint16 inw(uint16 port);

#endif
