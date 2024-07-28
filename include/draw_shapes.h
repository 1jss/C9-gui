#ifndef C9_DRAW_SHAPES

#include <SDL2/SDL.h>
#include <stdbool.h> // bool
#include "SDL_image.h"
#include "SDL_ttf.h" // TTF_RenderUTF8_Blended
#include "color.h" // RGBA, get_dithered_gradient_color, C9_Gradient, red, green, blue, alpha
#include "types.h" // u8, f32, i32

void draw_image(SDL_Renderer *renderer, char *image_url, SDL_Rect image_position) {
  SDL_Texture *texture = IMG_LoadTexture(renderer, image_url);
  if (texture != NULL) {
    SDL_RenderCopy(renderer, texture, NULL, &image_position);
    SDL_DestroyTexture(texture);
  }
}

void draw_text(SDL_Renderer *renderer, TTF_Font *font, char *text, i32 x, i32 y, RGBA color) {
  // Check if text has any content
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

// Draws a filled rectangle with optional superellipse corners
void draw_filled_rectangle(SDL_Renderer *renderer, SDL_Rect rectangle, i32 corner_radius, RGBA background_color) {
  u8 r = red(background_color);
  u8 g = green(background_color);
  u8 b = blue(background_color);

  if (corner_radius > 0) {
    // Cap corner radius to half of the rectangle width or height
    if (2 * corner_radius >= rectangle.w || 2 * corner_radius >= rectangle.h) {
      corner_radius = rectangle.w < rectangle.h ? rectangle.w / 2 : rectangle.h / 2;
    }

    // Set the drawing color to the background color
    SDL_SetRenderDrawColor(renderer, r, g, b, 255);

    // Fill the area between the top corners
    SDL_Rect top_rect = {
      .x = rectangle.x + corner_radius,
      .y = rectangle.y,
      .w = rectangle.w - 2 * corner_radius,
      .h = corner_radius
    };
    SDL_RenderFillRect(renderer, &top_rect);

    // Fill the area between the bottom corners
    SDL_Rect bottom_rect = {
      .x = rectangle.x + corner_radius,
      .y = rectangle.y + rectangle.h - corner_radius,
      .w = rectangle.w - 2 * corner_radius,
      .h = corner_radius
    };
    SDL_RenderFillRect(renderer, &bottom_rect);

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

  // Set the drawing color to the background color
  SDL_SetRenderDrawColor(renderer, r, g, b, 255);

  // Fill the center area
  SDL_Rect center_rect = {
    .x = rectangle.x,
    .y = rectangle.y + corner_radius,
    .w = rectangle.w,
    .h = rectangle.h - 2 * corner_radius
  };
  SDL_RenderFillRect(renderer, &center_rect);
}

void draw_horizontal_gradient_rectangle(SDL_Renderer *renderer, SDL_Rect rectangle, i32 corner_radius, C9_Gradient gradient) {
  f32 dither_spread = get_dither_spread(gradient);
  f32 one_percent_width = 1.0 / rectangle.w; // avoid division in loops

  if (corner_radius > 0) {
    // Cap corner radius to half of the rectangle width or height
    if (2 * corner_radius >= rectangle.w || 2 * corner_radius >= rectangle.h) {
      corner_radius = rectangle.w < rectangle.h ? rectangle.w / 2 : rectangle.h / 2;
    }

    // Fill the area between the left corners
    for (i32 x = 0; x < corner_radius; x++) {
      f32 percent = x * one_percent_width;
      i32 render_x = rectangle.x + x;
      for (i32 y = corner_radius; y <= rectangle.h - corner_radius; y++) {
        f32 random_variation = get_blue_noise_value(x, y) * dither_spread;
        RGBA color = get_dithered_gradient_color(gradient, percent, random_variation);
        SDL_SetRenderDrawColor(renderer, red(color), green(color), blue(color), 255);
        SDL_RenderDrawPoint(renderer, render_x, rectangle.y + y);
      }
    }

    // Fill the area between the right corners
    for (i32 x = rectangle.w - corner_radius; x < rectangle.w; x++) {
      f32 percent = x * one_percent_width;
      i32 render_x = rectangle.x + x;
      for (i32 y = corner_radius; y < rectangle.h - corner_radius; y++) {
        f32 random_variation = get_blue_noise_value(x, y) * dither_spread;
        RGBA color = get_dithered_gradient_color(gradient, percent, random_variation);
        SDL_SetRenderDrawColor(renderer, red(color), green(color), blue(color), 255);
        SDL_RenderDrawPoint(renderer, render_x, rectangle.y + y);
      }
    }

    // Center points for the corners
    i32 left_center_x = rectangle.x + corner_radius - 1;
    i32 right_center_x = rectangle.x + rectangle.w - corner_radius;
    i32 top_center_y = rectangle.y + corner_radius - 1;
    i32 bottom_center_y = rectangle.y + rectangle.h - corner_radius;

    // Set thresholds for antialiasing and boundary detection
    f32 antialiasing_threshold = pow(corner_radius - 1, 4);
    f32 boundary_squared = pow(corner_radius, 4);
    f32 inverse_difference = 1 / (boundary_squared - antialiasing_threshold);
    bool inside_shape = false;
    f32 opacity = 1.0;
    f32 percent = 0.0; // Gradient position
    RGBA color = 0x000000FF; // Current drawing color

    // Calculate points for one quadrant and mirror it the other quadrants
    for (i32 x = 0; x <= corner_radius; x++) {
      for (i32 y = 0; y <= corner_radius; y++) {
        f32 distance_squared = pow(x, 4) + pow(y, 4);
        inside_shape = false;
        // Check if the point is within the solid part of the corner
        if (distance_squared <= antialiasing_threshold) {
          opacity = 1.0;
          inside_shape = true;
        }
        // Apply antialiasing for points near the edge of the corner
        else if (distance_squared <= boundary_squared) {
          if (distance_squared > antialiasing_threshold) {
            // Calculate opacity for antialiasing effect
            opacity = (boundary_squared - distance_squared) * inverse_difference;
            // Clamp opacity between 0 and 1
            opacity = clamp(opacity, 0.0, 1.0);
          }
          inside_shape = true;
        }
        // Draw the corner points in all four quadrants if inside the shape
        if (inside_shape == true) {
          f32 random_variation = get_blue_noise_value(x, y) * dither_spread;

          // Calculate gradient position for left corners
          percent = (corner_radius - x - 1) * one_percent_width;
          color = get_dithered_gradient_color(gradient, percent, random_variation);
          SDL_SetRenderDrawColor(renderer, red(color), green(color), blue(color), (u8)(opacity * 255));

          // Top left quadrant
          SDL_RenderDrawPoint(renderer, left_center_x - x, top_center_y - y);
          // Bottom left quadrant
          SDL_RenderDrawPoint(renderer, left_center_x - x, bottom_center_y + y);

          // Calculate gradient position for right corners
          percent = (rectangle.w - corner_radius + x) * one_percent_width;
          color = get_dithered_gradient_color(gradient, percent, random_variation);
          SDL_SetRenderDrawColor(renderer, red(color), green(color), blue(color), (u8)(opacity * 255));

          // Top right quadrant
          SDL_RenderDrawPoint(renderer, right_center_x + x, top_center_y - y);
          // Bottom right quadrant
          SDL_RenderDrawPoint(renderer, right_center_x + x, bottom_center_y + y);
        }
      }
    }
  }

  // Fill the center area
  for (i32 x = corner_radius; x < rectangle.w - corner_radius; x++) {
    f32 percent = x * one_percent_width;
    i32 render_x = rectangle.x + x;
    for (i32 y = 0; y < rectangle.h; y++) {
      f32 random_variation = get_blue_noise_value(x, y) * dither_spread;
      RGBA color = get_dithered_gradient_color(gradient, percent, random_variation);
      SDL_SetRenderDrawColor(renderer, red(color), green(color), blue(color), 255);
      SDL_RenderDrawPoint(renderer, render_x, rectangle.y + y);
    }
  }
}

void draw_vertical_gradient_rectangle(SDL_Renderer *renderer, SDL_Rect rectangle, i32 corner_radius, C9_Gradient gradient) {
  f32 dither_spread = get_dither_spread(gradient);
  f32 one_percent_height = 1.0 / rectangle.h; // avoid division in loops

  if (corner_radius > 0) {
    // Cap corner radius to half of the rectangle width or height
    if (2 * corner_radius >= rectangle.w || 2 * corner_radius >= rectangle.h) {
      corner_radius = rectangle.w < rectangle.h ? rectangle.w / 2 : rectangle.h / 2;
    }

    // Fill the area between the top corners
    for (i32 y = 0; y < corner_radius; y++) {
      f32 percent = y * one_percent_height;
      i32 render_y = rectangle.y + y;
      for (i32 x = corner_radius; x < rectangle.w - corner_radius; x++) {
        f32 random_variation = get_blue_noise_value(x, y) * dither_spread;
        RGBA color = get_dithered_gradient_color(gradient, percent, random_variation);
        SDL_SetRenderDrawColor(renderer, red(color), green(color), blue(color), 255);
        SDL_RenderDrawPoint(renderer, rectangle.x + x, render_y);
      }
    }

    // Fill the area between the bottom corners
    for (i32 y = rectangle.h - corner_radius; y < rectangle.h; y++) {
      f32 percent = y * one_percent_height;
      i32 render_y = rectangle.y + y;
      for (i32 x = corner_radius; x < rectangle.w - corner_radius; x++) {
        f32 random_variation = get_blue_noise_value(x, y) * dither_spread;
        RGBA color = get_dithered_gradient_color(gradient, percent, random_variation);
        SDL_SetRenderDrawColor(renderer, red(color), green(color), blue(color), 255);
        SDL_RenderDrawPoint(renderer, rectangle.x + x, render_y);
      }
    }

    // Center points for the corners
    i32 left_center_x = rectangle.x + corner_radius - 1;
    i32 right_center_x = rectangle.x + rectangle.w - corner_radius;
    i32 top_center_y = rectangle.y + corner_radius - 1;
    i32 bottom_center_y = rectangle.y + rectangle.h - corner_radius;

    // Set thresholds for antialiasing and boundary detection
    f32 antialiasing_threshold = pow(corner_radius - 1, 4);
    f32 boundary_squared = pow(corner_radius, 4);
    f32 inverse_difference = 1 / (boundary_squared - antialiasing_threshold);
    bool inside_shape = false;
    f32 opacity = 1.0;
    f32 percent = 0.0; // Gradient position
    RGBA color = 0x000000FF; // Current drawing color

    // Calculate points for one quadrant and mirror it the other quadrants
    for (i32 x = 0; x <= corner_radius; x++) {
      for (i32 y = 0; y <= corner_radius; y++) {
        f32 distance_squared = pow(x, 4) + pow(y, 4);
        inside_shape = false;
        // Check if the point is within the solid part of the corner
        if (distance_squared <= antialiasing_threshold) {
          opacity = 1.0;
          inside_shape = true;
        }
        // Apply antialiasing for points near the edge of the corner
        else if (distance_squared <= boundary_squared) {
          if (distance_squared > antialiasing_threshold) {
            // Calculate opacity for antialiasing effect
            opacity = (boundary_squared - distance_squared) * inverse_difference;
            // Clamp opacity between 0 and 1
            opacity = clamp(opacity, 0.0, 1.0);
          }
          inside_shape = true;
        }
        // Draw the corner points in all four quadrants if inside the shape
        if (inside_shape == true) {
          f32 random_variation = get_blue_noise_value(x, y) * dither_spread;

          // Calculate gradient position for top corners
          percent = (corner_radius - y - 1) * one_percent_height;
          color = get_dithered_gradient_color(gradient, percent, random_variation);
          SDL_SetRenderDrawColor(renderer, red(color), green(color), blue(color), (u8)(opacity * 255));

          // Top left quadrant
          SDL_RenderDrawPoint(renderer, left_center_x - x, top_center_y - y);
          // Top right quadrant
          SDL_RenderDrawPoint(renderer, right_center_x + x, top_center_y - y);

          // Calculate gradient position for bottom corners
          percent = (rectangle.h - corner_radius + y) * one_percent_height;
          color = get_dithered_gradient_color(gradient, percent, random_variation);
          SDL_SetRenderDrawColor(renderer, red(color), green(color), blue(color), (u8)(opacity * 255));

          // Bottom right quadrant
          SDL_RenderDrawPoint(renderer, right_center_x + x, bottom_center_y + y);
          // Bottom left quadrant
          SDL_RenderDrawPoint(renderer, left_center_x - x, bottom_center_y + y);
        }
      }
    }
  }

  // Fill the center area
  for (i32 y = corner_radius; y < rectangle.h - corner_radius; y++) {
    f32 percent = y * one_percent_height;
    i32 render_y = rectangle.y + y;
    for (i32 x = 0; x < rectangle.w; x++) {
      f32 random_variation = get_blue_noise_value(x, y) * dither_spread;
      RGBA color = get_dithered_gradient_color(gradient, percent, random_variation);
      SDL_SetRenderDrawColor(renderer, red(color), green(color), blue(color), 255);
      SDL_RenderDrawPoint(renderer, rectangle.x + x, render_y);
    }
  }
}

// This type is the same as Border in element_tree, but defined here to keep some modularity. This BorderSize type is imported by the renderer.
typedef struct {
  i32 top;
  i32 right;
  i32 bottom;
  i32 left;
} BorderSize;

i32 largest_border(BorderSize border) {
  i32 largest = 0;
  if (border.top > largest) largest = border.top;
  if (border.right > largest) largest = border.right;
  if (border.bottom > largest) largest = border.bottom;
  if (border.left > largest) largest = border.left;
  return largest;
}

void draw_rectangle_with_border(SDL_Renderer *renderer, SDL_Rect rectangle, i32 corner_radius, BorderSize border, RGBA border_color, RGBA background_color) {
  draw_filled_rectangle(renderer, rectangle, corner_radius, border_color);
  SDL_Rect inner_rectangle = {
    .x = rectangle.x + border.left,
    .y = rectangle.y + border.top,
    .w = rectangle.w - border.left - border.right,
    .h = rectangle.h - border.top - border.bottom
  };
  i32 inner_corner_radius = corner_radius - largest_border(border);
  if (inner_corner_radius < 0) {
    inner_corner_radius = 0;
  }
  draw_filled_rectangle(renderer, inner_rectangle, inner_corner_radius, background_color);
}

void draw_horizontal_gradient_rectangle_with_border(SDL_Renderer *renderer, SDL_Rect rectangle, i32 corner_radius, BorderSize border, RGBA border_color, C9_Gradient background_gradient) {
  draw_filled_rectangle(renderer, rectangle, corner_radius, border_color);
  SDL_Rect inner_rectangle = {
    .x = rectangle.x + border.left,
    .y = rectangle.y + border.top,
    .w = rectangle.w - border.left - border.right,
    .h = rectangle.h - border.top - border.bottom
  };
  i32 inner_corner_radius = corner_radius - largest_border(border);
  if (inner_corner_radius < 0) {
    inner_corner_radius = 0;
  }
  draw_horizontal_gradient_rectangle(renderer, inner_rectangle, inner_corner_radius, background_gradient);
}

void draw_vertical_gradient_rectangle_with_border(SDL_Renderer *renderer, SDL_Rect rectangle, i32 corner_radius, BorderSize border, RGBA border_color, C9_Gradient background_gradient) {
  draw_filled_rectangle(renderer, rectangle, corner_radius, border_color);
  SDL_Rect inner_rectangle = {
    .x = rectangle.x + border.left,
    .y = rectangle.y + border.top,
    .w = rectangle.w - border.left - border.right,
    .h = rectangle.h - border.top - border.bottom
  };
  i32 inner_corner_radius = corner_radius - largest_border(border);
  if (inner_corner_radius < 0) {
    inner_corner_radius = 0;
  }
  draw_vertical_gradient_rectangle(renderer, inner_rectangle, inner_corner_radius, background_gradient);
}

#define C9_DRAW_SHAPES
#endif