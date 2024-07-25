#ifndef C9_BLUE_NOISE

#include "types.h" // f32, u8
#include "blue_noise_texture_f32.h"

// Function to get a blue noise value from the texture
f32 get_blue_noise_value(i32 x, i32 y) {
 // Ensure coordinates are within bounds
  x = x & 31;
  y = y & 31;
  return blue_noise_texture_f32[y][x];
}

#define C9_BLUE_NOISE
#endif