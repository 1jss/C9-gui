#ifndef C9_EVENT

#include <SDL2/SDL.h> // SDL_WaitEvent, SDL_WINDOWEVENT, SDL_WINDOWEVENT_RESIZED, SDL_SetWindowSize, SDL_FlushEvent, SDL_MOUSEWHEEL, SDL_MOUSEBUTTONDOWN, SDL_QUIT, SDL_Event, SDL_Window, SDL_Renderer
#include <stdbool.h> // bool
#include <stdio.h> // printf
#include "element_tree.h" // ElementTree, Element
#include "input_actions.h" // select_word, set_selection_start_index, set_selection_end_index
#include "layout.h" // fill_scroll_width, get_clickable_element_at
#include "types.h" // i32
#include "types_draw.h" // Position

void rerender_element(ElementTree *tree, Element *element) {
  element->changed = true;
  tree->rerender = true;
}

void click_handler(ElementTree *tree, void *data) {
  Element *element = tree->active_element;
  if (element != 0 && element->on_click != 0) {
    element->on_click(tree, data);
    rerender_element(tree, element);
  }
}

void blur_handler(ElementTree *tree, void *data) {
  Element *element = tree->active_element;
  if (element != 0 && element->on_blur != 0) {
    element->on_blur(tree, data);
    rerender_element(tree, element);
  }
}

void input_handler(ElementTree *tree, void *data) {
  Element *element = tree->active_element;
  // Automatically handle text input
  if (element != 0 && element->input != 0) {
    char *text = (char *)data;
    handle_text_input(element->input, text);
    set_dimensions(tree);
    rerender_element(tree, element);
  }
  // Handle custom key press functions
  if (element != 0 && element->on_key_press != 0) {
    element->on_key_press(tree, data);
    rerender_element(tree, element);
  }
}

i32 absolute(i32 value) {
  return value < 0 ? -value : value;
}

void select_up(Element *element) {
  i32 end_index = element->input->selection.end_index;
  Position position = position_from_index(end_index, element);
  position.y -= get_text_line_height(element->font_variant);
  end_index = index_from_position(position, element);
  set_selection_end_index(element->input, end_index);
}

void select_down(Element *element) {
  i32 end_index = element->input->selection.end_index;
  Position position = position_from_index(end_index, element);
  position.y += get_text_line_height(element->font_variant);
  end_index = index_from_position(position, element);
  set_selection_end_index(element->input, end_index);
}

void move_up(Element *element) {
  i32 index = element->input->selection.end_index;
  Position position = position_from_index(index, element);
  position.y -= get_text_line_height(element->font_variant);
  index = index_from_position(position, element);
  set_selection_start_index(element->input, index);
  set_selection_end_index(element->input, index);
}

void move_down(Element *element) {
  i32 index = element->input->selection.end_index;
  Position position = position_from_index(index, element);
  position.y += get_text_line_height(element->font_variant);
  index = index_from_position(position, element);
  set_selection_start_index(element->input, index);
  set_selection_end_index(element->input, index);
}

