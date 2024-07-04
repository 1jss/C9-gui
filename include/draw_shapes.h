#ifndef C9_DRAW_SHAPES
#include <SDL2/SDL.h>
#include <math.h> // cosf, sinf, powf
#include "color.h" // RGBA, getGradientColor, C9_Gradient, red, green, blue, alpha
#include "types.h" // u8, f32, i32

f32 calculate_superellipse_radius(f32 radius, f32 t) {
  f32 radius_pow_2 = radius * radius;
  f32 radius_cos_t = radius * cosf(t);
  f32 radius_sin_t = radius * sinf(t);
  f32 cos_pow_4 = radius_cos_t * radius_cos_t * radius_cos_t * radius_cos_t;
  f32 sin_pow_4 = radius_sin_t * radius_sin_t * radius_sin_t * radius_sin_t;
  return radius_pow_2 / powf(cos_pow_4 + sin_pow_4, 0.25f);
}

void draw_superellipse_border_point(SDL_Renderer *renderer, i32 center_x, i32 center_y, i32 radius, f32 t, RGBA color) {
  f32 rad = calculate_superellipse_radius(radius, t);
  f32 float_x = 1.0 * center_x + rad * cos(t);
  f32 float_y = 1.0 * center_y + rad * sin(t);
  i32 nearest_x = round(float_x);
  i32 nearest_y = round(float_y);
  i32 left_x = (i32)float_x;
  i32 top_y = (i32)float_y;
  i32 right_x = left_x + 1;
  i32 bottom_y = top_y + 1;

  // Smudge rightwards
  if (t <= M_PI / 4 || t >= 7 * M_PI / 4) {
    f32 x_diff = float_x - left_x;
    u8 opacity = (1 - x_diff) * 255;
    SDL_SetRenderDrawColor(renderer, red(color), green(color), blue(color), opacity);
    SDL_RenderDrawPoint(renderer, left_x, nearest_y);
    SDL_SetRenderDrawColor(renderer, red(color), green(color), blue(color), 255 - opacity);
    SDL_RenderDrawPoint(renderer, right_x, nearest_y);
  }
  // Smudge downwards
  else if (t <= 3 * M_PI / 4) {
    f32 y_diff = float_y - top_y;
    u8 opacity = (1 - y_diff) * 255;
    SDL_SetRenderDrawColor(renderer, red(color), green(color), blue(color), opacity);
    SDL_RenderDrawPoint(renderer, nearest_x, top_y);
    SDL_SetRenderDrawColor(renderer, red(color), green(color), blue(color), 255 - opacity);
    SDL_RenderDrawPoint(renderer, nearest_x, bottom_y);
  }
  // Smudge leftwards
  else if (t <= 5 * M_PI / 4) {
    f32 x_diff = float_x - right_x;
    u8 opacity = (1 + x_diff) * 255;
    SDL_SetRenderDrawColor(renderer, red(color), green(color), blue(color), opacity);
    SDL_RenderDrawPoint(renderer, right_x, nearest_y);
    SDL_SetRenderDrawColor(renderer, red(color), green(color), blue(color), 255 - opacity);
    SDL_RenderDrawPoint(renderer, left_x, nearest_y);
  }
  // Smudge upwards
  else {
    f32 y_diff = float_y - bottom_y;
    u8 opacity = (1 + y_diff) * 255;
    SDL_SetRenderDrawColor(renderer, red(color), green(color), blue(color), opacity);
    SDL_RenderDrawPoint(renderer, nearest_x, bottom_y);
    SDL_SetRenderDrawColor(renderer, red(color), green(color), blue(color), 255 - opacity);
    SDL_RenderDrawPoint(renderer, nearest_x, top_y);
  }
}

