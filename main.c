#include <SDL2/SDL.h> // SDL_CreateWindow, SDL_DestroyWindow, SDL_CreateRenderer, SDL_DestroyRenderer, SDL_SetRenderDrawColor, SDL_RenderClear, SDL_RenderPresent, SDL_Delay, SDL_Event, SDL_WaitEvent, SDL_WINDOWEVENT, SDL_WINDOWEVENT_RESIZED, SDL_SetWindowSize, SDL_FlushEvent, SDL_MOUSEMOTION, SDL_MOUSEWHEEL, SDL_MOUSEBUTTONDOWN, SDL_QUIT
#include <stdbool.h> // bool
#include <stdio.h> // printf
#include "components/form.h"
#include "components/search_bar.h"
#include "constants/color_theme.h"
#include "constants/element_tags.h"
#include "include/SDL_ttf.h" // TTF_Init, TTF_OpenFont, TTF_RenderText_Blended, TTF_CloseFont
#include "include/arena.h" // Arena, arena_open, arena_close
#include "include/color.h" // RGBA, C9_Gradient
#include "include/layout.h" // Element, ElementTree, new_element_tree, add_new_element, get_min_width, get_min_height, set_dimensions, layout_direction, background_type, background_gradient, Border, Padding
#include "include/renderer.h" // render_element_tree
#include "include/string.h" // to_s8
#include "include/types.h" // i32

void reset_menu_elements(Element *side_panel) {
  // Loop through all children and set background color to none
  Array *children = side_panel->children;
  if (children == 0) {
    return;
  }
  for (size_t i = 0; i < array_length(children); i++) {
    Element *child = array_get(children, i);
    child->background_type = background_type.none;
    child->text_color = text_color;
  }
}

void set_active_menu_element(Element *element) {
  element->background_type = background_type.color;
  element->text_color = text_color_active;
}

void set_menu(ElementTree *tree) {
  Element *active_element = tree->active_element;
  Element *side_panel = get_element_by_tag(tree->root, side_panel_tag);
  if (active_element != 0 && side_panel != 0) {
    reset_menu_elements(side_panel);
    set_active_menu_element(active_element);
    bump_rerender(tree);
    tree->rerender_element = side_panel;
  }
}

void set_content_panel(ElementTree *tree, Element *element) {
  Element *content_panel = get_element_by_tag(tree->root, content_panel_tag);
  if (content_panel != 0) {
    // Clear children and add new element
    content_panel->children = array_create(tree->arena, sizeof(Element));
    array_push(content_panel->children, element);

    // Reset scroll position
    content_panel->layout.scroll_x = 0;
    content_panel->layout.scroll_y = 0;

    // Recalculate content layout
    set_dimensions(tree, tree->root->layout.max_width, tree->root->layout.max_height);

    // Set rerendering
    bump_rerender(tree);
    tree->rerender_element = content_panel;
  }
}

void click_item_1(ElementTree *tree, void *data) {
  (void)data;
  set_menu(tree);
  if (form_element == 0) {
    create_form_element(tree->arena);
  }
  set_content_panel(tree, form_element);
}

void click_item_2(ElementTree *tree, void *data) {
  (void)data;
  set_menu(tree);
}

