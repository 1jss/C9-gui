#ifndef OVERLAY_COMPONENT

#include "../constants/color_theme.h" // white
#include "../include/arena.h" // Arena
#include "../include/element_tree.h" // Element, add_new_element, new_element, background_type, layout_direction, Padding, ElementTree
#include "../include/layout.h" // set_overlay_dimensions

Element *overlay_element = 0;

void close_overlay(ElementTree *tree, void *data) {
  (void)data;
  tree->overlay = 0;
  tree->rerender = rerender_type.all;
}

void create_overlay_element(Arena *arena) {
  overlay_element = new_element(arena);
  *overlay_element = (Element){
    .background_type = background_type.color,
    .background.color = 0x00000080,
    .layout_direction = layout_direction.vertical,
    .overflow = overflow_type.scroll,
    .padding = (Padding){20, 20, 20, 20},
    .gutter = 10,
  };

  Element *card_element = add_new_element(arena, overlay_element);
  *card_element = (Element){
    .background_type = background_type.color,
    .background.color = white,
    .corner_radius = 35,
    .padding = (Padding){20, 20, 20, 20},
    .gutter = 20,
    .layout_direction = layout_direction.vertical,
  };

  Element *title_element = add_new_element(arena, card_element);
  *title_element = (Element){
    .text = to_s8("Overlay Title"),
    .text_color = text_color,
  };

  Element *hr_element = add_new_element(arena, card_element);
  *hr_element = (Element){
    .height = 1,
    .background_type = background_type.color,
    .background.color = border_color,
  };

  Element *content_element = add_new_element(arena, card_element);
  *content_element = (Element){
    .text = to_s8("Some explaining text"),
    .text_color = text_color,
  };

  Element *close_button = add_new_element(arena, card_element);
  *close_button = (Element){
    .height = 30,
    .background_type = background_type.horizontal_gradient,
    .background.gradient = button_gradient,
    .padding = (Padding){5, 10, 5, 10},
    .corner_radius = 15,
    .text = to_s8("Close overlay"),
    .text_color = white,
    .text_align = text_align.center,
    .on_click = &close_overlay,
  };
}

void open_overlay(ElementTree *tree) {
  if (overlay_element == 0) {
    create_overlay_element(tree->arena);
  }
  set_root_element_dimensions(overlay_element, tree->root->layout.max_width, tree->root->layout.max_height);
  tree->overlay = overlay_element;
  tree->rerender = rerender_type.all;
}

#define OVERLAY_COMPONENT
#endif