void draw_filled_superellipse_border_point(SDL_Renderer *renderer, i32 center_x, i32 center_y, i32 radius, f32 t, RGBA color) {
  f32 rad = calculate_superellipse_radius(radius, t);
  f32 float_x = 1.0 * center_x + rad * cos(t);
  f32 float_y = 1.0 * center_y + rad * sin(t);
  i32 nearest_x = round(float_x);
  i32 nearest_y = round(float_y);
  i32 left_x = (i32)float_x;
  i32 top_y = (i32)float_y;
  i32 right_x = left_x + 1;
  i32 bottom_y = top_y + 1;

  // Smudge rightwards
  if (t <= M_PI / 4 || t >= 7 * M_PI / 4) {
    f32 x_diff = float_x - left_x;
    u8 opacity = (1 - x_diff) * 255;
    SDL_SetRenderDrawColor(renderer, red(color), green(color), blue(color), opacity);
    SDL_RenderDrawPoint(renderer, left_x, nearest_y);
    SDL_SetRenderDrawColor(renderer, red(color), green(color), blue(color), 255 - opacity);
    SDL_RenderDrawPoint(renderer, right_x, nearest_y);
    // Fill to the center
    for (i32 i = center_x; i <= left_x; i++) {
      SDL_SetRenderDrawColor(renderer, red(color), green(color), blue(color), 255);
      SDL_RenderDrawPoint(renderer, i, nearest_y);
    }
  }
  // Smudge downwards
  else if (t <= 3 * M_PI / 4) {
    f32 y_diff = float_y - top_y;
    u8 opacity = (1 - y_diff) * 255;
    SDL_SetRenderDrawColor(renderer, red(color), green(color), blue(color), opacity);
    SDL_RenderDrawPoint(renderer, nearest_x, top_y);
    SDL_SetRenderDrawColor(renderer, red(color), green(color), blue(color), 255 - opacity);
    SDL_RenderDrawPoint(renderer, nearest_x, bottom_y);
    // Fill to the center
    for (i32 i = center_y; i <= top_y; i++) {
      SDL_SetRenderDrawColor(renderer, red(color), green(color), blue(color), 255);
      SDL_RenderDrawPoint(renderer, nearest_x, i);
    }
  }
  // Smudge leftwards
  else if (t <= 5 * M_PI / 4) {
    f32 x_diff = float_x - right_x;
    u8 opacity = (1 + x_diff) * 255;
    SDL_SetRenderDrawColor(renderer, red(color), green(color), blue(color), opacity);
    SDL_RenderDrawPoint(renderer, right_x, nearest_y);
    SDL_SetRenderDrawColor(renderer, red(color), green(color), blue(color), 255 - opacity);
    SDL_RenderDrawPoint(renderer, left_x, nearest_y);
    // Fill to the center
    for (i32 i = center_x; i >= right_x; i--) {
      SDL_SetRenderDrawColor(renderer, red(color), green(color), blue(color), 255);
      SDL_RenderDrawPoint(renderer, i, nearest_y);
    }
  }
  // Smudge upwards
  else {
    f32 y_diff = float_y - bottom_y;
    u8 opacity = (1 + y_diff) * 255;
    SDL_SetRenderDrawColor(renderer, red(color), green(color), blue(color), opacity);
    SDL_RenderDrawPoint(renderer, nearest_x, bottom_y);
    SDL_SetRenderDrawColor(renderer, red(color), green(color), blue(color), 255 - opacity);
    SDL_RenderDrawPoint(renderer, nearest_x, top_y);
    // Fill to the center
    for (i32 i = center_y; i >= bottom_y; i--) {
      SDL_SetRenderDrawColor(renderer, red(color), green(color), blue(color), 255);
      SDL_RenderDrawPoint(renderer, nearest_x, i);
    }
  }
}

// Draws a superellipse with a given center and radius
void draw_superellipse(SDL_Renderer *renderer, i32 center_x, i32 center_y, i32 radius, RGBA color) {
  SDL_SetRenderDrawColor(renderer, red(color), green(color), blue(color), SDL_ALPHA_OPAQUE);
  // Calculate radian step size so that every border pixel is drawn
  f32 step = 0.85 / radius;
  for (f32 t = 0; t < 2 * M_PI; t += step) {
    draw_superellipse_border_point(renderer, center_x, center_y, radius, t, color);
  }
}

