#ifndef C9_EVENT

#include <SDL2/SDL.h> // SDL_WaitEvent, SDL_WINDOWEVENT, SDL_WINDOWEVENT_RESIZED, SDL_SetWindowSize, SDL_FlushEvent, SDL_MOUSEWHEEL, SDL_MOUSEBUTTONDOWN, SDL_QUIT, SDL_Event, SDL_Window, SDL_Renderer
#include <stdbool.h> // bool
#include "element_tree.h" // ElementTree, Element
#include "layout.h" // fill_scroll_width
#include "types.h" // i32

void click_handler(ElementTree *tree, void *data) {
  Element *element = tree->active_element;
  if (element != 0 && element->on_click != 0) {
    element->on_click(tree, data);
    element->render.changed = 1;
  }
}

void blur_handler(ElementTree *tree, void *data) {
  Element *element = tree->active_element;
  if (element != 0 && element->on_blur != 0) {
    element->on_blur(tree, data);
    element->render.changed = 1;
  }
}

void input_handler(ElementTree *tree, void *data) {
  Element *element = tree->active_element;
  if (element != 0 && element->on_key_press != 0) {
    element->on_key_press(tree, data);
    element->render.changed = 1;
    fill_scroll_width(element);
  }
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
          // Cap to min width and height
          i32 min_width = get_min_width(tree->root);
          i32 min_height = get_min_height(tree->root);
          i32 width = event.window.data1;
          i32 height = event.window.data2;
          if (width < min_width) {
            width = min_width;
          }
          if (height < min_height) {
            height = min_height;
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
          set_dimensions(tree, width, height);
          tree->rerender = rerender_type.all;
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
        } else if (keysym.sym == SDLK_DOWN &&
                   mod & KMOD_SHIFT) {
          input_handler(tree, "SELECT_END");
        } else if (keysym.sym == SDLK_UP &&
                   mod & KMOD_SHIFT) {
          input_handler(tree, "SELECT_START");
        } else if (keysym.sym == SDLK_LEFT) {
          input_handler(tree, "MOVE_LEFT");
        } else if (keysym.sym == SDLK_RIGHT) {
          input_handler(tree, "MOVE_RIGHT");
        } else if (keysym.sym == SDLK_a && ctrl_cmd) {
          input_handler(tree, "SELECT_ALL");
        } else if (keysym.sym == SDLK_RETURN) {
          printf("Return\n");
        } else if (keysym.sym == SDLK_ESCAPE) {
          input_handler(tree, "DESELECT");
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
          if (!tree->scroll.is_active) {
            tree->scroll.is_active = true;
            tree->scroll.last_x = event.mgesture.x;
            tree->scroll.last_y = event.mgesture.y;
          } else {
            scroll_distance_x += (event.mgesture.x - tree->scroll.last_x) * 800;
            scroll_distance_y += (event.mgesture.y - tree->scroll.last_y) * 800;
            tree->scroll.last_x = event.mgesture.x;
            tree->scroll.last_y = event.mgesture.y;
          }
        }
        // We do not flush the SDL_MULTIGESTURE event here as we want to add up the scroll distance for each frame
      } else if (event.type == SDL_FINGERDOWN) {
        tree->scroll.is_active = false;
      } else if (event.type == SDL_MOUSEBUTTONDOWN) {
        // Blur former active element
        blur_handler(tree, 0);
        Element *click_root = tree->overlay != 0 ? tree->overlay : tree->root;
        // Set new active element
        tree->active_element = get_clickable_element_at(click_root, mouse_x, mouse_y);
        // Set selection if active element has input
        if (tree->active_element != 0 &&
            tree->active_element->input != 0) {
          // Doubble click selects word
          if (event.button.clicks == 2) {
            i32 relative_x_position = mouse_x - tree->active_element->layout.x - tree->active_element->padding.left - tree->active_element->layout.scroll_x;
            select_word(tree->active_element->input, relative_x_position);
          }
          // Triple click selects all
          else if (event.button.clicks == 3) {
            input_handler(tree, "SELECT_ALL");
          } else {
            i32 relative_x_position = mouse_x - tree->active_element->layout.x - tree->active_element->padding.left - tree->active_element->layout.scroll_x;
            set_selection(tree->active_element->input, relative_x_position);
          }
        }
        click_handler(tree, 0);
        SDL_FlushEvent(SDL_MOUSEBUTTONDOWN);
      } else if (event.type == SDL_MOUSEMOTION) {
        if (mouse_button_down & SDL_BUTTON_LMASK &&
            tree->active_element != 0 &&
            tree->active_element->input != 0) {
          i32 relative_x_position = mouse_x - tree->active_element->layout.x - tree->active_element->padding.left - tree->active_element->layout.scroll_x;
          set_selection_end(tree->active_element->input, relative_x_position);
          // Set input to rerender
          tree->active_element->render.changed = 1;
          // Redraw parent to prevent bleeding corners
          tree->rerender_element = get_parent(tree, tree->active_element);
          bump_rerender(tree);
        }
        SDL_FlushEvent(SDL_MOUSEMOTION);
      } else if (event.type == SDL_QUIT) {
        main_loop = false;
      }
    }
    if (tree->scroll.is_active) {
      Element *scroll_root = tree->overlay != 0 ? tree->overlay : tree->root;
      // Scroll left or right
      if (scroll_distance_x != 0) {
        i32 remaining_scroll = scroll_x(scroll_root, mouse_x, mouse_y, scroll_distance_x);
        if (remaining_scroll != scroll_distance_x) {
          set_x(scroll_root, 0);
          tree->rerender = rerender_type.all;
        }
      }
      // Scroll up or down
      if (scroll_distance_y != 0) {
        i32 remaining_scroll = scroll_y(scroll_root, mouse_x, mouse_y, scroll_distance_y);
        if (remaining_scroll != scroll_distance_y) {
          set_y(scroll_root, 0);
          tree->rerender = rerender_type.all;
        }
      }
    }
  }
  return main_loop;
}

#define C9_EVENT
#endif