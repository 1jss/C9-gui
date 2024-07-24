#ifndef C9_RANDOM

#include "types.h"

u32 seed_state = 2;

f32 random_number() {
  seed_state = (u64)seed_state * 48271 % 0x7fffffff;
  return (f32)seed_state / 0x7fffffff;
}

void set_seed(u32 new_seed){
  seed_state = new_seed;
}

#define C9_RANDOM
#endif