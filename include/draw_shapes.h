#ifndef C9_DRAW_SHAPES

#include <SDL2/SDL.h>
#include <stdbool.h> // bool
#include "SDL_image.h"
#include "SDL_ttf.h" // TTF_RenderUTF8_Blended
#include "color.h" // RGBA, get_dithered_gradient_color, C9_Gradient, red, green, blue, alpha
#include "types.h" // u8, f32, i32

// Locked texture as pixel data
typedef struct {
  RGBA *pixels;
  i32 width;
} PixelData;

void renderer_draw_image(SDL_Renderer *renderer, char *image_url, SDL_Rect image_position) {
  SDL_Texture *texture = IMG_LoadTexture(renderer, image_url);
  if (texture != NULL) {
    SDL_RenderCopy(renderer, texture, NULL, &image_position);
    SDL_DestroyTexture(texture);
  }
}

void draw_image(PixelData target, char *image_url, SDL_Rect image_position) {
  SDL_Surface *surface = IMG_Load(image_url);
  if (surface != NULL) {
    SDL_LockSurface(surface);
    for (i32 y = 0; y < surface->h; y++) {
      if (y < image_position.h) {
        for (i32 x = 0; x < surface->w; x++) {
          if (x < image_position.w) {
            u8 *pixel = (u8 *)surface->pixels + y * surface->pitch + x * surface->format->BytesPerPixel;
            RGBA image_pixel = RGBA_from_u8(pixel[0], pixel[1], pixel[2], pixel[3]);
            target.pixels[(image_position.y + y) * target.width + image_position.y + x] = image_pixel;
          }
        }
      }
    }
    SDL_UnlockSurface(surface);
    SDL_FreeSurface(surface);
  }
}

