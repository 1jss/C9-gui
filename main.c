#include <SDL2/SDL.h> // SDL_CreateWindow, SDL_DestroyWindow, SDL_CreateRenderer, SDL_DestroyRenderer, SDL_RenderPresent, SDL_Delay
#include <stdbool.h> // bool
#include <stdio.h> // printf
#include <time.h> // clock
#include "components/home.h" // home_element, create_home_element
#include "components/menu.h" // add_menu_items
#include "components/search_bar.h" // search_bar, create_search_bar_element
#include "constants/color_theme.h" // white, white_2, gray_1, gray_2, border_color, text_color
#include "constants/element_tags.h" // content_panel_tag, search_panel_tag, side_panel_tag
#include "include/arena.h" // Arena, arena_open, arena_close
#include "include/color.h" // RGBA, C9_Gradient
#include "include/element_tree.h" // Element, ElementTree, new_element_tree, add_new_element, layout_direction, background_type, Border, Padding
#include "include/event.h" // click_handler, blur_handler, input_handler, handle_events
#include "include/font.h" // init_font, close_font
#include "include/layout.h" // set_dimensions
#include "include/renderer.h" // render_element_tree, render_selected_element
#include "include/types.h" // i32

i32 main() {
  i32 window_width = 640;
  i32 window_height = 640;

  // Make sure we get multitouch events for scrolling
  SDL_SetHint(SDL_HINT_MOUSE_TOUCH_EVENTS, "1");

  // Initialize SDL
  if (SDL_Init(SDL_INIT_VIDEO)) {
    printf("SDL_Init: %s\n", SDL_GetError());
    return -1;
  }
  SDL_StartTextInput();

  // Create SDL window
  SDL_Window *window = SDL_CreateWindow(
    "Window Title", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
    window_width, window_height, SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN
  );
  if (!window) {
    printf("SDL_CreateWindow: %s\n", SDL_GetError());
    return -1;
  }

  // Create SDL renderer
  SDL_Renderer *renderer =
    SDL_CreateRenderer(window, -1, 0);
  if (!renderer) {
    printf("SDL_CreateRenderer: %s\n", SDL_GetError());
    return -1;
  }
  SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
  SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");

  // Set a background while loading
  SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
  SDL_RenderClear(renderer);
  SDL_Rect logo_rect = {
    .x = window_width / 2 - 180,
    .y = window_height / 2 - 155,
    .w = 360,
    .h = 310
  };
  renderer_draw_image(renderer, "C9_segment_large.png", logo_rect);
  SDL_RenderPresent(renderer);

  // Initialize font
  if (init_font() == status.ERROR) return -1;

  Arena *element_arena = arena_open(4096);
  // Root element
  ElementTree *tree = new_element_tree(element_arena);
  tree->root->layout_direction = layout_direction.vertical;

  // The target texture is used as a back buffer that persists between frames. This lets us rerender only the parts of the screen that have changed.
  tree->target_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, window_width, window_height);
  SDL_SetTextureBlendMode(tree->target_texture, SDL_BLENDMODE_BLEND);
  SDL_SetRenderTarget(renderer, tree->target_texture);

  // Top panel
  Element *top_panel = add_new_element(tree->arena, tree->root);
  top_panel->height = 50;

  // Bottom panel
  Element *bottom_panel = add_new_element(tree->arena, tree->root);

  Element *top_left_panel = add_new_element(tree->arena, top_panel);
  *top_left_panel = (Element){
    .width = 200,
    .background_type = background_type.horizontal_gradient,
    .background.gradient = white_shade,
    .border_color = border_color,
    .border = (Border){0, 1, 1, 0},
    .padding = (Padding){9, 9, 9, 9},
  };

  Element *top_left_logo = add_new_element(tree->arena, top_left_panel);
  *top_left_logo = (Element){
    .background_type = background_type.image,
    .background.image = to_s8("C9_segment_small.png")
  };

  Element *top_right_panel = add_new_element(tree->arena, top_panel);
  *top_right_panel = (Element){
    .element_tag = search_panel_tag,
    .background_type = background_type.color,
    .background.color = white,
    .padding = (Padding){10, 10, 10, 10},
    .border_color = border_color,
    .border = (Border){0, 0, 1, 0},
  };

  Element *side_panel = add_new_element(tree->arena, bottom_panel);
  *side_panel = (Element){
    .element_tag = side_panel_tag,
    .width = 200,
    .background_type = background_type.horizontal_gradient,
    .background.gradient = gray_1_shade,
    .padding = (Padding){10, 10, 10, 10},
    .gutter = 10,
    .border_color = border_color,
    .layout_direction = layout_direction.vertical,
    .border = (Border){0, 1, 0, 0},
    .overflow = overflow_type.scroll_y,
  };

  Element *content_panel = add_new_element(tree->arena, bottom_panel);
  *content_panel = (Element){
    .element_tag = content_panel_tag,
    .background_type = background_type.color,
    .background.color = white,
  };

  // Fill content panel with home element
  create_home_element(tree->arena);
  add_element(tree->arena, content_panel, home_element);

  // Fill side panel with menu items
  add_menu_items(tree->arena, side_panel);

  // search_bar is defined in search_bar.h
  create_search_bar_element(tree->arena);
  add_element(tree->arena, top_right_panel, search_bar);

  // apply element layout
  set_dimensions(tree, window_width, window_height);

  if (tree->root->children == 0) {
    printf("No children\n");
  }

  SDL_RaiseWindow(window);

  // Begin main loop
  bool main_loop = true;
  while (main_loop) {
    clock_t main_loop_start = clock();
    main_loop = handle_events(tree, window, renderer);

    if (tree->rerender == rerender_type.all) {
      SDL_SetRenderTarget(renderer, tree->target_texture);
      render_element_tree(renderer, tree);
      // Draw target_texture to back buffer and present
      SDL_SetRenderTarget(renderer, NULL);
      SDL_RenderCopy(renderer, tree->target_texture, NULL, NULL);
      SDL_RenderPresent(renderer);
      tree->rerender = rerender_type.none;
      tree->rerender_element = 0;
    } else if (tree->rerender == rerender_type.selected && tree->rerender_element != 0) {
      SDL_SetRenderTarget(renderer, tree->target_texture);
      render_selected_element(renderer, tree);
      // Draw target_texture to back buffer and present
      SDL_SetRenderTarget(renderer, NULL);
      SDL_RenderCopy(renderer, tree->target_texture, NULL, NULL);
      SDL_RenderPresent(renderer);
      tree->rerender = rerender_type.none;
      tree->rerender_element = 0;
    }

    clock_t main_loop_end = clock();
    f64 main_loop_cycle = (f64)(main_loop_end - main_loop_start) / CLOCKS_PER_SEC;
    if (main_loop_cycle < 0.016) {
      SDL_Delay((0.016 - main_loop_cycle) * 1000);
    }
  }; // End of rendering loop

  if (tree->target_texture) {
    SDL_DestroyTexture(tree->target_texture);
  }
  if (renderer) {
    SDL_DestroyRenderer(renderer);
  }
  if (window) {
    SDL_DestroyWindow(window);
  }
  SDL_StopTextInput();
  // printf("Size of Element: %zu\n", sizeof(Element));
  // printf("Size of element_arena %d\n", arena_size(element_arena));
  free_textures(tree->root);
  arena_close(element_arena);
  close_font();
  SDL_Quit();

  return 0;
}
