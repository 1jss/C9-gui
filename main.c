#include <SDL2/SDL.h>
#include <dirent.h>
#include <math.h>
#include <stdbool.h> // bool
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "include/SDL_image.h"
#include "include/SDL_ttf.h"
#include "include/arena.h"
#include "include/array.h"
#include "include/color.h"
#include "include/string.h"
#include "include/types.h"

void draw_text(SDL_Renderer *renderer, TTF_Font *font, char *text, i32 x, i32 y) {
  const SDL_Color dark_gray = {50, 50, 50, SDL_ALPHA_OPAQUE};
  SDL_Surface *surface = TTF_RenderText_Blended(font, text, dark_gray);
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

void draw_superellipse_border_point(SDL_Renderer *renderer, i32 center_x, i32 center_y, i32 radius, f32 t, C9_RGB color) {
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
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, opacity);
    SDL_RenderDrawPoint(renderer, left_x, nearest_y);
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 255 - opacity);
    SDL_RenderDrawPoint(renderer, right_x, nearest_y);
  }
  // Smudge downwards
  else if (t <= 3 * M_PI / 4) {
    f32 y_diff = float_y - top_y;
    u8 opacity = (1 - y_diff) * 255;
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, opacity);
    SDL_RenderDrawPoint(renderer, nearest_x, top_y);
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 255 - opacity);
    SDL_RenderDrawPoint(renderer, nearest_x, bottom_y);
  }
  // Smudge leftwards
  else if (t <= 5 * M_PI / 4) {
    f32 x_diff = float_x - right_x;
    u8 opacity = (1 + x_diff) * 255;
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, opacity);
    SDL_RenderDrawPoint(renderer, right_x, nearest_y);
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 255 - opacity);
    SDL_RenderDrawPoint(renderer, left_x, nearest_y);
  }
  // Smudge upwards
  else {
    f32 y_diff = float_y - bottom_y;
    u8 opacity = (1 + y_diff) * 255;
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, opacity);
    SDL_RenderDrawPoint(renderer, nearest_x, bottom_y);
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 255 - opacity);
    SDL_RenderDrawPoint(renderer, nearest_x, top_y);
  }
}

void draw_filled_superellipse_border_point(SDL_Renderer *renderer, i32 center_x, i32 center_y, i32 radius, f32 t, C9_RGB color) {
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
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, opacity);
    SDL_RenderDrawPoint(renderer, left_x, nearest_y);
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 255 - opacity);
    SDL_RenderDrawPoint(renderer, right_x, nearest_y);
    // Fill to the center
    for (i32 i = center_x; i <= left_x; i++) {
      SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 255);
      SDL_RenderDrawPoint(renderer, i, nearest_y);
    }
  }
  // Smudge downwards
  else if (t <= 3 * M_PI / 4) {
    f32 y_diff = float_y - top_y;
    u8 opacity = (1 - y_diff) * 255;
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, opacity);
    SDL_RenderDrawPoint(renderer, nearest_x, top_y);
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 255 - opacity);
    SDL_RenderDrawPoint(renderer, nearest_x, bottom_y);
    // Fill to the center
    for (i32 i = center_y; i <= top_y; i++) {
      SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 255);
      SDL_RenderDrawPoint(renderer, nearest_x, i);
    }
  }
  // Smudge leftwards
  else if (t <= 5 * M_PI / 4) {
    f32 x_diff = float_x - right_x;
    u8 opacity = (1 + x_diff) * 255;
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, opacity);
    SDL_RenderDrawPoint(renderer, right_x, nearest_y);
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 255 - opacity);
    SDL_RenderDrawPoint(renderer, left_x, nearest_y);
    // Fill to the center
    for (i32 i = center_x; i >= right_x; i--) {
      SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 255);
      SDL_RenderDrawPoint(renderer, i, nearest_y);
    }
  }
  // Smudge upwards
  else {
    f32 y_diff = float_y - bottom_y;
    u8 opacity = (1 + y_diff) * 255;
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, opacity);
    SDL_RenderDrawPoint(renderer, nearest_x, bottom_y);
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 255 - opacity);
    SDL_RenderDrawPoint(renderer, nearest_x, top_y);
    // Fill to the center
    for (i32 i = center_y; i >= bottom_y; i--) {
      SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 255);
      SDL_RenderDrawPoint(renderer, nearest_x, i);
    }
  }
}

