#ifndef C9_RENDERER

#include <SDL2/SDL.h> // SDL_rect, SDL_Texture
#include "SDL_ttf.h" // TTF_Font, TTF_SizeUTF8
#include "arena.h" // Arena, arena_fill
#include "array.h" // array_get
#include "draw_shapes.h" // draw_filled_rectangle, draw_horizontal_gradient_rectangle, draw_vertical_gradient_rectangle, draw_rectangle_with_border, draw_rectangle, BorderSize, largest_border
#include "element_tree.h" // Element, ElementTree
#include "font.h" // get_font
#include "input.h" // InputData, measure_selection
#include "types.h" // i32

// Recursively draws all elements
void draw_elements(SDL_Renderer *renderer, Element *element, SDL_Rect target_rect, Element *active_element) {
  TTF_Font *font = get_font();
  // Save the current render target for later
  SDL_Texture *target_texture = SDL_GetRenderTarget(renderer);

  // Rectangle that covers the entire element texture
  SDL_Rect element_texture_rect = {
    .x = 0,
    .y = 0,
    .w = element->layout.max_width,
    .h = element->layout.max_height,
  };

  // Rectangle that represents the true uncut position of an element
  SDL_Rect element_rect = {
    .x = element->layout.x,
    .y = element->layout.y,
    .w = element->layout.max_width,
    .h = element->layout.max_height,
  };

  // Target left edge (where to copy to)
  i32 target_left_edge = 0;
  // Target right edge (how far to copy)
  i32 target_right_edge = 0;
  // Texture left edge (where to copy from)
  i32 texture_left_edge = 0;

  // Target top edge (where to copy to)
  i32 target_top_edge = 0;
  // Texture top edge (where to copy from)
  i32 texture_top_edge = 0;
  // Target bottom edge (how far to copy)
  i32 target_bottom_edge = 0;

  // Set left and right bounds
  if (element_rect.x < target_rect.x) {
    texture_left_edge = target_rect.x - element_rect.x; // start copying a bit into the texture
    target_left_edge = target_rect.x; // copy to the target x
  } else {
    texture_left_edge = 0; // start copying from the beginning of the texture
    target_left_edge = element_rect.x; // copy to the element x position
  }
  if (element_rect.x + element_rect.w > target_rect.x + target_rect.w) {
    target_right_edge = target_rect.x + target_rect.w; // copy to the end of the target texture
  } else {
    target_right_edge = element_rect.x + element_rect.w; // copy to the end of the element
  }
  // Set top and bottom bounds
  if (element_rect.y < target_rect.y) {
    texture_top_edge = target_rect.y - element_rect.y; // start copying a bit into the texture
    target_top_edge = target_rect.y; // copy to the target y
  } else {
    texture_top_edge = 0; // start copying from the beginning of the texture
    target_top_edge = element_rect.y; // copy to the element y position
  }
  if (element_rect.y + element_rect.h > target_rect.y + target_rect.h) {
    target_bottom_edge = target_rect.y + target_rect.h; // copy to the end of the target texture
  } else {
    target_bottom_edge = element_rect.y + element_rect.h; // copy to the end of the element
  }

  SDL_Rect texture_rect = {
    .x = texture_left_edge,
    .y = texture_top_edge,
    .w = target_right_edge - target_left_edge,
    .h = target_bottom_edge - target_top_edge,
  };
  SDL_Rect render_target_rect = {
    .x = target_left_edge,
    .y = target_top_edge,
    .w = target_right_edge - target_left_edge,
    .h = target_bottom_edge - target_top_edge,
  };

  // Invalid shape
  if (element_texture_rect.w == 0 || element_texture_rect.h == 0) {
    return;
  };

  // If the element has a cached texture and it hasn't changed we just copy it
  if (element->render.texture != 0 &&
      element->render.changed == 0 &&
      element->render.width == element_texture_rect.w &&
      element->render.height == element_texture_rect.h) {
    // Copy a portion of the element texture to the same location on the target texture
    SDL_RenderCopy(renderer, element->render.texture, &texture_rect, &render_target_rect);
  } else {
    // If the element has a cached texture but its dimensions are not correct we need to destroy it
    if (element->render.texture != 0 && (element->render.width != element_texture_rect.w || element->render.height != element_texture_rect.h)) {
      SDL_DestroyTexture(element->render.texture);
      element->render.texture = 0;
    }
    // If the element doesn't have a cached texture we need to create one
    if (element->render.texture == 0) {
      element->render.texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, element_texture_rect.w, element_texture_rect.h);
      SDL_SetTextureBlendMode(element->render.texture, SDL_BLENDMODE_BLEND);
      element->render.width = element_texture_rect.w;
      element->render.height = element_texture_rect.h;
    }
    // Set the element texture as the render target
    SDL_SetRenderTarget(renderer, element->render.texture);
    // Clear the texture
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 0);
    SDL_RenderClear(renderer);
    // Draw the background to the texture
    SDL_RenderCopy(renderer, target_texture, &render_target_rect, &texture_rect);

    // Convert from Border to BorderSize
    BorderSize border_size = {
      .top = element->border.top,
      .right = element->border.right,
      .bottom = element->border.bottom,
      .left = element->border.left
    };
    if (element->background_type == background_type.color) {
      if (largest_border(border_size) > 0) {
        draw_rectangle_with_border(renderer, element_texture_rect, element->corner_radius, border_size, element->border_color, element->background_color);
      } else {
        draw_filled_rectangle(renderer, element_texture_rect, element->corner_radius, element->background_color);
      }
    } else if (element->background_type == background_type.horizontal_gradient) {
      if (largest_border(border_size) > 0) {
        draw_horizontal_gradient_rectangle_with_border(renderer, element_texture_rect, element->corner_radius, border_size, element->border_color, element->background_gradient);
      } else {
        draw_horizontal_gradient_rectangle(renderer, element_texture_rect, element->corner_radius, element->background_gradient);
      }
    } else if (element->background_type == background_type.vertical_gradient) {
      if (largest_border(border_size) > 0) {
        draw_vertical_gradient_rectangle_with_border(renderer, element_texture_rect, element->corner_radius, border_size, element->border_color, element->background_gradient);
      } else {
        draw_vertical_gradient_rectangle(renderer, element_texture_rect, element->corner_radius, element->background_gradient);
      }
    } else if (element->background_type == background_type.image) {
      if (element->background_image.length > 0) {
        draw_image(renderer, to_char(element->background_image), element_texture_rect);
      }
    }
    if (element->text.length > 0) {
      i32 text_x = element_texture_rect.x + element->padding.left + element->layout.scroll_x;
      i32 text_y = element_texture_rect.y + element->padding.top + element->layout.scroll_y;
      draw_text(
        renderer, font, to_char(element->text), text_x, text_y, element->text_color
      );
    } else if (element->input != 0) {
      // If the element is the active element we should also draw the cursor
      if (element == active_element) {
        SDL_Rect selection_rect = measure_selection(font, element->input);
        SDL_Rect selection = {
          .x = element_texture_rect.x + element->padding.left + selection_rect.x - 1, // Subtract 1 pixel for the cursor
          .y = element_texture_rect.y + element->padding.top + selection_rect.y,
          .w = selection_rect.w + 2, // Add 2 pixels for the cursor
          .h = element_texture_rect.h - element->padding.top - element->padding.bottom,
        };
        if (selection_rect.w == 0) {
          draw_filled_rectangle(renderer, selection, 0, text_cursor_color);
        } else {
          draw_filled_rectangle(renderer, selection, 0, selection_color);
        }
      }
      InputData input = *element->input;
      i32 text_x = element_texture_rect.x + element->padding.left;
      i32 text_y = element_texture_rect.y + element->padding.top;
      char *text_data = (char *)input.text.data;

      draw_text(
        renderer, font, text_data, text_x, text_y, element->text_color
      );
    }

    // Reset the target texture as the render target
    SDL_SetRenderTarget(renderer, target_texture);
    // Copy a portion of the element texture to the same location on the target texture
    SDL_RenderCopy(renderer, element->render.texture, &texture_rect, &render_target_rect);
    // Set the element as unchanged
    element->render.changed = 0;
  }

  Array *children = element->children;
  if (children == 0) return;

  for (size_t i = 0; i < array_length(children); i++) {
    Element *child = array_get(children, i);
    draw_elements(renderer, child, render_target_rect, active_element);
  }

  // Draw a scrollbar if the element has Y overflow
  if ((element->overflow == overflow_type.scroll ||
       element->overflow == overflow_type.scroll_y) &&
      element->children != 0 &&
      element_rect.h < element->layout.scroll_height) {
    f32 scroll_percentage = -element->layout.scroll_y / (f32)(element->layout.scroll_height - element_rect.h);

    i32 scrollbar_width = 4;
    i32 scrollbar_x = element_rect.x + element_rect.w - scrollbar_width;
    i32 scrollbar_height = element_rect.h * element_rect.h / element->layout.scroll_height;
    i32 scrollbar_y = element_rect.y + scroll_percentage * (element_rect.h - scrollbar_height);

    SDL_Rect scrollbar_rect = {
      .x = scrollbar_x,
      .y = scrollbar_y,
      .w = scrollbar_width,
      .h = scrollbar_height,
    };
    draw_filled_rectangle(renderer, scrollbar_rect, 0, scrollbar_color);
  }
  // Draw a scrollbar if the element has X overflow
  if ((element->overflow == overflow_type.scroll ||
       element->overflow == overflow_type.scroll_x) &&
      element->children != 0 &&
      element_rect.w < element->layout.scroll_width) {
    f32 scroll_percentage = -element->layout.scroll_x / (f32)(element->layout.scroll_width - element_rect.w);

    i32 scrollbar_height = 4;
    i32 scrollbar_y = element_rect.y + element_rect.h - scrollbar_height;
    i32 scrollbar_width = element_rect.w * element_rect.w / element->layout.scroll_width;
    i32 scrollbar_x = element_rect.x + scroll_percentage * (element_rect.w - scrollbar_width);

    SDL_Rect scrollbar_rect = {
      .x = scrollbar_x,
      .y = scrollbar_y,
      .w = scrollbar_width,
      .h = scrollbar_height,
    };
    draw_filled_rectangle(renderer, scrollbar_rect, 0, scrollbar_color);
  }
}

void render_element_tree(SDL_Renderer *renderer, ElementTree *element_tree) {
  // Clear buffer
  SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
  SDL_RenderClear(renderer);
  // Create a rectangle that covers the entire target texture
  SDL_Texture *target_texture = SDL_GetRenderTarget(renderer);
  SDL_Rect target_rectangle = {0, 0, 0, 0};
  // Get the width and height of the target texture
  SDL_QueryTexture(target_texture, NULL, NULL, &target_rectangle.w, &target_rectangle.h);
  draw_elements(renderer, element_tree->root, target_rectangle, element_tree->active_element);
}

// Sets selective rerendering if no rendering is set and all if selective is set
void bump_rerender(ElementTree *tree) {
  if (tree->rerender == rerender_type.none) {
    tree->rerender = rerender_type.selected;
  } else {
    tree->rerender = rerender_type.all;
  }
}

#define C9_RENDERER
#endif