bool handle_events(ElementTree *tree, SDL_Window *window, SDL_Renderer *renderer) {
  bool main_loop = true;
  SDL_Event event;
  // Wait for events before continuing
  if (SDL_WaitEvent(NULL)) {
    i32 mouse_x = 0;
    i32 mouse_y = 0;
    i32 mouse_button_down = SDL_GetMouseState(&mouse_x, &mouse_y);
    i32 scroll_distance_x = 0;
    i32 scroll_distance_y = 0;

    // Handle all available events before rendering
    while (main_loop && SDL_PollEvent(&event)) {
      if (event.type == SDL_WINDOWEVENT) {
        if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
       
          i32 width = event.window.data1;
          i32 height = event.window.data2;
          if (width < tree->size.min_width) {
            width = tree->size.min_width;
          }
          if (height < tree->size.min_height) {
            height = tree->size.min_height;
          }
          SDL_SetWindowSize(window, width, height);
          // Recreate the target-texture with the new dimensions
          if (tree->target_texture) {
            SDL_DestroyTexture(tree->target_texture);
            tree->target_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, width, height);
            if (!tree->target_texture) {
              printf("SDL_CreateTexture failed: %s\n", SDL_GetError());
            }
          }
          // Set new dimensions for the tree
          tree->size.width = width;
          tree->size.height = height;
          set_dimensions(tree);
          tree->rerender = true;
        }
      } else if (event.type == SDL_KEYDOWN) {
        SDL_Keymod mod = SDL_GetModState();
        // Get key press content
        SDL_Keysym keysym = event.key.keysym;
        bool ctrl_cmd = mod & (KMOD_CTRL + KMOD_GUI);
        if (keysym.sym == SDLK_BACKSPACE) {
          input_handler(tree, "BACKSPACE");
        } else if (keysym.sym == SDLK_LEFT &&
                   mod & KMOD_SHIFT) {
          input_handler(tree, "SELECT_LEFT");
        } else if (keysym.sym == SDLK_RIGHT &&
                   mod & KMOD_SHIFT) {
          input_handler(tree, "SELECT_RIGHT");
        } else if (keysym.sym == SDLK_UP &&
                   mod & KMOD_SHIFT) {
          Element *element = tree->active_element;
          if (element != 0 && element->input != 0) {
            select_up(element);
            rerender_element(tree, element);
          } else {
            input_handler(tree, "SELECT_START");
          }
        } else if (keysym.sym == SDLK_DOWN &&
                   mod & KMOD_SHIFT) {
          Element *element = tree->active_element;
          if (element != 0 && element->input != 0) {
            select_down(element);
            rerender_element(tree, element);
          } else {
            input_handler(tree, "SELECT_END");
          }
        } else if (keysym.sym == SDLK_LEFT) {
          input_handler(tree, "MOVE_LEFT");
        } else if (keysym.sym == SDLK_RIGHT) {
          input_handler(tree, "MOVE_RIGHT");
        } else if (keysym.sym == SDLK_UP) {
          Element *element = tree->active_element;
          if (element != 0 && element->input != 0) {
            move_up(element);
            rerender_element(tree, element);
          } else {
            input_handler(tree, "MOVE_UP");
          }
        } else if (keysym.sym == SDLK_DOWN) {
          Element *element = tree->active_element;
          if (element != 0 && element->input != 0) {
            move_down(element);
            rerender_element(tree, element);
          } else {
            input_handler(tree, "MOVE_DOWN");
          }
        } else if (keysym.sym == SDLK_a && ctrl_cmd) {
          input_handler(tree, "SELECT_ALL");
        } else if (keysym.sym == SDLK_RETURN) {
          Element *element = tree->active_element;
          if (element != 0 && element->input != 0 &&
              element->overflow != overflow_type.scroll &&
              element->overflow != overflow_type.scroll_x) {
            input_handler(tree, "\n");
          }
        } else if (keysym.sym == SDLK_ESCAPE) {
          input_handler(tree, "ESCAPE");
        } else if (keysym.sym == SDLK_z && ctrl_cmd) {
          input_handler(tree, "UNDO");
        } else if (keysym.sym == SDLK_y && ctrl_cmd) {
          input_handler(tree, "REDO");
        } else if (keysym.sym == SDLK_c && ctrl_cmd) {
          input_handler(tree, "COPY");
        } else if (keysym.sym == SDLK_x && ctrl_cmd) {
          input_handler(tree, "CUT");
        } else if (keysym.sym == SDLK_v && ctrl_cmd) {
          input_handler(tree, "PASTE");
        }
      } else if (event.type == SDL_TEXTINPUT) {
        // Get text input content
        if (!(SDL_GetModState() & (KMOD_CTRL + KMOD_GUI))) {
          char *text = (char *)event.text.text;
          input_handler(tree, text);
        }
      } else if (event.type == SDL_MULTIGESTURE) {
        if (event.mgesture.numFingers == 2) {
          if (tree->scroll.state == scroll_state.available) {
            tree->scroll.state = scroll_state.active;
            tree->scroll.last_x = event.mgesture.x;
            tree->scroll.last_y = event.mgesture.y;
          } else if (tree->scroll.state == scroll_state.active) {
            scroll_distance_x += (event.mgesture.x - tree->scroll.last_x) * 800;
            scroll_distance_y += (event.mgesture.y - tree->scroll.last_y) * 800;
            tree->scroll.last_x = event.mgesture.x;
            tree->scroll.last_y = event.mgesture.y;
          }
        }
        // We do not flush the SDL_MULTIGESTURE event here as we want to add up the scroll distance for each frame
      } else if (event.type == SDL_FINGERDOWN) {
        if (tree->scroll.state != scroll_state.blocked) {
          tree->scroll.state = scroll_state.available;
        }
      } else if (event.type == SDL_MOUSEBUTTONDOWN) {
        // Blur former active element
        blur_handler(tree, 0);
        Element *click_root = tree->overlay != 0 ? tree->overlay : tree->root;
        // Set new active element
        tree->active_element = get_clickable_element_at(click_root, mouse_x, mouse_y);
        Position mouse = {mouse_x, mouse_y};
        // Run on_click function
        click_handler(tree, &mouse);
        // Set selection if active element has input
        if (tree->active_element != 0 &&
            tree->active_element->input != 0) {
          // Doubble click selects word
          if (event.button.clicks == 2) {
            i32 click_index = index_from_position(mouse, tree->active_element);
            select_word_at_index(tree->active_element->input, click_index);
          }
          // Triple click selects all
          else if (event.button.clicks == 3) {
            input_handler(tree, "SELECT_ALL");
          } else {
            i32 start_index = index_from_position(mouse, tree->active_element);
            set_selection_start_index(tree->active_element->input, start_index);
            set_selection_end_index(tree->active_element->input, start_index);
          }
        }
        SDL_FlushEvent(SDL_MOUSEBUTTONDOWN);
      } else if (event.type == SDL_MOUSEBUTTONUP) {
        tree->scroll.state = scroll_state.available;
        SDL_FlushEvent(SDL_MOUSEBUTTONUP);
      } else if (event.type == SDL_MOUSEMOTION) {
        if (mouse_button_down & SDL_BUTTON_LMASK &&
            tree->active_element != 0 &&
            tree->active_element->input != 0) {
          tree->scroll.state = scroll_state.blocked;
          Position mouse = {mouse_x, mouse_y};
          i32 end_index = index_from_position(mouse, tree->active_element);
          set_selection_end_index(tree->active_element->input, end_index);
          rerender_element(tree, tree->active_element);
        }
        SDL_FlushEvent(SDL_MOUSEMOTION);
      } else if (event.type == SDL_QUIT) {
        main_loop = false;
      }
    }
    if (tree->scroll.state == scroll_state.active) {
      Element *scroll_layer = get_root(tree);
      // Scroll left or right
      if (scroll_distance_x != 0 && absolute(scroll_distance_x) > absolute(scroll_distance_y)) {
        i32 remaining_scroll = scroll_x(scroll_layer, mouse_x, mouse_y, scroll_distance_x);
        if (remaining_scroll != scroll_distance_x) {
          set_x(scroll_layer, 0);
          tree->rerender = true;
        }
      }
      // Scroll up or down
      else if (scroll_distance_y != 0) {
        i32 remaining_scroll = scroll_y(scroll_layer, mouse_x, mouse_y, scroll_distance_y);
        if (remaining_scroll != scroll_distance_y) {
          set_y(scroll_layer, 0);
          tree->rerender = true;
        }
      }
    }
  }
  return main_loop;
}

#define C9_EVENT
#endif