void draw_text(PixelData target, TTF_Font *font, char *text, i32 x_pos, i32 y_pos, RGBA color) {
  // Check if text has any content
  if (text[0] != '\0') {
    const SDL_Color text_color = {red(color), green(color), blue(color), alpha(color)};
    SDL_Surface *surface = TTF_RenderUTF8_Blended(font, text, text_color);
    SDL_LockSurface(surface);
    // Loop over text pixels
    for (i32 y = 0; y < surface->h; y++) {
      for (i32 x = 0; x < surface->w; x++) {
        // Check if we're inside the target bounds
        if (x_pos + x < target.width) {
          // Get the pixel color from the text surface
          u8 *pixel = (u8 *)surface->pixels + y * surface->pitch + x * surface->format->BytesPerPixel;
          RGBA text_pixel = RGBA_from_u8(pixel[0], pixel[1], pixel[2], pixel[3]);
          RGBA target_pixel = target.pixels[(y_pos + y) * target.width + x_pos + x];
          RGBA blended_pixel = blend_colors(text_pixel, target_pixel);
          target.pixels[(y_pos + y) * target.width + x_pos + x] = blended_pixel;
        }
      }
    }
    SDL_UnlockSurface(surface);
    SDL_FreeSurface(surface);
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

void renderer_fill_rectangle(SDL_Renderer *renderer, SDL_Rect rectangle, RGBA color) {
  SDL_SetRenderDrawColor(renderer, red(color), green(color), blue(color), alpha(color));
  SDL_RenderFillRect(renderer, &rectangle);
}

// Draws a filled rectangle with optional superellipse corners
void draw_filled_rectangle(PixelData target, SDL_Rect rectangle, i32 corner_radius, RGBA background_color) {
  if (corner_radius > 0) {
    // Cap corner radius to half of the rectangle width or height
    if (2 * corner_radius > rectangle.w || 2 * corner_radius > rectangle.h) {
      corner_radius = rectangle.w < rectangle.h ? rectangle.w / 2 : rectangle.h / 2;
    }

    // Fill the area between the top corners
    SDL_Rect top_rect = {
      .x = rectangle.x + corner_radius,
      .y = rectangle.y,
      .w = rectangle.w - 2 * corner_radius,
      .h = corner_radius
    };
    for (i32 y = 0; y < top_rect.h; y++) {
      for (i32 x = 0; x < top_rect.w; x++) {
        target.pixels[(top_rect.y + y) * target.width + top_rect.x + x] = background_color;
      }
    }

    // Fill the area between the bottom corners
    SDL_Rect bottom_rect = {
      .x = rectangle.x + corner_radius,
      .y = rectangle.y + rectangle.h - corner_radius,
      .w = rectangle.w - 2 * corner_radius,
      .h = corner_radius
    };
    for (i32 y = 0; y < bottom_rect.h; y++) {
      for (i32 x = 0; x < bottom_rect.w; x++) {
        target.pixels[(bottom_rect.y + y) * target.width + bottom_rect.x + x] = background_color;
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
    bool inside_shape = false;

    RGBA draw_color = background_color;
    // Calculate points for one quadrant and mirror it the other quadrants
    for (i32 x = 0; x < corner_radius; x++) {
      for (i32 y = 0; y < corner_radius; y++) {
        f32 distance_squared = pow(x, 4) + pow(y, 4);
        inside_shape = false;
        // Check if the point is within the solid part of the corner
        if (distance_squared <= antialiasing_threshold) {
          draw_color = background_color;
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
          draw_color = set_alpha(background_color, (u8)(opacity * 255));

          // Blend inside of border with border color
          RGBA target_pixel = target.pixels[(top_center_y - y) * target.width + left_center_x - x];
          if (target_pixel != 0) {
            draw_color = blend_colors(draw_color, target_pixel);
          }
          inside_shape = true;
        }
        // Draw the corner points in all four quadrants if inside the shape
        if (inside_shape == true) {
          // Top left quadrant
          target.pixels[(top_center_y - y) * target.width + left_center_x - x] = draw_color;
          // Top right quadrant
          target.pixels[(top_center_y - y) * target.width + right_center_x + x] = draw_color;
          // Bottom right quadrant
          target.pixels[(bottom_center_y + y) * target.width + right_center_x + x] = draw_color;
          // Bottom left quadrant
          target.pixels[(bottom_center_y + y) * target.width + left_center_x - x] = draw_color;
        }
      }
    }
  }

  // Fill the center area
  SDL_Rect center_rect = {
    .x = rectangle.x,
    .y = rectangle.y + corner_radius,
    .w = rectangle.w,
    .h = rectangle.h - 2 * corner_radius
  };
  if (center_rect.w > 0 && center_rect.h > 0) {
    for (i32 y = 0; y < center_rect.h; y++) {
      for (i32 x = 0; x < center_rect.w; x++) {
        target.pixels[(center_rect.y + y) * target.width + center_rect.x + x] = background_color;
      }
    }
  }
}

void draw_horizontal_gradient_rectangle(PixelData target, SDL_Rect rectangle, i32 corner_radius, C9_Gradient gradient) {
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
        target.pixels[(rectangle.y + y) * target.width + render_x] = color;
      }
    }

    // Fill the area between the right corners
    for (i32 x = rectangle.w - corner_radius; x < rectangle.w; x++) {
      f32 percent = x * one_percent_width;
      i32 render_x = rectangle.x + x;
      for (i32 y = corner_radius; y < rectangle.h - corner_radius; y++) {
        f32 random_variation = get_blue_noise_value(x, y) * dither_spread;
        RGBA color = get_dithered_gradient_color(gradient, percent, random_variation);
        target.pixels[(rectangle.y + y) * target.width + render_x] = color;
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
    RGBA color = 0; // Current drawing color
    RGBA blend_color = 0; // Color to blend with

    // Calculate points for one quadrant and mirror it the other quadrants
    for (i32 x = 0; x < corner_radius; x++) {
      for (i32 y = 0; y < corner_radius; y++) {
        f32 distance_squared = pow(x, 4) + pow(y, 4);
        blend_color = 0;
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
            blend_color = target.pixels[(top_center_y - y) * target.width + left_center_x - x];
          }
          inside_shape = true;
        }
        // Draw the corner points in all four quadrants if inside the shape
        if (inside_shape == true) {
          f32 random_variation = get_blue_noise_value(x, y) * dither_spread;

          // Calculate gradient position for left corners
          percent = (corner_radius - x - 1) * one_percent_width;
          color = get_dithered_gradient_color(gradient, percent, random_variation);
          color = set_alpha(color, (u8)(opacity * 255));
          if (blend_color != 0) {
            color = blend_colors(color, blend_color);
          }

          // Top left quadrant
          target.pixels[(top_center_y - y) * target.width + left_center_x - x] = color;
          // Bottom left quadrant
          target.pixels[(bottom_center_y + y) * target.width + left_center_x - x] = color;

          // Calculate gradient position for right corners
          percent = (rectangle.w - corner_radius + x) * one_percent_width;
          color = get_dithered_gradient_color(gradient, percent, random_variation);
          color = set_alpha(color, (u8)(opacity * 255));
          if (blend_color != 0) {
            color = blend_colors(color, blend_color);
          }

          // Top right quadrant
          target.pixels[(top_center_y - y) * target.width + right_center_x + x] = color;
          // Bottom right quadrant
          target.pixels[(bottom_center_y + y) * target.width + right_center_x + x] = color;
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
      target.pixels[(rectangle.y + y) * target.width + render_x] = color;
    }
  }
}

void draw_vertical_gradient_rectangle(PixelData target, SDL_Rect rectangle, i32 corner_radius, C9_Gradient gradient) {
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
        target.pixels[render_y * target.width + rectangle.x + x] = color;
      }
    }

    // Fill the area between the bottom corners
    for (i32 y = rectangle.h - corner_radius; y < rectangle.h; y++) {
      f32 percent = y * one_percent_height;
      i32 render_y = rectangle.y + y;
      for (i32 x = corner_radius; x < rectangle.w - corner_radius; x++) {
        f32 random_variation = get_blue_noise_value(x, y) * dither_spread;
        RGBA color = get_dithered_gradient_color(gradient, percent, random_variation);
        target.pixels[render_y * target.width + rectangle.x + x] = color;
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
    RGBA color = 0; // Current drawing color
    RGBA blend_color = 0; // Color to blend with

    // Calculate points for one quadrant and mirror it the other quadrants
    for (i32 x = 0; x < corner_radius; x++) {
      for (i32 y = 0; y < corner_radius; y++) {
        f32 distance_squared = pow(x, 4) + pow(y, 4);
        blend_color = 0;
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
            blend_color = target.pixels[(top_center_y - y) * target.width + left_center_x - x];
          }
          inside_shape = true;
        }
        // Draw the corner points in all four quadrants if inside the shape
        if (inside_shape == true) {
          f32 random_variation = get_blue_noise_value(x, y) * dither_spread;

          // Calculate gradient position for top corners
          percent = (corner_radius - y - 1) * one_percent_height;
          color = get_dithered_gradient_color(gradient, percent, random_variation);
          color = set_alpha(color, (u8)(opacity * 255));
          if (blend_color != 0) {
            color = blend_colors(color, blend_color);
          }

          // Top left quadrant
          target.pixels[(top_center_y - y) * target.width + left_center_x - x] = color;
          // Top right quadrant
          target.pixels[(top_center_y - y) * target.width + right_center_x + x] = color;

          // Calculate gradient position for bottom corners
          percent = (rectangle.h - corner_radius + y) * one_percent_height;
          color = get_dithered_gradient_color(gradient, percent, random_variation);
          color = set_alpha(color, (u8)(opacity * 255));
          if (blend_color != 0) {
            color = blend_colors(color, blend_color);
          }

          // Bottom right quadrant
          target.pixels[(bottom_center_y + y) * target.width + right_center_x + x] = color;
          // Bottom left quadrant
          target.pixels[(bottom_center_y + y) * target.width + left_center_x - x] = color;
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
      target.pixels[render_y * target.width + rectangle.x + x] = color;
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

void draw_rectangle_with_border(PixelData target, SDL_Rect rectangle, i32 corner_radius, BorderSize border, RGBA border_color, RGBA background_color) {
  draw_filled_rectangle(target, rectangle, corner_radius, border_color);
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
  draw_filled_rectangle(target, inner_rectangle, inner_corner_radius, background_color);
}

void draw_horizontal_gradient_rectangle_with_border(PixelData target, SDL_Rect rectangle, i32 corner_radius, BorderSize border, RGBA border_color, C9_Gradient background_gradient) {
  draw_filled_rectangle(target, rectangle, corner_radius, border_color);
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
  draw_horizontal_gradient_rectangle(target, inner_rectangle, inner_corner_radius, background_gradient);
}

void draw_vertical_gradient_rectangle_with_border(PixelData target, SDL_Rect rectangle, i32 corner_radius, BorderSize border, RGBA border_color, C9_Gradient background_gradient) {
  draw_filled_rectangle(target, rectangle, corner_radius, border_color);
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
  draw_vertical_gradient_rectangle(target, inner_rectangle, inner_corner_radius, background_gradient);
}

#define C9_DRAW_SHAPES
#endif