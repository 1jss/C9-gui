#include <SDL2/SDL.h> // SDL_CreateWindow, SDL_DestroyWindow, SDL_CreateRenderer, SDL_DestroyRenderer, SDL_SetRenderDrawColor, SDL_RenderClear, SDL_RenderPresent, SDL_Delay, SDL_Event, SDL_WaitEvent, SDL_WINDOWEVENT, SDL_WINDOWEVENT_RESIZED, SDL_SetWindowSize, SDL_FlushEvent, SDL_MOUSEMOTION, SDL_MOUSEWHEEL, SDL_MOUSEBUTTONDOWN, SDL_QUIT
#include <stdbool.h> // bool
#include <stdio.h> // printf
#include "include/SDL_ttf.h" // TTF_Init, TTF_OpenFont, TTF_RenderText_Blended, TTF_CloseFont
#include "include/arena.h" // Arena, arena_open, arena_close
#include "include/color.h" // RGBA, C9_Gradient
#include "include/layout.h" // Element, ElementTree, new_element_tree, add_new_element, get_min_width, get_min_height, set_dimensions, layout_direction, background_type, background_gradient, Border, Padding
#include "include/renderer.h" // render_element_tree
#include "include/types.h" // i32

#if 0
i32 resizeWatcher(void *data, SDL_Event *event) {
  if (event->type == SDL_WINDOWEVENT &&
      event->window.event == SDL_WINDOWEVENT_RESIZED) {
    SDL_Window *window = SDL_GetWindowFromID(event->window.windowID);
    if (window == (SDL_Window *)data) {
      i32 width;
      i32 height;
      SDL_GetWindowSize(window, &width, &height);

      printf("width: %d, height: %d\n", width, height);
    }
  }
  return 0;
}
#endif

void reset_menu_elements(Element *side_panel) {
  // Loop through all children and set background color to none
  Array *children = side_panel->children;
  if (children == 0) {
    return;
  }
  for (size_t i = 0; i < array_length(children); i++) {
    Element *child = array_get(children, i);
    child->background_type = background_type.none;
  }
}

void set_active_menu_element(Element *element) {
  element->background_type = background_type.color;
}

void set_menu(ElementTree *tree, Element *side_panel){
Element *active_element = tree->active_element;
  reset_menu_elements(side_panel);
  set_active_menu_element(active_element);
  tree->rerender = rerender_type.selected;
  tree->rerender_element = side_panel;
}

void click_item_1(ElementTree *tree, void *data) {
  set_menu(tree, (Element *)data);
  printf("Clicked menu item 1\n");
}

void click_item_2(ElementTree *tree, void *data) {
  set_menu(tree, (Element *)data);
  printf("Clicked menu item 2\n");
}

void click_item_3(ElementTree *tree, void *data) {
  set_menu(tree, (Element *)data);
  printf("Clicked menu item 3\n");
}

void click_serach_bar(ElementTree *tree, void *data) {
  printf("Clicked search bar\n");
}