// Draws a superellipse with a given center and radius
void draw_superellipse(SDL_Renderer *renderer, i32 center_x, i32 center_y, i32 radius, C9_RGB color) {
  SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, SDL_ALPHA_OPAQUE);
  // Calculate radian step size so that every border pixel is drawn
  f32 step = 0.85 / radius;
  for (f32 t = 0; t < 2 * M_PI; t += step) {
    draw_superellipse_border_point(renderer, center_x, center_y, radius, t, color);
  }
}

// Draws a filled superellipse with a given center and radius
void draw_filled_superellipse(SDL_Renderer *renderer, i32 center_x, i32 center_y, i32 radius, C9_RGB color) {
  SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, SDL_ALPHA_OPAQUE);
  // Calculate radian step size so that every border pixel is drawn
  f32 step = 0.85 / radius;
  for (f32 t = 0; t < 2 * M_PI; t += step) {
    draw_filled_superellipse_border_point(renderer, center_x, center_y, radius, t, color);
  }
}

// Draws a rectangle with superellipse corners
void draw_rounded_rectangle(SDL_Renderer *renderer, i32 x, i32 y, i32 rectangle_width, i32 rectangle_height, i32 corner_radius, C9_RGB color) {
  // Cap corner radius to half of the rectangle width or height
  if (2 * corner_radius < rectangle_width || 2 * corner_radius < rectangle_height) {
    corner_radius = rectangle_width < rectangle_height ? rectangle_width / 2 : rectangle_height / 2;
  }
  SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, SDL_ALPHA_OPAQUE);
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
void draw_filled_rounded_rectangle(SDL_Renderer *renderer, i32 x, i32 y, i32 rectangle_width, i32 rectangle_height, i32 corner_radius, C9_RGB color) {
  // Smallest acceptable corner radius is 10px
  if (corner_radius < 10) {
    corner_radius = 10;
  }
  // Cap corner radius to half of the rectangle width or height
  if (2 * corner_radius >= rectangle_width || 2 * corner_radius >= rectangle_height) {
    corner_radius = rectangle_width < rectangle_height ? rectangle_width / 2 : rectangle_height / 2;
  }
  SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, SDL_ALPHA_OPAQUE);
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

void draw_rectangle_with_border(SDL_Renderer *renderer, i32 x, i32 y, i32 width, i32 height, i32 border_radius, i32 border_width, C9_RGB content_color, C9_RGB border_color) {
  draw_filled_rounded_rectangle(renderer, x, y, width, height, border_radius, content_color);
  draw_filled_rounded_rectangle(renderer, x + border_width, y + border_width, width - 2 * border_width, height - 2 * border_width, border_radius - border_width, border_color);
}

void draw_filled_rectangle(SDL_Renderer *renderer, i32 x, i32 y, i32 width, i32 height, C9_RGB color) {
  SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, SDL_ALPHA_OPAQUE);
  SDL_Rect rect = {x, y, width, height};
  SDL_RenderFillRect(renderer, &rect);
}

void draw_horizontal_gradient(SDL_Renderer *renderer, i32 x, i32 y, i32 width, i32 height, C9_Gradient gradient) {
  for (i32 i = 0; i < width; i++) {
    f32 t = 1.0 * i / width;
    C9_RGB color = getGradientColor(gradient, t);
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, SDL_ALPHA_OPAQUE);
    SDL_RenderDrawLine(renderer, x + i, y, x + i, y + height);
  }
}

void draw_vertical_gradient(SDL_Renderer *renderer, i32 x, i32 y, i32 width, i32 height, C9_Gradient gradient) {
  for (i32 i = 0; i < height; i++) {
    f32 t = 1.0 * i / height;
    C9_RGB color = {
      gradient.start.r + t * (gradient.end.r - gradient.start.r),
      gradient.start.g + t * (gradient.end.g - gradient.start.g),
      gradient.start.b + t * (gradient.end.b - gradient.start.b)
    };
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, SDL_ALPHA_OPAQUE);
    SDL_RenderDrawLine(renderer, x, y + i, x + width, y + i);
  }
}

