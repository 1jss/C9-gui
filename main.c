#include <SDL2/SDL.h> // SDL_CreateWindow, SDL_DestroyWindow, SDL_CreateRenderer, SDL_DestroyRenderer, SDL_RenderPresent, SDL_Delay, SDL_Event, SDL_WaitEvent, SDL_WINDOWEVENT, SDL_WINDOWEVENT_RESIZED, SDL_SetWindowSize, SDL_FlushEvent, SDL_MOUSEWHEEL, SDL_MOUSEBUTTONDOWN, SDL_QUIT
#include <stdbool.h> // bool
#include <stdio.h> // printf
#include "components/home.h" // home_element, create_home_element
#include "components/menu.h" // add_menu_items
#include "components/search_bar.h" // search_bar, create_search_bar_element
#include "constants/color_theme.h" // white, white_2, gray_1, gray_2, border_color, text_color
#include "constants/element_tags.h" // content_panel_tag, search_panel_tag, side_panel_tag
#include "include/arena.h" // Arena, arena_open, arena_close
#include "include/color.h" // RGBA, C9_Gradient
#include "include/element_tree.h" // Element, ElementTree, new_element_tree, add_new_element, layout_direction, background_type, background_gradient, Border, Padding
#include "include/event.h" // click_handler, blur_handler, input_handler
#include "include/font.h" // init_font, close_font
#include "include/layout.h" //  get_min_width, get_min_height, set_dimensions, get_clickable_element_at, scroll_x, scroll_y
#include "include/renderer.h" // render_element_tree
#include "include/types.h" // i32

