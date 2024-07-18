#ifndef C9_DRAW_SHAPES

#include <SDL2/SDL.h>
#include <math.h> // cosf, sinf, powf
#include <stdbool.h> // bool
#include "SDL_ttf.h" // TTF_RenderText_Blended
#include "color.h" // RGBA, getGradientColor, C9_Gradient, red, green, blue, alpha
#include "types.h" // u8, f32, i32

void draw_text(SDL_Renderer *renderer, TTF_Font *font, char *text, i32 x, i32 y, RGBA color) {
  // check if text has any content
  if (text[0] != '\0') {
    const SDL_Color text_color = {red(color), green(color), blue(color), alpha(color)};
    SDL_Surface *surface = TTF_RenderUTF8_Blended(font, text, text_color);
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_Rect rect = {x, y, surface->w, surface->h};
    SDL_RenderCopy(renderer, texture, NULL, &rect);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
  }
}

f32 clamp(f32 value, f32 min, f32 max) {
  if (value < min) {
    return min;
  } else if (value > max) {
    return max;
  } else {
    return value;
  }
}

// Draws a filled rectangle with superellipse corners
void draw_filled_rounded_rectangle(SDL_Renderer *renderer, SDL_Rect rectangle, i32 corner_radius, RGBA background_color) {
  // Cap corner radius to half of the rectangle width or height
  if (2 * corner_radius >= rectangle.w || 2 * corner_radius >= rectangle.h) {
    corner_radius = rectangle.w < rectangle.h ? rectangle.w / 2 : rectangle.h / 2;
  }
  // Set the drawing color to the background color
  u8 r = red(background_color);
  u8 g = green(background_color);
  u8 b = blue(background_color);
  SDL_SetRenderDrawColor(renderer, r, g, b, 255);
  // Draw a rect that fills the top and bottom borders and the area between them
  SDL_Rect top_bottom_rect = {rectangle.x + corner_radius, rectangle.y, rectangle.w - 2 * corner_radius, rectangle.h};
  SDL_RenderFillRect(renderer, &top_bottom_rect);
  // Draw a rect that fills the left and right borders and the area between them
  SDL_Rect left_right_rect = {rectangle.x, rectangle.y + corner_radius, rectangle.w, rectangle.h - 2 * corner_radius};
  SDL_RenderFillRect(renderer, &left_right_rect);

  // Center points for the corners
  i32 left_center_x = rectangle.x + corner_radius - 1;
  i32 right_center_x = rectangle.x + rectangle.w - corner_radius;
  i32 top_center_y = rectangle.y + corner_radius - 1;
  i32 bottom_center_y = rectangle.y + rectangle.h - corner_radius;

  // Set thresholds for antialiasing and boundary detection
  f32 antialiasing_threshold = pow(corner_radius - 1, 4);
  f32 boundary_squared = pow(corner_radius, 4);
  bool inside_shape = false;

  // Calculate points for one quadrant and mirror it the other quadrants
  for (i32 x = 0; x <= corner_radius; x++) {
    for (i32 y = 0; y <= corner_radius; y++) {
      f32 distance_squared = pow(x, 4) + pow(y, 4);
      inside_shape = false;
      // Check if the point is within the solid part of the corner
      if (distance_squared <= antialiasing_threshold) {
        SDL_SetRenderDrawColor(renderer, r, g, b, 255);
        inside_shape = true;
      }
      // Apply antialiasing for points near the edge of the corner
      else if (distance_squared <= boundary_squared) {
        f32 opacity = 1.0;
        if (distance_squared > antialiasing_threshold) {
          // Calculate opacity for antialiasing effect
          opacity = (boundary_squared - distance_squared) / (boundary_squared - antialiasing_threshold);
          // Clamp opacity between 0 and 1
          opacity = clamp(opacity, 0.0, 1.0);
        }
        SDL_SetRenderDrawColor(renderer, r, g, b, (u8)(opacity * 255));
        inside_shape = true;
      }
      // Draw the corner points in all four quadrants if inside the shape
      if (inside_shape == true) {
        // Top left quadrant
        SDL_RenderDrawPoint(renderer, left_center_x - x, top_center_y - y);
        // Top right quadrant
        SDL_RenderDrawPoint(renderer, right_center_x + x, top_center_y - y);
        // Bottom right quadrant
        SDL_RenderDrawPoint(renderer, right_center_x + x, bottom_center_y + y);
        // Bottom left quadrant
        SDL_RenderDrawPoint(renderer, left_center_x - x, bottom_center_y + y);
      }
    }
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