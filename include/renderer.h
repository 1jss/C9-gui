#ifndef C9_RENDERER

#include <SDL2/SDL.h>
#include <stdio.h>
#include "SDL_ttf.h" // TTF_Font, TTF_SizeUTF8
#include "arena.h" // Arena, arena_fill
#include "array.h" // array_get
#include "draw_shapes.h" // draw_filled_rectangle, draw_horizontal_gradient, draw_vertical_gradient, draw_rounded_rectangle_with_border, draw_filled_rounded_rectangle, BorderSize, largest_border
#include "font.h" // get_font
#include "input.h" // InputData, measure_selection
#include "layout.h" // Element, ElementTree
#include "types.h" // i32

// Recursively draws all elements
void draw_elements(SDL_Renderer *renderer, Element *element, SDL_Rect target_rect, Element *active_element) {
  TTF_Font *font = get_font();
  // Save the current render target for later
  SDL_Texture *target_texture = SDL_GetRenderTarget(renderer);
  // Set the width and height of the target texture
  i32 target_right_edge = target_rect.x + target_rect.w;
  i32 target_bottom_edge = target_rect.y + target_rect.h;
  // Create a new texture to draw the element on
  SDL_Texture *element_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, target_right_edge, target_bottom_edge);
  SDL_SetTextureBlendMode(element_texture, SDL_BLENDMODE_BLEND);
  // Set the new texture as the render target
  SDL_SetRenderTarget(renderer, element_texture);
  // Clear the texture
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
  SDL_RenderClear(renderer);
  SDL_RenderCopy(renderer, target_texture, &target_rect, &target_rect);

  SDL_Rect element_rect = {
    .x = element->layout.x,
    .y = element->layout.y,
    .w = element->layout.max_width,
    .h = element->layout.max_height,
  };
  // Invalid shape
  if (element_rect.w == 0 || element_rect.h == 0) {
    return;
  };

  // Convert from Border to BorderSize
  BorderSize border_size = {
    .top = element->border.top,
    .right = element->border.right,
    .bottom = element->border.bottom,
    .left = element->border.left
  };
  if (element->background_type == background_type.color) {
    if (element->corner_radius > 0) {
      if (largest_border(border_size) > 0) {
        draw_rounded_rectangle_with_border(renderer, element_rect, element->corner_radius, border_size, element->border_color, element->background_color);
      } else {
        draw_filled_rounded_rectangle(renderer, element_rect, element->corner_radius, element->background_color);
      }
    } else {
      draw_filled_rectangle(renderer, element_rect, element->background_color);
      draw_border(renderer, element_rect, border_size, element->border_color);
    }
  } else if (element->background_type == background_type.horizontal_gradient) {
    if (element->corner_radius > 0) {
      if (largest_border(border_size) > 0) {
        draw_horizontal_gradient_rounded_rectangle_with_border(renderer, element_rect, element->corner_radius, border_size, element->border_color, element->background_gradient);
      } else {
        draw_horizontal_gradient_rounded_rectangle(renderer, element_rect, element->corner_radius, element->background_gradient);
      }
    } else {
      draw_horizontal_gradient(renderer, element_rect, element->background_gradient);
      draw_border(renderer, element_rect, border_size, element->border_color);
    }
  } else if (element->background_type == background_type.vertical_gradient) {
    if (element->corner_radius > 0) {
      if (largest_border(border_size) > 0) {
        draw_vertical_gradient_rounded_rectangle_with_border(renderer, element_rect, element->corner_radius, border_size, element->border_color, element->background_gradient);
      } else {
        draw_vertical_gradient_rounded_rectangle(renderer, element_rect, element->corner_radius, element->background_gradient);
      }
    } else {
      draw_vertical_gradient(renderer, element_rect, element->background_gradient);
      draw_border(renderer, element_rect, border_size, element->border_color);
    }
  }
  if (element->text.length > 0) {
    i32 text_x = element_rect.x + element->padding.left + element->layout.scroll_x;
    i32 text_y = element_rect.y + element->padding.top + element->layout.scroll_y;
    draw_text(
      renderer, font, to_char(element->text), text_x, text_y, element->text_color
    );
  } else if (element->input != 0) {
    // If the element is the active element we should also draw the cursor
    if (element == active_element) {
      SDL_Rect selection_rect = measure_selection(font, element->input);
      SDL_Rect selection = {
        .x = element_rect.x + element->padding.left + selection_rect.x - 1, // Subtract 1 pixel for the cursor
        .y = element_rect.y + element->padding.top + selection_rect.y,
        .w = selection_rect.w + 2, // Add 2 pixels for the cursor
        .h = element_rect.h - element->padding.top - element->padding.bottom,
      };
      if (selection_rect.w == 0) {
        draw_filled_rectangle(renderer, selection, text_cursor_color);
      } else {
        draw_filled_rectangle(renderer, selection, selection_color);
      }
    }
    InputData input = *element->input;
    i32 text_x = element_rect.x + element->padding.left;
    i32 text_y = element_rect.y + element->padding.top;
    char *text_data = (char *)input.text.data;

    draw_text(
      renderer, font, text_data, text_x, text_y, element->text_color
    );
  }

  // Reset the target texture as the render target
  SDL_SetRenderTarget(renderer, target_texture);
  // Copy a portion of the element texture to the same location on the target texture
  SDL_RenderCopy(renderer, element_texture, &target_rect, &target_rect);
  if (element_texture) {
    SDL_DestroyTexture(element_texture);
  }

  Array *children = element->children;
  if (children == 0) return;

  // Set outer bounds for the children
  i32 child_left_edge = 0;
  i32 child_right_edge = 0;
  i32 child_top_edge = 0;
  i32 child_bottom_edge = 0;
  i32 element_right_edge = element_rect.x + element_rect.w;
  i32 element_bottom_edge = element_rect.y + element_rect.h;

  // Set left and right bounds
  if (element_rect.x < target_rect.x) {
    child_left_edge = target_rect.x;
  } else {
    child_left_edge = element_rect.x;
  }
  if (element_right_edge > target_right_edge) {
    child_right_edge = target_right_edge;
  } else {
    child_right_edge = element_right_edge;
  }

  // Set top and bottom bounds
  if (element_rect.y < target_rect.y) {
    child_top_edge = target_rect.y;
  } else {
    child_top_edge = element_rect.y;
  }
  if (element_bottom_edge > target_bottom_edge) {
    child_bottom_edge = target_bottom_edge;
  } else {
    child_bottom_edge = element_bottom_edge;
  }

  SDL_Rect child_bounds_rect = {
    .x = child_left_edge,
    .y = child_top_edge,
    .w = child_right_edge - child_left_edge,
    .h = child_bottom_edge - child_top_edge,
  };
  for (size_t i = 0; i < array_length(children); i++) {
    Element *child = array_get(children, i);
    draw_elements(renderer, child, child_bounds_rect, active_element);
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
    draw_filled_rectangle(renderer, scrollbar_rect, scrollbar_color);
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
    draw_filled_rectangle(renderer, scrollbar_rect, scrollbar_color);
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