i32 main() {
  i32 window_width = 640;
  i32 window_height = 640;
  i32 scroll_speed = 10;

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

  // Initialize font
  if (init_font() < 0) return -1;

  // The target texture is used as a back buffer that persists between frames. This lets us rerender only the parts of the screen that have changed.
  SDL_Texture *target_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, window_width, window_height);
  SDL_SetTextureBlendMode(target_texture, SDL_BLENDMODE_BLEND);
  SDL_SetRenderTarget(renderer, target_texture);

  Arena *element_arena = arena_open(4096);
  // Root element
  ElementTree *tree = new_element_tree(element_arena);
  tree->root->layout_direction = layout_direction.vertical;

  // Top panel
  Element *top_panel = add_new_element(tree->arena, tree->root);
  top_panel->height = 50;

  // Bottom panel
  Element *bottom_panel = add_new_element(tree->arena, tree->root);

  Element *top_left_panel = add_new_element(tree->arena, top_panel);
  *top_left_panel = (Element){
    .width = 200,
    .background_type = background_type.horizontal_gradient,
    .background_gradient = white_shade,
    .border_color = border_color,
    .border = (Border){0, 1, 1, 0},
  };

  Element *top_right_panel = add_new_element(tree->arena, top_panel);
  *top_right_panel = (Element){
    .element_tag = search_panel_tag,
    .background_type = background_type.color,
    .background_color = white,
    .padding = (Padding){10, 10, 10, 10},
    .border_color = border_color,
    .border = (Border){0, 0, 1, 0},
  };

  Element *side_panel = add_new_element(tree->arena, bottom_panel);
  *side_panel = (Element){
    .element_tag = side_panel_tag,
    .width = 200,
    .background_type = background_type.horizontal_gradient,
    .background_gradient = gray_1_shade,
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
    .background_color = white,
  };

  // Fill content panel with home element
  create_home_element(tree->arena);
  add_element(tree->arena, content_panel, home_element);

  // Fill side panel with menu items
  add_menu_items(tree->arena, side_panel);

  // search_bar is defined in search_bar.h
  create_search_bar_element(tree->arena);
  add_element(tree->arena, top_right_panel, search_bar);

  i32 min_width = get_min_width(tree->root);
  i32 min_height = get_min_height(tree->root);
  set_dimensions(tree, window_width, window_height);

  if (tree->root->children == 0) {
    printf("No children\n");
  }

  SDL_Event event;
  SDL_RaiseWindow(window);

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
          // Recreate the target-texture with the new dimensions
          if (target_texture) {
            SDL_DestroyTexture(target_texture);
            target_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, width, height);
            if (!target_texture) {
              printf("SDL_CreateTexture failed: %s\n", SDL_GetError());
            }
          }
          set_dimensions(tree, width, height);
          tree->rerender = rerender_type.all;
        }
      } else if (event.type == SDL_KEYDOWN) {
        SDL_Keymod mod = SDL_GetModState();
        // Get key press content
        SDL_Keysym keysym = event.key.keysym;
        if (keysym.sym == SDLK_BACKSPACE) {
          input_handler(tree, "BACKSPACE");
        } else if (keysym.sym == SDLK_LEFT &&
                   mod & KMOD_SHIFT) {
          input_handler(tree, "SELECT_LEFT");
        } else if (keysym.sym == SDLK_RIGHT &&
                   mod & KMOD_SHIFT) {
          input_handler(tree, "SELECT_RIGHT");
        } else if (keysym.sym == SDLK_DOWN &&
                   mod & KMOD_SHIFT) {
          input_handler(tree, "SELECT_END");
        } else if (keysym.sym == SDLK_UP &&
                   mod & KMOD_SHIFT) {
          input_handler(tree, "SELECT_START");
        } else if (keysym.sym == SDLK_z &&
                   mod & KMOD_CTRL) {
          input_handler(tree, "UNDO");
        } else if (keysym.sym == SDLK_y &&
                   mod & KMOD_CTRL) {
          input_handler(tree, "REDO");
        } else if (keysym.sym == SDLK_LEFT) {
          input_handler(tree, "MOVE_LEFT");
        } else if (keysym.sym == SDLK_RIGHT) {
          input_handler(tree, "MOVE_RIGHT");
        } else if (keysym.sym == SDLK_a &&
                   mod & KMOD_CTRL) {
          input_handler(tree, "SELECT_ALL");
        } else if (keysym.sym == SDLK_RETURN) {
          printf("Return\n");
        } else if (keysym.sym == SDLK_ESCAPE) {
          input_handler(tree, "DESELECT");
        }
      } else if (event.type == SDL_TEXTINPUT) {
        // Get text input content
        if (!(SDL_GetModState() & KMOD_CTRL)) {
          char *text = (char *)event.text.text;
          input_handler(tree, text);
        }
      } else if (event.type == SDL_MOUSEWHEEL) {
        i32 mouse_x = 0;
        i32 mouse_y = 0;
        SDL_GetMouseState(&mouse_x, &mouse_y);
        // scroll up or down
        if (event.wheel.y != 0) {
          scroll_y(tree->root, mouse_x, mouse_y, event.wheel.y * scroll_speed);
          set_y(tree->root, 0);
          tree->rerender = rerender_type.all;
        }
        // scroll left or right
        if (event.wheel.x != 0) {
          scroll_x(tree->root, mouse_x, mouse_y, event.wheel.x * -scroll_speed);
          set_x(tree->root, 0);
          tree->rerender = rerender_type.all;
        }
        SDL_FlushEvent(SDL_MOUSEWHEEL);
      } else if (event.type == SDL_MOUSEBUTTONDOWN) {
        i32 mouse_x = 0;
        i32 mouse_y = 0;
        SDL_GetMouseState(&mouse_x, &mouse_y);
        // Blur former active element
        blur_handler(tree, 0);
        // Set new active element
        tree->active_element = get_clickable_element_at(tree->root, mouse_x, mouse_y);
        // Set selection if active element has input
        if (tree->active_element != 0 &&
            tree->active_element->input != 0) {
          i32 relative_x_position = mouse_x - tree->active_element->layout.x - tree->active_element->padding.left;
          set_selection(tree->active_element->input, relative_x_position);
        }
        click_handler(tree, 0);
        SDL_FlushEvent(SDL_MOUSEBUTTONDOWN);
      } else if (event.type == SDL_QUIT) {
        done = true;
      }
    }

    if (tree->rerender == rerender_type.all) {
      SDL_SetRenderTarget(renderer, target_texture);
      render_element_tree(renderer, tree);
      // Draw target_texture to back buffer and present
      SDL_SetRenderTarget(renderer, NULL);
      SDL_RenderCopy(renderer, target_texture, NULL, NULL);
      SDL_RenderPresent(renderer);
      tree->rerender = rerender_type.none;
      tree->rerender_element = 0;
    } else if (tree->rerender == rerender_type.selected && tree->rerender_element != 0) {
      SDL_Rect target_rectangle = {
        .x = tree->rerender_element->layout.x,
        .y = tree->rerender_element->layout.y,
        .w = tree->rerender_element->layout.max_width,
        .h = tree->rerender_element->layout.max_height,
      };
      SDL_SetRenderTarget(renderer, target_texture);
      draw_elements(renderer, tree->rerender_element, target_rectangle, tree->active_element);
      // Draw target_texture to back buffer and present
      SDL_SetRenderTarget(renderer, NULL);
      SDL_RenderCopy(renderer, target_texture, NULL, NULL);
      SDL_RenderPresent(renderer);
      tree->rerender = rerender_type.none;
      tree->rerender_element = 0;
    }

    SDL_Delay(8);
  }; // End of rendering loop

  if (target_texture) {
    SDL_DestroyTexture(target_texture);
  }
  if (renderer) {
    SDL_DestroyRenderer(renderer);
  }
  if (window) {
    SDL_DestroyWindow(window);
  }
  SDL_StopTextInput();
  // printf("Size of element_arena %zu\n", arena_size(element_arena));
  arena_close(element_arena);
  close_font();
  SDL_Quit();

  return 0;
}
