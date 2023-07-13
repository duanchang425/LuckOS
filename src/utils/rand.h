#ifndef UTILS_RAND_H
#define UTILS_RAND_H

#include "common/common.h"

void rand_seed(uint32 seed);

void rand_seed_with_time();

uint32 rand();

uint32 rand_range(uint32 min, uint32 max);

#endif
