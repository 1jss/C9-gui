#ifndef C9_COLOR

#include "random.h" // random_number
#include "types.h" // u8, u32, f32
#include "blue_noise.h" // get_blue_noise_value

typedef u32 RGBA; // 0xRRGGBBAA

u8 red(RGBA color) {
  return (color & 0xFF000000) >> 24;
}
u8 green(RGBA color) {
  return (color & 0x00FF0000) >> 16;
}
u8 blue(RGBA color) {
  return (color & 0x0000FF00) >> 8;
}
u8 alpha(RGBA color) {
  return color & 0x000000FF;
}

// Gradient is a struct that represents a color gradient between two colors. The start_at and end_at parameters are values between 0 and 1.
typedef struct {
  RGBA start_color;
  RGBA end_color;
  f32 start_at; // 0 to 1
  f32 end_at; // 0 to 1
} C9_Gradient;

// Get the color at a given position in a gradient.
RGBA get_gradient_color(C9_Gradient gradient, f32 position) {
  // Destruct the gradient
  RGBA start_color = gradient.start_color;
  RGBA end_color = gradient.end_color;
  f32 start_at = gradient.start_at;
  f32 end_at = gradient.end_at;
  // Default to 1 if the end_at is 0
  end_at = end_at == 0 ? 1 : end_at;

  // If the position is outside the gradient, return the start or end color
  if (position < start_at) return start_color;
  if (position > end_at) return end_color;

  // The current position between the start and end of the gradient expressed as a value between 0 and 1.
  f32 normalized_position = (position - start_at) / (end_at - start_at);

  // Calculate red
  u8 start_r = red(start_color);
  u8 end_r = red(end_color);
  u8 r = start_r + (end_r - start_r) * normalized_position;

  // Calculate green
  u8 start_g = green(start_color);
  u8 end_g = green(end_color);
  u8 g = start_g + (end_g - start_g) * normalized_position;

  // Calculate blue
  u8 start_b = blue(start_color);
  u8 end_b = blue(end_color);
  u8 b = start_b + (end_b - start_b) * normalized_position;

  // Calculate alpha
  u8 start_a = alpha(start_color);
  u8 end_a = alpha(end_color);
  u8 a = start_a + (end_a - start_a) * normalized_position;

  RGBA color = (r << 24) | (g << 16) | (b << 8) | a;
  return color;
}

// Get the color width of the gradient and determine what dithering ammount should be applied
f32 get_dither_spread(C9_Gradient gradient) {
  RGBA start_color = gradient.start_color;
  RGBA end_color = gradient.end_color;
  f32 gradient_span = gradient.end_at - gradient.start_at;
  if (gradient_span <= 0) {
    gradient_span = 1.0;
  }

  // Sum and weight each channel (YIQ luminance formula)
  f32 red_spread = 0.299 * abs(red(start_color) - red(end_color));
  f32 green_spread = 0.587 * abs(green(start_color) - green(end_color));
  f32 blue_spread = 0.114 * abs(blue(start_color) - blue(end_color));
  
  i32 total_spread = red_spread + green_spread + blue_spread;
  if (total_spread != 0) {
    return gradient_span / total_spread;
  }
  return 0.0;
}

// Get the color at a given position in a gradient.
RGBA get_dithered_gradient_color(C9_Gradient gradient, f32 position, f32 random_variation) {
  // Destruct the gradient
  RGBA start_color = gradient.start_color;
  RGBA end_color = gradient.end_color;
  f32 start_at = gradient.start_at;
  f32 end_at = gradient.end_at;
  // Default to 1 if the end_at is 0
  end_at = end_at == 0 ? 1 : end_at;

  // If the position is outside the gradient, return the start or end color
  if (position < start_at) return start_color;
  if (position > end_at) return end_color;

  // The current position between the start and end of the gradient expressed as a value between 0 and 1.
  f32 normalized_position = (position + random_variation - start_at) / (end_at - start_at);

  // Prevent overflow
  if (normalized_position < 0) {
    normalized_position = 0.0;
  } else if (normalized_position > 1) {
    normalized_position = 1.0;
  }

  // Calculate red
  u8 start_r = red(start_color);
  u8 end_r = red(end_color);
  u8 r = start_r + (end_r - start_r) * normalized_position;

  // Calculate green
  u8 start_g = green(start_color);
  u8 end_g = green(end_color);
  u8 g = start_g + (end_g - start_g) * normalized_position;

  // Calculate blue
  u8 start_b = blue(start_color);
  u8 end_b = blue(end_color);
  u8 b = start_b + (end_b - start_b) * normalized_position;

  RGBA color = (r << 24) | (g << 16) | (b << 8) | 0xFF;
  return color;
}

#define C9_COLOR
#endif