#ifndef C9_DRAW_SHAPES

#include <SDL2/SDL.h>
#include <math.h> // cosf, sinf, powf
#include "color.h" // RGBA, getGradientColor, C9_Gradient, red, green, blue, alpha
#include "SDL_ttf.h" // TTF_RenderText_Blended
#include "types.h" // u8, f32, i32

void draw_text(SDL_Renderer *renderer, TTF_Font *font, char *text, i32 x, i32 y, RGBA color) {
  const SDL_Color text_color = {red(color), green(color), blue(color), alpha(color)};
  SDL_Surface *surface = TTF_RenderText_Blended(font, text, text_color);
  SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
  SDL_Rect rect = {x, y, surface->w, surface->h};
  SDL_RenderCopy(renderer, texture, NULL, &rect);
  SDL_FreeSurface(surface);
  SDL_DestroyTexture(texture);
}

f32 calculate_superellipse_radius(f32 radius, f32 t) {
  f32 radius_pow_2 = radius * radius;
  f32 radius_cos_t = radius * cosf(t);
  f32 radius_sin_t = radius * sinf(t);
  f32 cos_pow_4 = radius_cos_t * radius_cos_t * radius_cos_t * radius_cos_t;
  f32 sin_pow_4 = radius_sin_t * radius_sin_t * radius_sin_t * radius_sin_t;
  return radius_pow_2 / powf(cos_pow_4 + sin_pow_4, 0.25f);
}

typedef struct {
  i32 center_x;
  i32 center_y;
  i32 radius;
} Circle;