i32 main() {
  // i32 mouse_x = 0;
  // i32 mouse_y = 0;
  i32 scroll_y = 0;
  i32 window_width = 640;
  i32 window_height = 640;

  RGBA white = 0xFFFFFFFF;
  RGBA white_2 = 0xF8F8F8FF;
  RGBA gray_1 = 0xF8F9FAFF;
  RGBA gray_2 = 0xF2F3F4FF;
  RGBA border_color = 0xDEE2E6FF;
  RGBA text_color = 0x555555FF;

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

  // SDL_AddEventWatch(resizeWatcher, window);

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

  Arena *element_arena = arena_open(4096);
  // Root element
  ElementTree *tree = new_element_tree(element_arena);
  tree->root->layout_direction = layout_direction.vertical;

  // Top panel
  Element *top_panel = add_new_element(tree, tree->root);
  top_panel->height = 50;

  // Bottom panel
  Element *bottom_panel = add_new_element(tree, tree->root);

  Element *top_left_panel = add_new_element(tree, top_panel);
  *top_left_panel = (Element){
    .width = 200,
    .background_type = background_type.horizontal_gradient,
    .background_gradient = white_shade,
    .border_color = border_color,
    .border = (Border){0, 1, 1, 0},
  };

  Element *top_right_panel = add_new_element(tree, top_panel);
  *top_right_panel = (Element){
    .background_type = background_type.color,
    .background_color = white,
    .padding = (Padding){10, 10, 10, 10},
    .border_color = border_color,
    .border = (Border){0, 0, 1, 0},
  };

  Element *side_panel = add_new_element(tree, bottom_panel);
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

  Element *content_panel = add_new_element(tree, bottom_panel);
  *content_panel = (Element){
    .background_type = background_type.color,
    .background_color = white,
    .padding = (Padding){10, 10, 10, 10},
  };

  Element *menu_item = add_new_element(tree, side_panel);
  *menu_item = (Element){
    .element_group = 1,
    .height = 30,
    .background_type = background_type.none,
    .background_color = border_color,
    .padding = (Padding){5, 10, 5, 10},
    .border_radius = 15,
    .text = to_s8("Menu item 1"),
    .text_color = text_color,
    .on_click = &click_item_1,
  };

  Element *menu_item_2 = add_new_element(tree, side_panel);
  *menu_item_2 = (Element){
    .element_group = 1,
    .height = 30,
    .background_type = background_type.none,
    .background_color = border_color,
    .padding = (Padding){5, 10, 5, 10},
    .border_radius = 15,
    .text = to_s8("Menu item 2"),
    .text_color = text_color,
    .on_click = &click_item_2,
  };

  Element *menu_item_3 = add_new_element(tree, side_panel);
  *menu_item_3 = (Element){
    .element_group = 1,
    .height = 30,
    .background_type = background_type.none,
    .background_color = border_color,
    .padding = (Padding){5, 10, 5, 10},
    .border_radius = 15,
    .text = to_s8("Menu item 3"),
    .text_color = text_color,
    .on_click = &click_item_3,
  };

  Element *search_bar = add_new_element(tree, top_right_panel);
  *search_bar = (Element){
    .min_width = 100,
    .background_type = background_type.color,
    .background_color = white,
    .padding = (Padding){5, 10, 5, 10},
    .border_radius = 15,
    .border_color = border_color,
    .border = (Border){1, 1, 1, 1},
    .text = to_s8("Search"),
    .text_color = text_color,
    .on_click = &click_serach_bar,
  };

  i32 min_width = get_min_width(tree->root);
  i32 min_height = get_min_height(tree->root);
  set_dimensions(tree, window_width, window_height);

  if (tree->root->children == 0) {
    printf("No children\n");
  }

  // Begin main loop
  bool done = false;
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
          // Cap to min width and height
          SDL_SetWindowSize(window, width, height);
          set_dimensions(tree, width, height);
          tree->rerender = rerender_type.all;
        }
      } else if (event.type == SDL_KEYDOWN) {
        printf("Key press\n");
      } else if (event.type == SDL_MOUSEMOTION) {
        //mouse_x = event.motion.x;
        //mouse_y = event.motion.y;
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
        i32 mouse_down_x = event.button.x;
        i32 mouse_down_y = event.button.y;
        tree->active_element = get_element_at(tree->root, mouse_down_x, mouse_down_y);
        // If menu item is clicked, pass the side panel ref as data
        if (tree->active_element->element_group == 1) {
          click_handler(tree, side_panel);
        } else {
          click_handler(tree, 0);
        }
        SDL_FlushEvent(SDL_MOUSEBUTTONDOWN);
      } else if (event.type == SDL_QUIT) {
        done = true;
      }
    }

    if (tree->rerender == rerender_type.all || tree->rerender == rerender_type.selected) {
      printf("Rerender all\n");
      // Clear back buffer
      SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
      if (SDL_RenderClear(renderer)) {
        printf("SDL_RenderClear: %s\n", SDL_GetError());
        return -1;
      }

      render_element_tree(renderer, Inter, tree);
      // Draw back buffer to screen
      SDL_RenderPresent(renderer);
      tree->rerender = rerender_type.none;
    } 
    #if 0
    // Back rederer is not stable, so partial rendering can not be done like this:
    if (tree->rerender != rerender_type.selected && tree->rerender_element != 0) {
      printf("Rerender selected\n");
      draw_elements(renderer, Inter, tree->rerender_element);
      // Draw back buffer to screen
      SDL_RenderPresent(renderer);
      tree->rerender = rerender_type.none;
      tree->rerender_element = 0;
    }
    #endif

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
