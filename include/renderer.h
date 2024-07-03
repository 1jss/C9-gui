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
  draw_rounded_rectangle(renderer, x, y, width, height, element->border.radius, element->border.color);
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