// Draws a filled superellipse with a given center and radius
void draw_filled_superellipse(SDL_Renderer *renderer, i32 center_x, i32 center_y, i32 radius, RGBA color) {
  SDL_SetRenderDrawColor(renderer, red(color), green(color), blue(color), SDL_ALPHA_OPAQUE);
  // Calculate radian step size so that every border pixel is drawn
  f32 step = 0.85 / radius;
  for (f32 t = 0; t < 2 * M_PI; t += step) {
    draw_filled_superellipse_border_point(renderer, center_x, center_y, radius, t, color);
  }
}

// Draws a rectangle with superellipse corners
void draw_rounded_rectangle(SDL_Renderer *renderer, i32 x, i32 y, i32 rectangle_width, i32 rectangle_height, i32 corner_radius, RGBA color) {
  // Cap corner radius to half of the rectangle width or height
  if (2 * corner_radius < rectangle_width || 2 * corner_radius < rectangle_height) {
    corner_radius = rectangle_width < rectangle_height ? rectangle_width / 2 : rectangle_height / 2;
  }
  SDL_SetRenderDrawColor(renderer, red(color), green(color), blue(color), SDL_ALPHA_OPAQUE);
  // Draw top and bottom lines
  for (i32 i = x + corner_radius; i <= x + rectangle_width - corner_radius; i++) {
    SDL_RenderDrawPoint(renderer, i, y);
    SDL_RenderDrawPoint(renderer, i, y + rectangle_height);
  }
  // Draw left and right lines
  for (i32 i = y + corner_radius; i <= y + rectangle_height - corner_radius; i++) {
    SDL_RenderDrawPoint(renderer, x, i);
    SDL_RenderDrawPoint(renderer, x + rectangle_width, i);
  }
  // Draw corners
  f32 step = 0.85 / corner_radius;
  for (f32 t = 0; t <= M_PI / 2; t += step) {
    // Bottom right corner (t)
    draw_superellipse_border_point(renderer, x + rectangle_width - corner_radius, y + rectangle_height - corner_radius, corner_radius, t, color);
    // Bottom left corner (t + M_PI / 2)
    draw_superellipse_border_point(renderer, x + corner_radius, y + rectangle_height - corner_radius, corner_radius, t + M_PI / 2, color);
    // Top left corner (t + M_PI)
    draw_superellipse_border_point(renderer, x + corner_radius, y + corner_radius, corner_radius, t + M_PI, color);
    // Top right corner (t + 3 * M_PI / 2)
    draw_superellipse_border_point(renderer, x + rectangle_width - corner_radius, y + corner_radius, corner_radius, t + 3 * M_PI / 2, color);
  }
}