i32 main() {
  i32 mouse_x = 0;
  i32 mouse_y = 0;
  i32 scroll_y = 0;

  // Initialize SDL
  if (SDL_Init(SDL_INIT_VIDEO)) {
    printf("SDL_Init: %s\n", SDL_GetError());
    return -1;
  }

  // Create SDL window
  SDL_Window *window = SDL_CreateWindow(
    "Window Title", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
    640, 640, 0
  );
  if (!window) {
    printf("SDL_CreateWindow: %s\n", SDL_GetError());
    return -1;
  }

  // Create SDL renderer
  SDL_Renderer *renderer =
    SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC);
  if (!renderer) {
    printf("SDL_CreateRenderer: %s\n", SDL_GetError());
    return -1;
  }
  SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
  SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");

  // Initialize font
  if (TTF_Init()) {
    printf("TTF_Init\n");
    return -1;
  }
  TTF_Font *Inter = TTF_OpenFont("Inter-Regular.ttf", 16);
  SDL_Event event;

  // Begin main loop
  bool done = false;
  bool redraw = true;
  while (!done) {
    // Check for events
    if (SDL_WaitEvent(&event)) {
      if (event.type == SDL_WINDOWEVENT) {
        if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
          printf("Window resized\n");
        }
      } else if (event.type == SDL_KEYDOWN) {
        printf("Key press\n");
      } else if (event.type == SDL_MOUSEMOTION) {
        mouse_x = event.motion.x;
        mouse_y = event.motion.y;
        //redraw = true;
        // printf("Mouse motion %d, %d\n", mouse_x, mouse_y);
        // Flush event queue to only use one event
        // Otherwise renderer laggs behind while emptying event queue
        SDL_FlushEvent(SDL_MOUSEMOTION);
      } else if (event.type == SDL_MOUSEWHEEL) {
        // scroll up or down
        if (event.wheel.y != 0) {
          scroll_y += event.wheel.y;
        }
        SDL_FlushEvent(SDL_MOUSEWHEEL);
      } else if (event.type == SDL_MOUSEBUTTONDOWN) {
        SDL_FlushEvent(SDL_MOUSEBUTTONDOWN);
      } else if (event.type == SDL_QUIT) {
        done = true;
      }
    }

    if (redraw) {
      // Clear back buffer
      SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
      if (SDL_RenderClear(renderer)) {
        printf("SDL_RenderClear: %s\n", SDL_GetError());
        return -1;
      }

      C9_RGB white = {255, 255, 255};
      C9_RGB white_2 = {248, 248, 248};
      C9_Gradient white_shade = {white, white_2};

      C9_RGB gray_1 = {248, 249, 250};
      C9_RGB gray_2 = {242, 243, 244};

      C9_Gradient gray_1_shade = {gray_1, gray_2};
      C9_RGB border_color = {222, 226, 230};

      // Pathbar primary with alpha 0.05

      // Top pane backgrounds
      draw_filled_rectangle(renderer, 0, 0, 640, 50, white);
      draw_horizontal_gradient(renderer, 190, 0, 10, 50, white_shade);
      // draw_vertical_gradient(renderer, 0, 40, 640, 10, white_shade);

      // Search bar
      draw_rectangle_with_border(renderer, 210, 10, 420, 30, 15, 1, border_color, white);
      draw_text(renderer, Inter, "Search", 220, 15);

      // Side pane backgrounds
      draw_filled_rectangle(renderer, 0, 50, 200, 590, gray_1);
      draw_horizontal_gradient(renderer, 190, 50, 10, 590, gray_1_shade);

      // Borders
      draw_filled_rectangle(renderer, 0, 49, 640, 1, border_color);
      draw_filled_rectangle(renderer, 199, 0, 1, 640, border_color);

      // Menu item
      draw_filled_rounded_rectangle(renderer, 10, 60, 180, 30, 15, border_color);
      draw_text(renderer, Inter, "Hello, World!", 20, 65);

      C9_RGB blue = {100, 100, 255};
      // Draw border of squircle
      draw_superellipse(renderer, 25, 25, 15, blue);
      // Draw filled squircle
      draw_filled_superellipse(renderer, 65, 25, 15, blue);

      // Draw back buffer to screen
      SDL_RenderPresent(renderer);
      redraw = false;
    }

    // Throttle frame rate to 60fps
    SDL_Delay(1000 / 60);
  }; // End of rendering loop

  if (renderer) {
    SDL_DestroyRenderer(renderer);
  }
  if (window) {
    SDL_DestroyWindow(window);
  }

  TTF_CloseFont(Inter);
  TTF_Quit();
  IMG_Quit();
  SDL_Quit();

  return 0;
}
