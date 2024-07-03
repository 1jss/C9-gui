#include <SDL2/SDL.h>
#include <dirent.h>
#include <stdbool.h> // bool
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "include/SDL_ttf.h"
#include "include/arena.h"
#include "include/array.h"
#include "include/color.h"
#include "include/draw_shapes.h" // draw_filled_rectangle, draw_horizontal_gradient, draw_rectangle_with_border, draw_filled_rounded_rectangle, draw_superellipse, draw_filled_superellipse
#include "include/layout.h"
#include "include/renderer.h"
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

  Arena *element_arena = arena_open(1024);
  ElementTree *element_tree = new_element_tree(element_arena);
  element_tree->root->width = 640;
  element_tree->root->height = 640;
  element_tree->root->padding = (PaddingProps){10, 10, 10, 10};
  Element test_element = new_element();
  test_element = (Element){
    .element_sizing = element_sizing.fixed,
    .width = 100,
    .height = 100,
  };
  Element *child = add_child_element(element_tree, element_tree->root, test_element);
  printf("Child width: %d\n", child->width);
  set_dimensions(element_tree);
  if (element_tree->root->children == 0) {
    printf("No children\n");
  }
  // printf("Number of children: %zu\n", array_length(element_tree->root->children));

  // Begin main loop
  bool done = false;
  bool redraw = true;
  while (!done) {
    // Wait for events before rendering
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
        printf("Mouse wheel\n");
        // scroll up or down
        if (event.wheel.y != 0) {
          scroll_y += event.wheel.y;
        }
        SDL_FlushEvent(SDL_MOUSEWHEEL);
      } else if (event.type == SDL_MOUSEBUTTONDOWN) {
        printf("Mouse button down\n");
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

      RGBA white = 0xFFFFFFFF;
      RGBA white_2 = 0xF8F8F8FF;
      RGBA gray_1 = 0xF8F9FAFF;
      RGBA gray_2 = 0xF2F3F4FF;
      RGBA border_color = 0xDEE2E6FF;

      C9_Gradient white_shade = {
        .start_color = white,
        .end_color = white_2
      };
      C9_Gradient gray_1_shade = {
        .start_color = gray_1,
        .end_color = gray_2
      };

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

      RGBA blue = 0x0000FFFF;
      // Draw border of squircle
      draw_superellipse(renderer, 25, 25, 15, blue);
      // Draw filled squircle
      draw_filled_superellipse(renderer, 65, 25, 15, blue);

      // Draw a dot that follows the mouse
      SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
      SDL_RenderDrawPoint(renderer, mouse_x, mouse_y);

      render_element_tree(renderer, element_tree);
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

  arena_close(element_arena);
  TTF_CloseFont(Inter);
  TTF_Quit();
  SDL_Quit();

  return 0;
}