void draw_superellipse_border_point(SDL_Renderer *renderer, Circle circle, f32 t, RGBA color) {
  f32 rad = calculate_superellipse_radius(circle.radius, t);
  f32 float_x = 1.0 * circle.center_x + rad * cos(t);
  f32 float_y = 1.0 * circle.center_y + rad * sin(t);
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
void draw_superellipse(SDL_Renderer *renderer, Circle circle, RGBA color) {
  SDL_SetRenderDrawColor(renderer, red(color), green(color), blue(color), SDL_ALPHA_OPAQUE);
  // Calculate radian step size so that every border pixel is drawn
  f32 step = 0.85 / circle.radius;
  for (f32 t = 0; t < 2 * M_PI; t += step) {
    draw_superellipse_border_point(renderer, circle, t, color);
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
void draw_rounded_rectangle(SDL_Renderer *renderer, SDL_Rect rectangle, i32 corner_radius, RGBA border_color) {
  // Cap corner radius to half of the rectangle width or height
  if (2 * corner_radius < rectangle.w || 2 * corner_radius < rectangle.h) {
    corner_radius = rectangle.w < rectangle.h ? rectangle.w / 2 : rectangle.h / 2;
  }
  SDL_SetRenderDrawColor(renderer, red(border_color), green(border_color), blue(border_color), SDL_ALPHA_OPAQUE);
  // Draw top and bottom lines
  for (i32 i = rectangle.x + corner_radius; i <= rectangle.x + rectangle.w - corner_radius; i++) {
    SDL_RenderDrawPoint(renderer, i, rectangle.y);
    SDL_RenderDrawPoint(renderer, i, rectangle.y + rectangle.h);
  }
  // Draw left and right lines
  for (i32 i = rectangle.y + corner_radius; i <= rectangle.y + rectangle.h - corner_radius; i++) {
    SDL_RenderDrawPoint(renderer, rectangle.x, i);
    SDL_RenderDrawPoint(renderer, rectangle.x + rectangle.w, i);
  }
  // Draw corners
  f32 step = 0.85 / corner_radius;
  for (f32 t = 0; t <= M_PI / 2; t += step) {
    // Bottom right corner (t)
    Circle bottom_right_corner = {
      .center_x = rectangle.x + rectangle.w - corner_radius,
      .center_y = rectangle.y + rectangle.h - corner_radius,
      .radius = corner_radius
    };
    draw_superellipse_border_point(renderer, bottom_right_corner, t, border_color);
    // Bottom left corner (t + M_PI / 2)
    Circle bottom_left_corner = {
      .center_x = rectangle.x + corner_radius,
      .center_y = rectangle.y + rectangle.h - corner_radius,
      .radius = corner_radius
    };
    draw_superellipse_border_point(renderer, bottom_left_corner, t + M_PI / 2, border_color);
    // Top left corner (t + M_PI)
    Circle top_left_corner = {
      .center_x = rectangle.x + corner_radius,
      .center_y = rectangle.y + corner_radius,
      .radius = corner_radius
    };
    draw_superellipse_border_point(renderer, top_left_corner, t + M_PI, border_color);
    // Top right corner (t + 3 * M_PI / 2)
    Circle top_right_corner = {
      .center_x = rectangle.x + rectangle.w - corner_radius,
      .center_y = rectangle.y + corner_radius,
      .radius = corner_radius
    };
    draw_superellipse_border_point(renderer, top_right_corner, t + 3 * M_PI / 2, border_color);
  }
}

// Draws a filled rectangle with superellipse corners
void draw_filled_rounded_rectangle(SDL_Renderer *renderer, SDL_Rect rectangle, i32 corner_radius, RGBA background_color) {
  // Smallest acceptable corner radius is 10px
  if (corner_radius < 10) {
    corner_radius = 10;
  }
  // Cap corner radius to half of the rectangle width or height
  if (2 * corner_radius >= rectangle.w || 2 * corner_radius >= rectangle.h) {
    corner_radius = rectangle.w < rectangle.h ? rectangle.w / 2 : rectangle.h / 2;
  }
  SDL_SetRenderDrawColor(renderer, red(background_color), green(background_color), blue(background_color), SDL_ALPHA_OPAQUE);
  // A rect that filles the top and bottom lines and the area between them
  SDL_Rect top_bottom_rect = {rectangle.x + corner_radius, rectangle.y, rectangle.w - 2 * corner_radius + 1, rectangle.h + 1};
  SDL_RenderFillRect(renderer, &top_bottom_rect);
  // A rect that filles the left and right lines and the area between them
  SDL_Rect left_right_rect = {rectangle.x, rectangle.y + corner_radius, rectangle.w + 1, rectangle.h - 2 * corner_radius + 1};
  SDL_RenderFillRect(renderer, &left_right_rect);

  // Draw corners
  f32 step = 0.85 / corner_radius;
  for (f32 t = 0; t <= M_PI / 2; t += step) {
    // Bottom right corner (t)
    draw_filled_superellipse_border_point(renderer, rectangle.x + rectangle.w - corner_radius, rectangle.y + rectangle.h - corner_radius, corner_radius, t, background_color);
    // Bottom left corner (t + M_PI / 2)
    draw_filled_superellipse_border_point(renderer, rectangle.x + corner_radius, rectangle.y + rectangle.h - corner_radius, corner_radius, t + M_PI / 2, background_color);
    // Top left corner (t + M_PI)
    draw_filled_superellipse_border_point(renderer, rectangle.x + corner_radius, rectangle.y + corner_radius, corner_radius, t + M_PI, background_color);
    // Top right corner (t + 3 * M_PI / 2)
    draw_filled_superellipse_border_point(renderer, rectangle.x + rectangle.w - corner_radius, rectangle.y + corner_radius, corner_radius, t + 3 * M_PI / 2, background_color);
  }
}

void draw_rounded_rectangle_with_border(SDL_Renderer *renderer, SDL_Rect rectangle, i32 corner_radius, i32 border_width, RGBA border_color, RGBA background_color) {
  draw_filled_rounded_rectangle(renderer, rectangle, corner_radius, border_color);
  SDL_Rect inner_rectangle = {rectangle.x + border_width, rectangle.y + border_width, rectangle.w - 2 * border_width, rectangle.h - 2 * border_width};
  draw_filled_rounded_rectangle(renderer, inner_rectangle, corner_radius - border_width, background_color);
}

void draw_filled_rectangle(SDL_Renderer *renderer, SDL_Rect rectangle, RGBA color) {
  SDL_SetRenderDrawColor(renderer, red(color), green(color), blue(color), SDL_ALPHA_OPAQUE);
  SDL_Rect rect = {rectangle.x, rectangle.y, rectangle.w, rectangle.h};
  SDL_RenderFillRect(renderer, &rect);
}

void draw_horizontal_gradient(SDL_Renderer *renderer, SDL_Rect rectangle, C9_Gradient gradient) {
  for (i32 i = 0; i < rectangle.w; i++) {
    f32 t = 1.0 * i / rectangle.w;
    RGBA color = getGradientColor(gradient, t);
    SDL_SetRenderDrawColor(renderer, red(color), green(color), blue(color), SDL_ALPHA_OPAQUE);
    SDL_RenderDrawLine(renderer, rectangle.x + i, rectangle.y, rectangle.x + i, rectangle.y + rectangle.h);
  }
}

void draw_vertical_gradient(SDL_Renderer *renderer, SDL_Rect rectangle, C9_Gradient gradient) {
  for (i32 i = 0; i < rectangle.h; i++) {
    f32 t = 1.0 * i / rectangle.h;
    RGBA color = getGradientColor(gradient, t);
    SDL_SetRenderDrawColor(renderer, red(color), green(color), blue(color), SDL_ALPHA_OPAQUE);
    SDL_RenderDrawLine(renderer, rectangle.x, rectangle.y + i, rectangle.x + rectangle.w, rectangle.y + i);
  }
}

typedef struct {
  i32 top;
  i32 right;
  i32 bottom;
  i32 left;
} BorderSize;

void draw_border(SDL_Renderer *renderer, SDL_Rect rectangle, BorderSize border, RGBA border_color) {
  // Draw top border
  if (border.top > 0) {
    SDL_Rect top_border = {rectangle.x, rectangle.y, rectangle.w, border.top};
    draw_filled_rectangle(renderer, top_border, border_color);
  }
  // Draw right border
  if (border.right > 0) {
    SDL_Rect right_border = {rectangle.x + rectangle.w - border.right, rectangle.y, border.right, rectangle.h};
    draw_filled_rectangle(renderer, right_border, border_color);
  }
  // Draw bottom border
  if (border.bottom > 0) {
    SDL_Rect bottom_border = {rectangle.x, rectangle.y + rectangle.h - border.bottom, rectangle.w, border.bottom};
    draw_filled_rectangle(renderer, bottom_border, border_color);
  }
  // Draw left border
  if (border.left > 0) {
    SDL_Rect left_border = {rectangle.x, rectangle.y, border.left, rectangle.h};
    draw_filled_rectangle(renderer, left_border, border_color);
  }
}

#define C9_DRAW_SHAPES
#endif