// Draws a filled rectangle with superellipse corners
void draw_filled_rounded_rectangle(SDL_Renderer *renderer, i32 x, i32 y, i32 rectangle_width, i32 rectangle_height, i32 corner_radius, RGBA color) {
  // Smallest acceptable corner radius is 10px
  if (corner_radius < 10) {
    corner_radius = 10;
  }
  // Cap corner radius to half of the rectangle width or height
  if (2 * corner_radius >= rectangle_width || 2 * corner_radius >= rectangle_height) {
    corner_radius = rectangle_width < rectangle_height ? rectangle_width / 2 : rectangle_height / 2;
  }
  SDL_SetRenderDrawColor(renderer, red(color), green(color), blue(color), SDL_ALPHA_OPAQUE);
  // A rect that filles the top and bottom lines and the area between them
  SDL_Rect top_bottom_rect = {x + corner_radius, y, rectangle_width - 2 * corner_radius + 1, rectangle_height + 1};
  SDL_RenderFillRect(renderer, &top_bottom_rect);
  // A rect that filles the left and right lines and the area between them
  SDL_Rect left_right_rect = {x, y + corner_radius, rectangle_width + 1, rectangle_height - 2 * corner_radius + 1};
  SDL_RenderFillRect(renderer, &left_right_rect);

  // Draw corners
  f32 step = 0.85 / corner_radius;
  for (f32 t = 0; t <= M_PI / 2; t += step) {
    // Bottom right corner (t)
    draw_filled_superellipse_border_point(renderer, x + rectangle_width - corner_radius, y + rectangle_height - corner_radius, corner_radius, t, color);
    // Bottom left corner (t + M_PI / 2)
    draw_filled_superellipse_border_point(renderer, x + corner_radius, y + rectangle_height - corner_radius, corner_radius, t + M_PI / 2, color);
    // Top left corner (t + M_PI)
    draw_filled_superellipse_border_point(renderer, x + corner_radius, y + corner_radius, corner_radius, t + M_PI, color);
    // Top right corner (t + 3 * M_PI / 2)
    draw_filled_superellipse_border_point(renderer, x + rectangle_width - corner_radius, y + corner_radius, corner_radius, t + 3 * M_PI / 2, color);
  }
}

void draw_rectangle_with_border(SDL_Renderer *renderer, i32 x, i32 y, i32 width, i32 height, i32 border_radius, i32 border_width, RGBA content_color, RGBA border_color) {
  draw_filled_rounded_rectangle(renderer, x, y, width, height, border_radius, content_color);
  draw_filled_rounded_rectangle(renderer, x + border_width, y + border_width, width - 2 * border_width, height - 2 * border_width, border_radius - border_width, border_color);
}

void draw_filled_rectangle(SDL_Renderer *renderer, i32 x, i32 y, i32 width, i32 height, RGBA color) {
  SDL_SetRenderDrawColor(renderer, red(color), green(color), blue(color), SDL_ALPHA_OPAQUE);
  SDL_Rect rect = {x, y, width, height};
  SDL_RenderFillRect(renderer, &rect);
}

void draw_horizontal_gradient(SDL_Renderer *renderer, i32 x, i32 y, i32 width, i32 height, C9_Gradient gradient) {
  for (i32 i = 0; i < width; i++) {
    f32 t = 1.0 * i / width;
    RGBA color = getGradientColor(gradient, t);
    SDL_SetRenderDrawColor(renderer, red(color), green(color), blue(color), SDL_ALPHA_OPAQUE);
    SDL_RenderDrawLine(renderer, x + i, y, x + i, y + height);
  }
}

void draw_vertical_gradient(SDL_Renderer *renderer, i32 x, i32 y, i32 width, i32 height, C9_Gradient gradient) {
  for (i32 i = 0; i < height; i++) {
    f32 t = 1.0 * i / height;
    RGBA color = getGradientColor(gradient, t);
    SDL_SetRenderDrawColor(renderer, red(color), green(color), blue(color), SDL_ALPHA_OPAQUE);
    SDL_RenderDrawLine(renderer, x, y + i, x + width, y + i);
  }
}

void draw_border(SDL_Renderer *renderer, i32 x, i32 y, i32 width, i32 height, i32 top, i32 right, i32 bottom, i32 left, RGBA color) {
  printf("Drawing border\n");
  // Draw top border
  if (top > 0) {
    draw_filled_rectangle(renderer, x, y, width, top, color);
  }
  // Draw right border
  if (right > 0) {
    draw_filled_rectangle(renderer, x + width - right, y, right, height, color);
  }
  // Draw bottom border
  if (bottom > 0) {
    draw_filled_rectangle(renderer, x, y + height - bottom, width, bottom, color);
  }
  // Draw left border
  if (left > 0) {
    draw_filled_rectangle(renderer, x, y, left, height, color);
  }
}

#define C9_DRAW_SHAPES
#endif