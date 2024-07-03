#ifndef C9_COLOR

#include "types.h" // u8, u32, f32

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
} Gradient;

// Get the color at a given position in a gradient.
RGBA GetGradientColorAt(Gradient gradient, f32 position) {
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
  f32 normalizedPosition = (position - start_at) / (end_at - start_at);

  // Calculate red
  u8 start_r = red(start_color);
  u8 end_r = red(end_color);
  u8 r = start_r + (end_r - start_r) * normalizedPosition;

  // Calculate green
  u8 start_g = green(start_color);
  u8 end_g = green(end_color);
  u8 g = start_g + (end_g - start_g) * normalizedPosition;

  // Calculate blue
  u8 start_b = blue(start_color);
  u8 end_b = blue(end_color);
  u8 b = start_b + (end_b - start_b) * normalizedPosition;

  // Calculate alpha
  u8 start_a = alpha(start_color);
  u8 end_a = alpha(end_color);
  u8 a = start_a + (end_a - start_a) * normalizedPosition;

  RGBA color = (r << 24) | (g << 16) | (b << 8) | a;
  return color;
}

C9_RGB rgbaToRgb(RGBA color) {
  return (C9_RGB){
    .r = (color & 0xFF000000) >> 24,
    .g = (color & 0x00FF0000) >> 16,
    .b = (color & 0x0000FF00) >> 8
  };
}

typedef struct {
  u8 r;
  u8 g;
  u8 b;
} C9_RGB;

typedef struct {
  C9_RGB start;
  C9_RGB end;
} C9_Gradient;

// getGradientColor returns a color between the start and end colors of a gradient. The t parameter is a value between 0 and 1.
C9_RGB getGradientColor(C9_Gradient gradient, f32 t) {
  return (C9_RGB){
    .r = gradient.start.r + (gradient.end.r - gradient.start.r) * t,
    .g = gradient.start.g + (gradient.end.g - gradient.start.g) * t,
    .b = gradient.start.b + (gradient.end.b - gradient.start.b) * t
  };
}

#define C9_COLOR
#endif