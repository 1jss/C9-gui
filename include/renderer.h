#ifndef C9_RENDERER

#include <SDL2/SDL.h>
#include <stdio.h>
#include "array.h" // array_get
#include "draw_shapes.h" // draw_filled_rectangle, draw_horizontal_gradient, draw_rectangle_with_border, draw_filled_rounded_rectangle, draw_superellipse, draw_filled_superellipse
#include "layout.h" // Element, ElementTree
#include "types.h" // i32

// Recursively draws all elements
void draw_elements(SDL_Renderer *renderer, Element *element) {
  i32 x = element->layout.x;
  i32 y = element->layout.y;
  i32 width = element->width;
  i32 height = element->height;
  if (element->element_sizing != element_sizing.fixed) {
    width = element->layout.scroll_width;
    height = element->layout.scroll_height;
  }
  if (element->background_type == background_type.none) {
    if (element->border_radius > 0) {
      draw_rounded_rectangle(renderer, x, y, width, height, element->border_radius, element->border_color);
    }
  } else if (element->background_type == background_type.color) {
    if (element->border_radius > 0) {
      if (element->border.top == 1 && element->border.right == 1 && element->border.bottom == 1 && element->border.left == 1) {
        draw_rectangle_with_border(renderer, x, y, width, height, element->border_radius, 1, element->border_color, element->background_color);
      } else {
        draw_filled_rounded_rectangle(renderer, x, y, width, height, element->border_radius, element->background_color);
      }
    } else {
      draw_filled_rectangle(renderer, x, y, width, height, element->background_color);
      draw_border(renderer, x, y, width, height, element->border.top, element->border.right, element->border.bottom, element->border.left, element->border_color);
    }
  } else if (element->background_type == background_type.horizontal_gradient) {
    draw_horizontal_gradient(renderer, x, y, width, height, element->background_gradient);
    draw_border(renderer, x, y, width, height, element->border.top, element->border.right, element->border.bottom, element->border.left, element->border_color);
  } else if (element->background_type == background_type.vertical_gradient) {
    draw_vertical_gradient(renderer, x, y, width, height, element->background_gradient);
    draw_border(renderer, x, y, width, height, element->border.top, element->border.right, element->border.bottom, element->border.left, element->border_color);
  }
  Array *children = element->children;
  if (children == 0) return;
  for (size_t i = 0; i < array_length(children); i++) {
    Element *child = array_get(children, i);
    draw_elements(renderer, child);
  }
}

void render_element_tree(SDL_Renderer *renderer, ElementTree *element_tree) {
  printf("Rendering element tree\n");
  draw_elements(renderer, element_tree->root);
}

#define C9_RENDERER
#endif