void click_item_3(ElementTree *tree, void *data) {
  (void)data;
  set_menu(tree);
}

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
    window_width, window_height, SDL_WINDOW_RESIZABLE
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
  if (TTF_Init()) {
    printf("TTF_Init\n");
    return -1;
  }
  TTF_Font *Inter = TTF_OpenFont("Inter-Regular.ttf", 16);
  SDL_Event event;

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
  };

  Element *content_panel = add_new_element(tree->arena, bottom_panel);
  *content_panel = (Element){
    .element_tag = content_panel_tag,
    .background_type = background_type.color,
    .background_color = white,
    .padding = (Padding){10, 10, 10, 10},
    .layout_direction = layout_direction.vertical,
    .gutter = 10,
    .overflow = overflow_type.scroll_y,
  };

  Element *content_panel_top = add_new_element(tree->arena, content_panel);
  *content_panel_top = (Element){
    .background_type = background_type.color,
    .background_color = gray_2,
    .border_radius = 15,
    .padding = (Padding){10, 10, 10, 10},
    .layout_direction = layout_direction.horizontal,
    .gutter = 10,
  };

  Element *content_panel_top_content = add_new_element(tree->arena, content_panel_top);
  *content_panel_top_content = (Element){
    .width = 100,
    .height = 600,
    .background_type = background_type.color,
    .background_color = white,
    .border = (Border){1, 1, 1, 1},
    .border_radius = 15,
  };

  Element *content_panel_top_content_2 = add_new_element(tree->arena, content_panel_top);
  *content_panel_top_content_2 = (Element){
    .width = 100,
    .height = 600,
    .background_type = background_type.color,
    .background_color = white,
    .border = (Border){2, 2, 2, 2},
    .border_radius = 30,
  };

  Element *content_panel_bottom = add_new_element(tree->arena, content_panel);
  *content_panel_bottom = (Element){
    .background_type = background_type.color,
    .background_color = gray_2,
    .border_radius = 15,
    .padding = (Padding){10, 10, 10, 10},
  };

  Element *menu_item = add_new_element(tree->arena, side_panel);
  *menu_item = (Element){
    .height = 30,
    .background_type = background_type.none,
    .background_color = menu_active_color,
    .padding = (Padding){5, 10, 5, 10},
    .border_radius = 15,
    .text = to_s8("Menu item 1"),
    .text_color = text_color,
    .on_click = &click_item_1,
  };

  Element *menu_item_2 = add_new_element(tree->arena, side_panel);
  *menu_item_2 = (Element){
    .height = 30,
    .background_type = background_type.none,
    .background_color = menu_active_color,
    .padding = (Padding){5, 10, 5, 10},
    .border_radius = 15,
    .text = to_s8("Menu item 2"),
    .text_color = text_color,
    .on_click = &click_item_2,
  };

  Element *menu_item_3 = add_new_element(tree->arena, side_panel);
  *menu_item_3 = (Element){
    .height = 30,
    .background_type = background_type.none,
    .background_color = menu_active_color,
    .padding = (Padding){5, 10, 5, 10},
    .border_radius = 15,
    .text = to_s8("Menu item 3"),
    .text_color = text_color,
    .on_click = &click_item_3,
  };

  // search_bar is defined in search_bar.h
  create_search_bar_element(tree->arena);
  add_element(tree->arena, top_right_panel, search_bar);

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
        }
        // scroll left or right
        if (event.wheel.x != 0) {
          scroll_x(tree->root, mouse_x, mouse_y, event.wheel.x * scroll_speed);
          set_x(tree->root, 0);
        }
        tree->rerender = rerender_type.all;
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
          set_selection(Inter, tree->active_element->input, relative_x_position);
        }
        click_handler(tree, 0);
        SDL_FlushEvent(SDL_MOUSEBUTTONDOWN);
      } else if (event.type == SDL_QUIT) {
        done = true;
      }
    }

    if (tree->rerender == rerender_type.all) {
      SDL_SetRenderTarget(renderer, target_texture);
      render_element_tree(renderer, Inter, tree);
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
      draw_elements(renderer, Inter, tree->rerender_element, target_rectangle, tree->active_element);
      // Draw target_texture to back buffer and present
      SDL_SetRenderTarget(renderer, NULL);
      SDL_RenderCopy(renderer, target_texture, NULL, NULL);
      SDL_RenderPresent(renderer);
      tree->rerender = rerender_type.none;
      tree->rerender_element = 0;
    }

    // Throttle frame rate to 125fps
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
  TTF_CloseFont(Inter);
  TTF_Quit();
  SDL_Quit();

  return 0;
}
