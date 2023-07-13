#include "utils/math.h"

uint32 max(uint32 x, uint32 y) {
  return x > y ? x : y;
}

uint32 min(uint32 x, uint32 y) {
  return x < y ? x : y;
}

int32 div(int32 x, int32 N) {
  if (x >= 0) {
    return x / N;
  } else {
    return (x - N + 1) / N;
  }
}

int32 mod(int32 x, int32 N) {
  return (x % N + N) %N;
}