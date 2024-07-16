#ifndef C9_RENDERER

#include <SDL2/SDL.h>
#include <stdio.h>
#include <string.h> // memcpy
#include "SDL_ttf.h" // TTF_Font, TTF_SizeUTF8
#include "array.h" // array_get
#include "draw_shapes.h" // draw_filled_rectangle, draw_horizontal_gradient, draw_rectangle_with_border, draw_filled_rounded_rectangle, draw_superellipse, draw_filled_superellipse
#include "input.h" // InputData
#include "layout.h" // Element, ElementTree
#include "types.h" // i32
#include "arena.h" // Arena, arena_fill

SDL_Rect measure_selection(TTF_Font *font, InputData *input) {
  Arena *temp_arena = arena_open(sizeof(char) * input->text.capacity);
  u32 start_index = input->selection.start_index;
  u32 end_index = input->selection.end_index;
  fs8 text = input->text;

  // Measure the text before the selection
  char *before_selection = arena_fill(temp_arena, start_index + 1); // +1 for null terminator
  memcpy(before_selection, text.data, start_index);
  // Add null-terminator
  before_selection[start_index] = '\0';
  i32 selection_x;
  TTF_SizeUTF8(font, before_selection, &selection_x, 0);

  // Measure the selected text
  i32 selection_length = end_index - start_index;
  char *selected_text = arena_fill(temp_arena, selection_length + 1); // +1 for null terminator
  memcpy(selected_text, text.data + start_index, selection_length);
  // Add null-terminator
  selected_text[selection_length] = '\0';
  i32 selection_w;
  i32 selection_h;
  TTF_SizeUTF8(font, selected_text, &selection_w, &selection_h);

  arena_close(temp_arena);
  SDL_Rect result = {
    .x = selection_x,
    .y = 0,
    .w = selection_w,
    .h = selection_h
  };
  return result;
}

// Recursively draws all elements
void draw_elements(SDL_Renderer *renderer, TTF_Font *font, Element *element, SDL_Rect target_rectangle, Element *active_element) {
  // Save the current render target for later
  SDL_Texture *target_texture = SDL_GetRenderTarget(renderer);
  // Get the width and height of the target texture
  i32 target_width = 0;
  i32 target_height = 0;
  SDL_QueryTexture(target_texture, NULL, NULL, &target_width, &target_height);
  // Create a new texture to draw the element on
  SDL_Texture *element_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, target_width, target_height);
  SDL_SetTextureBlendMode(element_texture, SDL_BLENDMODE_BLEND);
  // Set the new texture as the render target
  SDL_SetRenderTarget(renderer, element_texture);
  SDL_RenderCopy(renderer, target_texture, &target_rectangle, &target_rectangle);

  SDL_Rect rectangle = {
    .x = element->layout.x,
    .y = element->layout.y,
    .w = element->layout.max_width,
    .h = element->layout.max_height,
  };
  // Invalid shape
  if (rectangle.w == 0 || rectangle.h == 0) {
    return;
  };

  BorderSize border_size = {
    .top = element->border.top,
    .right = element->border.right,
    .bottom = element->border.bottom,
    .left = element->border.left
  };
  i32 border_width = element->border.top;
  if (element->background_type == background_type.none) {
    if (element->border_radius > 0 && element->border.top == 1) {
      draw_rounded_rectangle(renderer, rectangle, element->border_radius, element->border_color);
    }
  } else if (element->background_type == background_type.color) {
    if (element->border_radius > 0) {
      if (border_width > 0) {
        draw_rounded_rectangle_with_border(renderer, rectangle, element->border_radius, border_width, element->border_color, element->background_color);
      } else {
        draw_filled_rounded_rectangle(renderer, rectangle, element->border_radius, element->background_color);
      }
    } else {
      draw_filled_rectangle(renderer, rectangle, element->background_color);
      draw_border(renderer, rectangle, border_size, element->border_color);
    }
  } else if (element->background_type == background_type.horizontal_gradient) {
    draw_horizontal_gradient(renderer, rectangle, element->background_gradient);
    draw_border(renderer, rectangle, border_size, element->border_color);
  } else if (element->background_type == background_type.vertical_gradient) {
    draw_vertical_gradient(renderer, rectangle, element->background_gradient);
    draw_border(renderer, rectangle, border_size, element->border_color);
  }
  if (element->text.length > 0) {
    i32 text_x = rectangle.x + element->padding.left;
    i32 text_y = rectangle.y + element->padding.top;
    draw_text(
      renderer, font, to_char(element->text), text_x, text_y, element->text_color
    );
  } else if (element->input != 0) {
    // If the element is the active element we should also draw the cursor
    if (element == active_element) {
      SDL_Rect selection_rect = measure_selection(font, element->input);
      SDL_Rect selection = {
        .x = rectangle.x + element->padding.left + selection_rect.x - 1, // Subtract 1 pixel for the cursor
        .y = rectangle.y + element->padding.top + selection_rect.y,
        .w = selection_rect.w + 2, // Add 2 pixels for the cursor
        .h = rectangle.h - element->padding.top - element->padding.bottom,
      };
      draw_filled_rectangle(renderer, selection, border_color_active);
    }
    InputData input = *element->input;
    i32 text_x = rectangle.x + element->padding.left;
    i32 text_y = rectangle.y + element->padding.top;
    char *text_data = (char *)input.text.data;

    draw_text(
      renderer, font, text_data, text_x, text_y, element->text_color
    );
  }
  // Reset the target texture as the render target
  SDL_SetRenderTarget(renderer, target_texture);
  // Copy a portion of the element texture to the same location on the target texture
  SDL_RenderCopy(renderer, element_texture, &target_rectangle, &target_rectangle);
  if (element_texture) {
    SDL_DestroyTexture(element_texture);
  }

  Array *children = element->children;
  if (children == 0) return;
  // Cap the child rectangle to the target rectangle
  SDL_Rect child_rectangle = {
    .x = rectangle.x > target_rectangle.x ? rectangle.x : target_rectangle.x,
    .y = rectangle.y > target_rectangle.y ? rectangle.y : target_rectangle.y,
    .w = rectangle.w < target_rectangle.w ? rectangle.w : target_rectangle.w,
    .h = rectangle.h < target_rectangle.h ? rectangle.h : target_rectangle.h
  };
  for (size_t i = 0; i < array_length(children); i++) {
    Element *child = array_get(children, i);
    draw_elements(renderer, font, child, child_rectangle, active_element);
  }
}

void render_element_tree(SDL_Renderer *renderer, TTF_Font *font, ElementTree *element_tree) {
  // Create a rectangle that covers the entire target texture
  SDL_Texture *target_texture = SDL_GetRenderTarget(renderer);
  SDL_Rect target_rectangle = {0, 0, 0, 0};
  // Get the width and height of the target texture
  SDL_QueryTexture(target_texture, NULL, NULL, &target_rectangle.w, &target_rectangle.h);

  draw_elements(renderer, font, element_tree->root, target_rectangle, element_tree->active_element);
}

#define C9_RENDERER
#endif