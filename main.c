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
  i32 window_width = 640;
  i32 window_height = 640;

  RGBA white = 0xFFFFFFFF;
  RGBA white_2 = 0xF8F8F8FF;
  RGBA gray_1 = 0xF8F9FAFF;
  RGBA gray_2 = 0xF2F3F4FF;
  RGBA border_color = 0xDEE2E6FF;

  C9_Gradient white_shade = {
    .start_color = white,
    .end_color = white_2,
    .start_at = 0.95,
    .end_at = 1
  };

  C9_Gradient gray_1_shade = {
    .start_color = gray_1,
    .end_color = gray_2,
    .start_at = 0.95,
    .end_at = 1
  };

  // Initialize SDL
  if (SDL_Init(SDL_INIT_VIDEO)) {
    printf("SDL_Init: %s\n", SDL_GetError());
    return -1;
  }

  // Create SDL window
  SDL_Window *window = SDL_CreateWindow(
    "Window Title", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
    window_width, window_height, SDL_WINDOW_RESIZABLE
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
  // Root element
  ElementTree *element_tree = new_element_tree(element_arena);
  element_tree->root->layout_direction = layout_direction.vertical;

  // Top panel
  Element *top_panel = add_new_element(element_arena, element_tree->root);
  top_panel->height = 50;

  // Bottom panel
  Element *bottom_panel = add_new_element(element_arena, element_tree->root);

  // Logo panel
  Element *top_left_panel = add_new_element(element_arena, top_panel);
  *top_left_panel = (Element){
    .width = 200,
    .background_type = background_type.horizontal_gradient,
    .background_gradient = white_shade,
    .border_color = border_color,
    .border = (Border){0, 1, 1, 0},
  };

  Element *top_right_panel = add_new_element(element_arena, top_panel);
  *top_right_panel = (Element){
    .background_type = background_type.color,
    .background_color = white,
    .padding = (Padding){10, 10, 10, 10},
    .border_color = border_color,
    .border = (Border){0, 0, 1, 0},
  };

  // Side panel
  Element *side_panel = add_new_element(element_arena, bottom_panel);
  *side_panel = (Element){
    .width = 200,
    .background_type = background_type.horizontal_gradient,
    .background_gradient = gray_1_shade,
    .padding = (Padding){10, 10, 10, 10},
    .gutter = 10,
    .border_color = border_color,
    .layout_direction = layout_direction.vertical,
    .border = (Border){0, 1, 0, 0},
  };

  // Content pane
  Element *content_panel = add_new_element(element_arena, bottom_panel);
  *content_panel = (Element){
    .background_type = background_type.color,
    .background_color = white,
    .padding = (Padding){10, 10, 10, 10},
  };

  // Menu element
  Element *menu_item = add_new_element(element_arena, side_panel);
  *menu_item = (Element){
    .height = 30,
    .background_type = background_type.color,
    .background_color = border_color,
    .padding = (Padding){10, 10, 10, 10},
    .border_radius = 15,
  };

  // Menu element 2
  Element *menu_item_2 = add_new_element(element_arena, side_panel);
  *menu_item_2 = (Element){
    .height = 30,
    .background_type = background_type.color,
    .background_color = border_color,
    .padding = (Padding){10, 10, 10, 10},
    .border_radius = 15,
  };

  // Search bar
  Element *search_bar = add_new_element(element_arena, top_right_panel);
  *search_bar = (Element){
    .height = 30,
    .background_type = background_type.color,
    .background_color = white,
    .border_radius = 15,
    .border_color = border_color,
    .border = (Border){1, 1, 1, 1},
  };

  i32 min_width = get_min_width(element_tree->root);
  printf("Min width: %d\n", min_width);
  i32 min_height = get_min_height(element_tree->root);
  printf("Min height: %d\n", min_height);
  set_dimensions(element_tree, window_width, window_height);

  if (element_tree->root->children == 0) {
    printf("No children\n");
  }

  // Begin main loop
  bool done = false;
  bool redraw = true;
  while (!done) {
    // Wait for events before rendering
    if (SDL_WaitEvent(&event)) {
      if (event.type == SDL_WINDOWEVENT) {
        if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
          i32 width = event.window.data1;
          i32 height = event.window.data2;
          if (width < min_width) {
            width = min_width;
          }
          if (height < min_height) {
            height = min_height;
          }

          SDL_SetWindowSize(window, width, height);
          set_dimensions(element_tree, width, height);
          redraw = true;
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

      render_element_tree(renderer, element_tree);
      draw_text(renderer, Inter, "Hello, World!", 20, 65);
      draw_text(renderer, Inter, "Search", 220, 15);
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
