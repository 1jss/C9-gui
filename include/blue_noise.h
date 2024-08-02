#ifndef C9_BLUE_NOISE

#include "types.h" // f32, i32
#include "blue_noise_texture_u8.h"

// Function to get a blue noise value from the texture
f32 get_blue_noise_value(i32 x, i32 y) {
  // Ensure coordinates are within bounds
  x = x % 32;
  y = y % 32;
  return 1.0 * blue_noise_texture_u8[x][y] / 255;
}

#define C9_BLUE_NOISE
#endif