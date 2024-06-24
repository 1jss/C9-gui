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
#include "include/string.h"
#include "include/types.h"

const SDL_Color WHITE = {255, 255, 255, SDL_ALPHA_OPAQUE};

void draw_text(SDL_Renderer *renderer, TTF_Font *font, char *text, i32 x, i32 y) {
  SDL_Surface *surface = TTF_RenderText_Blended(font, text, WHITE);
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

void draw_superellipse(SDL_Renderer *renderer, i32 center_x, i32 center_y, i32 radius) {
  SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);

  // Calculate radian step size so that every border pixel is drawn
  f32 step = 0.85 / radius;

  for (f32 t = 0; t < 2 * M_PI; t += step) {
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
      SDL_SetRenderDrawColor(renderer, 255, 255, 255, opacity);
      SDL_RenderDrawPoint(renderer, left_x, nearest_y);
      SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255 - opacity);
      SDL_RenderDrawPoint(renderer, right_x, nearest_y);
    }
    // Smudge downwards
    else if (t <= 3 * M_PI / 4) {
      f32 y_diff = float_y - top_y;
      u8 opacity = (1 - y_diff) * 255;
      SDL_SetRenderDrawColor(renderer, 255, 255, 255, opacity);
      SDL_RenderDrawPoint(renderer, nearest_x, top_y);
      SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255 - opacity);
      SDL_RenderDrawPoint(renderer, nearest_x, bottom_y);
    }
    // Smudge leftwards
    else if (t <= 5 * M_PI / 4) {
      f32 x_diff = float_x - right_x;
      u8 opacity = (1 + x_diff) * 255;
      SDL_SetRenderDrawColor(renderer, 255, 255, 255, opacity);
      SDL_RenderDrawPoint(renderer, right_x, nearest_y);
      SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255 - opacity);
      SDL_RenderDrawPoint(renderer, left_x, nearest_y);
    }
    // Smudge upwards
    else {
      f32 y_diff = float_y - bottom_y;
      u8 opacity = (1 + y_diff) * 255;
      SDL_SetRenderDrawColor(renderer, 255, 255, 255, opacity);
      SDL_RenderDrawPoint(renderer, nearest_x, bottom_y);
      SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255 - opacity);
      SDL_RenderDrawPoint(renderer, nearest_x, top_y);
    }
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
        redraw = true;
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
      SDL_SetRenderDrawColor(renderer, 36, 36, 36, SDL_ALPHA_OPAQUE);
      if (SDL_RenderClear(renderer)) {
        printf("SDL_RenderClear: %s\n", SDL_GetError());
        return -1;
      }

      // Draw a squircle at mouse position
      draw_superellipse(renderer, 320, 320, 300);
      draw_superellipse(renderer, 320, 320, 200);
      draw_superellipse(renderer, 320, 320, 100);
      draw_superellipse(renderer, 320, 320, 50);

      // Draw text at mouse position
      SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
      draw_text(renderer, Inter, "Hello, World!", mouse_x, mouse_y);

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
