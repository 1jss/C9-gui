#ifndef C9_COLOR

#include "types.c" // u8, u32, f32
#include "blue_noise.c" // get_blue_noise_value

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

RGBA set_alpha(RGBA color, u8 alpha) {
  return (color & 0xFFFFFF00) | alpha;
}

RGBA blend_colors(RGBA color_1, RGBA color_2) {
  if(alpha(color_1) == 0) return color_2;
  if(alpha(color_2) == 0) return color_1;
  u8 r_1 = (color_1 & 0xFF000000) >> 24;
  u8 g_1 = (color_1 & 0x00FF0000) >> 16;
  u8 b_1 = (color_1 & 0x0000FF00) >> 8;
  u8 a_1 = color_1 & 0x000000FF;
  u8 r_2 = (color_2 & 0xFF000000) >> 24;
  u8 g_2 = (color_2 & 0x00FF0000) >> 16;
  u8 b_2 = (color_2 & 0x0000FF00) >> 8;
  u8 a_2 = color_2 & 0x000000FF;
  u8 blend_r = (a_1 * r_1 + (255 - a_1) * r_2) / 255;
  u8 blend_g = (a_1 * g_1 + (255 - a_1) * g_2) / 255;
  u8 blend_b = (a_1 * b_1 + (255 - a_1) * b_2) / 255;
  u8 blend_a = a_1 + (255 - a_1) * a_2 / 255;
  return (blend_r << 24) | (blend_g << 16) | (blend_b << 8) | blend_a;
}

// Add overlay color to base color with alpha blending (0-255)
RGBA blend_alpha(RGBA base_color, RGBA overlay_color, u8 alpha) {
  if(base_color == 0) {
    return (overlay_color & 0xFFFFFF00) | alpha;
  }
  u8 base_r = (base_color & 0xFF000000) >> 24;
  u8 base_g = (base_color & 0x00FF0000) >> 16;
  u8 base_b = (base_color & 0x0000FF00) >> 8;
  u8 overlay_r = (overlay_color & 0xFF000000) >> 24;
  u8 overlay_g = (overlay_color & 0x00FF0000) >> 16;
  u8 overlay_b = (overlay_color & 0x0000FF00) >> 8;
  u8 blend_r = (alpha * overlay_r + (255 - alpha) * base_r) / 255;
  u8 blend_g = (alpha * overlay_g + (255 - alpha) * base_g) / 255;
  u8 blend_b = (alpha * overlay_b + (255 - alpha) * base_b) / 255;  
  return (blend_r << 24) | (blend_g << 16) | (blend_b << 8) | 255;
}

RGBA RGBA_from_u8(u8 r, u8 g, u8 b, u8 a) {
  return (r << 24) | (g << 16) | (b << 8) | a;
}

// Gradient is a struct that represents a color gradient between two colors. The start_at and end_at parameters are values between 0 and 1.
typedef struct {
  RGBA start_color;
  RGBA end_color;
  f32 start_at; // 0 to 1
  f32 end_at; // 0 to 1
} C9_Gradient;

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