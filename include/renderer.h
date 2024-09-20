#ifndef C9_RENDERER

#include <SDL2/SDL.h> // SDL_rect, SDL_Texture
#include "SDL_ttf.h" // TTF_Font, TTF_SizeUTF8
#include "arena.h" // Arena, arena_fill
#include "array.h" // array_get
#include "draw_shapes.h" // draw_filled_rectangle, draw_horizontal_gradient_rectangle, draw_vertical_gradient_rectangle, draw_rectangle_with_border, draw_rectangle, has_border
#include "element_tree.h" // Element, ElementTree
#include "font.h" // get_font
#include "font_layout.h" // get_text_line_height, split_string_by_width
#include "input.h" // InputData
#include "input_actions.h" // measure_selection
#include "types.h" // i32

// Recursively draws all elements
void draw_elements(SDL_Renderer *renderer, Element *element, SDL_Rect target_rect, Element *active_element, SDL_Rect window_rect) {
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

  // Lazy load elements outside of the window
  if (element->render.texture == 0 && (element_rect.x > window_rect.w || element_rect.y > window_rect.h)) {
    return;
  }

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

  // Rectangle that covers the visible part of the element texture
  SDL_Rect element_texture_cutout_rect = {
    .x = texture_left_edge,
    .y = texture_top_edge,
    .w = target_right_edge - target_left_edge,
    .h = target_bottom_edge - target_top_edge,
  };
  // Rectangle that positions the element texture in the target texture
  SDL_Rect target_texture_cutout_rect = {
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
      element->render.changed == false &&
      element->render.width == element_texture_rect.w &&
      element->render.height == element_texture_rect.h) {
    // Copy a portion of the element texture to the same location on the target texture
    SDL_RenderCopy(renderer, element->render.texture, &element_texture_cutout_rect, &target_texture_cutout_rect);
  } else {
    // If the element has a cached texture but its dimensions are not correct we need to destroy it
    if (element->render.texture != 0 && (element->render.width != element_texture_rect.w || element->render.height != element_texture_rect.h)) {
      SDL_DestroyTexture(element->render.texture);
      element->render.texture = 0;
    }
    // If the element doesn't have a cached texture we need to create one
    if (element->render.texture == 0) {
      element->render.texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, element_texture_rect.w, element_texture_rect.h);
      if (element->render.texture == 0) {
        printf("Failed to create texture: %s\n", SDL_GetError());
        return;
      }
      SDL_SetTextureBlendMode(element->render.texture, SDL_BLENDMODE_BLEND);
      element->render.width = element_texture_rect.w;
      element->render.height = element_texture_rect.h;
    }

    // Lock element texture for direct pixel access
    PixelData locked_element = {0};
    void *void_pixels = 0;
    i32 bytes_width = 0; // Row width in bytes
    if (SDL_LockTexture(element->render.texture, NULL, &void_pixels, &bytes_width)) {
      printf("Failed to lock element texture: %s\n", SDL_GetError());
      return;
    }
    locked_element.pixels = (RGBA *)void_pixels;
    locked_element.width = bytes_width / sizeof(RGBA); // Row width in pixels
    locked_element.height = element_texture_rect.h;

    // Empty the element texture from any previous data
    for (i32 y = 0; y < element_texture_rect.h; y++) {
      for (i32 x = 0; x < element_texture_rect.w; x++) {
        locked_element.pixels[y * locked_element.width + x] = 0;
      }
    }

    if (element->background_type == background_type.color) {
      if (has_border(element->border)) {
        draw_rectangle_with_border(locked_element, element_texture_rect, element->corner_radius, element->border, element->border_color, element->background.color);
      } else {
        draw_filled_rectangle(locked_element, element_texture_rect, element->corner_radius, element->background.color);
      }
    } else if (element->background_type == background_type.horizontal_gradient) {
      if (has_border(element->border)) {
        draw_horizontal_gradient_rectangle_with_border(locked_element, element_texture_rect, element->corner_radius, element->border, element->border_color, element->background.gradient);
      } else {
        draw_horizontal_gradient_rectangle(locked_element, element_texture_rect, element->corner_radius, element->background.gradient);
      }
    } else if (element->background_type == background_type.vertical_gradient) {
      if (has_border(element->border)) {
        draw_vertical_gradient_rectangle_with_border(locked_element, element_texture_rect, element->corner_radius, element->border, element->border_color, element->background.gradient);
      } else {
        draw_vertical_gradient_rectangle(locked_element, element_texture_rect, element->corner_radius, element->background.gradient);
      }
    } else if (element->background_type == background_type.image) {
      if (element->background.image.length > 0) {
        draw_image(locked_element, to_char(element->background.image), element_texture_rect);
      }
    } else if (element->background_type == background_type.none && has_border(element->border)) {
      draw_rectangle_with_border(locked_element, element_texture_rect, element->corner_radius, element->border, element->border_color, 0);
    }
    if (element->text.length > 0) {
      TTF_Font *font = get_font(element->font_variant);
      SDL_Rect text_position = {
        .x = element->padding.left + element->layout.scroll_x,
        .y = element->padding.top + element->layout.scroll_y,
        .w = element_texture_rect.w - element->padding.left - element->padding.right,
        .h = element_texture_rect.h - element->padding.top - element->padding.bottom,
      };
      // Apply text alignment
      if (element->text_align != text_align.start) {
        i32 text_width = 0;
        TTF_SizeUTF8(font, to_char(element->text), &text_width, 0);
        i32 extra_space = text_position.w - text_width;
        if (extra_space > 0 && element->text_align == text_align.center) {
          text_position.x += extra_space / 2;
        } else if (extra_space > 0 && element->text_align == text_align.end) {
          text_position.x += extra_space;
        }
      }
      if (element->overflow == overflow_type.scroll || element->overflow == overflow_type.scroll_x) {
        draw_text(locked_element, font, to_char(element->text), element->text_color, text_position, element->padding);
      } else {
        draw_multiline_text(locked_element, element->font_variant, element->text, element->text_color, text_position, element->padding);
      }
    } else if (element->input != 0) {
      TTF_Font *font = get_font(font_variant.regular);
      SDL_Rect text_position = {
        .x = element_texture_rect.x + element->padding.left + element->layout.scroll_x,
        .y = element_texture_rect.y + element->padding.top + element->layout.scroll_y,
        .w = element_texture_rect.w - element->padding.left - element->padding.right,
        .h = element_texture_rect.h - element->padding.top - element->padding.bottom,
      };
      // If the element is the active element we should also draw the cursor
      if (element == active_element) {
        // Text is scrolling horizontally
        if (element->overflow == overflow_type.scroll || element->overflow == overflow_type.scroll_x) {
          SDL_Rect selection_rect = measure_selection(font, element->input);
          SDL_Rect selection = {
            .x = text_position.x + selection_rect.x - 1, // Subtract 1 pixel for the cursor
            .y = text_position.y + selection_rect.y,
            .w = selection_rect.w + 2, // Add 2 pixels for the cursor
            .h = get_font_height(element->font_variant),
          };
          // Make sure selection is not drawn outside text bounds
          i32 text_limit_left = element_texture_rect.x + element->padding.left - 1;
          i32 text_limit_right = element_texture_rect.x + element_texture_rect.w - element->padding.right + 1;
          // Left bound
          if (selection.x < text_limit_left) {
            selection.w = selection.w - (text_limit_left - selection.x);
            selection.x = text_limit_left;
          }
          // Right bound
          if (selection.x + selection.w >= text_limit_right) {
            selection.w = text_limit_right - selection.x;
          }
          if (selection.x < text_limit_right) {
            if (selection_rect.w == 0) {
              draw_filled_rectangle(locked_element, selection, 0, text_cursor_color);
            } else {
              draw_filled_rectangle(locked_element, selection, 0, selection_color);
            }
          }
        }
        // Text is wrapping and growing vertically
        else {
          Arena *temp_arena = arena_open(512);
          i32 start_index = get_start_value(element->input->selection);
          i32 end_index = get_end_value(element->input->selection);
          Array *text = split_string_by_width(temp_arena, element->font_variant, element->input->text, text_position.w);
          i32 index = 0;
          i32 line_height = get_text_line_height(element->font_variant);
          SDL_Rect selection = {0};
          // Step over rows and draw a rectangle between selected indexes
          for (i32 i = 0; i < array_length(text); i++) {
            s8 *line = array_get(text, i);
            // If the selection has started at the end of the line and the line ends on or after the start index
            // Or if the selection starts on the end of the line and this is the last line
            if ((start_index < index + line->length && end_index >= index) ||
                (start_index == index + line->length && i == array_length(text) - 1)) {
              // Selection spans the entire line
              if (start_index <= index && end_index >= index + line->length) {
                i32 line_length = line->length;
                i32 text_width = 0;
                // All but the last (empty) line
                if (line_length > 0) {
                  // Remove newline character from the end of the line
                  if (line->data[line_length - 1] == '\n') {
                    line_length--;
                  }
                  // Create a new string from line to add null terminator
                  s8 trimmed_string = string_from_substring(temp_arena, line->data, 0, line_length);
                  // Measure text width
                  TTF_SizeUTF8(font, to_char(trimmed_string), &text_width, 0);
                }

                selection = (SDL_Rect){
                  .x = text_position.x - 1, // Subtract 1 pixel for the cursor
                  .y = text_position.y + line_height * i,
                  .w = text_width + 2,
                  .h = get_font_height(element->font_variant),
                };
              }
              // Selection spans only part of the line
              else {
                i32 relative_start = start_index - index;
                if (start_index < index) {
                  relative_start = 0;
                };
                i32 relative_end = end_index - index;
                if (end_index >= index + line->length) {
                  relative_end = line->length;
                }
                if (line->length > 0 && relative_end > relative_start && line->data[relative_end - 1] == '\n') {
                  relative_end--;
                }

                // Measure text from 0 to relative_end
                i32 selection_end_width = 0;
                s8 selection_end = string_from_substring(temp_arena, line->data, 0, relative_end);
                TTF_SizeUTF8(font, to_char(selection_end), &selection_end_width, 0);
                // Meassure text from 0 to relative_start
                i32 selection_start_width = 0;
                if (relative_start > 0) {
                  s8 selection_start = string_from_substring(temp_arena, line->data, 0, relative_start);
                  TTF_SizeUTF8(font, to_char(selection_start), &selection_start_width, 0);
                }

                selection = (SDL_Rect){
                  .x = text_position.x + selection_start_width - 1, // Subtract 1 pixel for the cursor
                  .y = text_position.y + line_height * i,
                  .w = selection_end_width - selection_start_width + 2, // Add 2 pixels for the cursor
                  .h = get_font_height(element->font_variant),
                };
              }
              if (start_index == end_index) {
                draw_filled_rectangle(locked_element, selection, 0, text_cursor_color);
              } else {
                draw_filled_rectangle(locked_element, selection, 0, selection_color);
              }
            }
            index += line->length;
          }
          arena_close(temp_arena);
        }
      }
      InputData input = *element->input;
      if (element->overflow == overflow_type.scroll || element->overflow == overflow_type.scroll_x) {
        char *text_data = (char *)input.text.data;
        draw_text(locked_element, font, text_data, element->text_color, text_position, element->padding);
      } else {
        draw_multiline_text(locked_element, element->font_variant, input.text, element->text_color, text_position, element->padding);
      }
    }

    // Unlock element texture to make it readable again
    SDL_UnlockTexture(element->render.texture);

    // Copy a portion of the element texture to the same location on the target texture
    SDL_RenderCopy(renderer, element->render.texture, &element_texture_cutout_rect, &target_texture_cutout_rect);
    // Set the element as unchanged
    element->render.changed = false;
  }

  Array *children = element->children;
  if (children == 0) return;

  for (i32 i = 0; i < array_length(children); i++) {
    Element *child = array_get(children, i);
    draw_elements(renderer, child, target_texture_cutout_rect, active_element, window_rect);
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
    renderer_fill_rectangle(renderer, scrollbar_rect, scrollbar_color);
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
    renderer_fill_rectangle(renderer, scrollbar_rect, scrollbar_color);
  }
}

void render_element_tree(SDL_Renderer *renderer, ElementTree *tree) {
  // Clear buffer
  SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
  SDL_RenderClear(renderer);
  // Create a rectangle that covers the entire target texture
  SDL_Rect target_rectangle = {0, 0, 0, 0};
  // Get the width and height of the target texture
  SDL_QueryTexture(tree->target_texture, NULL, NULL, &target_rectangle.w, &target_rectangle.h);
  // Draw the root element
  draw_elements(renderer, tree->root, target_rectangle, tree->active_element, target_rectangle);
  if (tree->overlay != 0) {
    // Draw the overlay element
    draw_elements(renderer, tree->overlay, target_rectangle, tree->active_element, target_rectangle);
  }
}

#define C9_RENDERER
#endif