#include "utils/rand.h"
#include "interrupt/timer.h"

static uint32 seed = 0;
static uint32 rand_a = 1103515245;
static uint32 rand_c = 12345;
static uint32 rand_m = 0x80000000;  // 2 ^ 31

void rand_seed(uint32 new_seed) {
  seed = new_seed;
}

void rand_seed_with_time() {
  rand_seed(getTick());
}

uint32 rand() {
  if (seed == 0) {
    rand_seed_with_time();
  }

  seed = (seed * rand_a + rand_c) % rand_m;
  return seed;
}

uint32 rand_range(uint32 min, uint32 max) {
  return (uint32)((double)rand() / rand_m * (max - min) + min);
}
