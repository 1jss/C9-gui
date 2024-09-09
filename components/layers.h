#ifndef LAYERS_COMPONENT

#include "../constants/color_theme.h" // gray_1, white
#include "../include/arena.h" // Arena
#include "../include/element_tree.h" // Element, add_new_element, new_element, overflow_type, background_type, layout_direction, Padding, ElementTree
#include "../include/font.h" // font_variant
#include "overlay.h" // open_overlay

Element *layers_element = 0;

void click_overlay_button(ElementTree *tree, void *data) {
  (void)data;
  open_overlay(tree);
}

void create_layers_element(Arena *arena) {
  layers_element = new_element(arena);
  *layers_element = (Element){
    .background_type = background_type.color,
    .background.color = white,
    .layout_direction = layout_direction.vertical,
    .overflow = overflow_type.scroll_y,
    .padding = (Padding){10, 10, 10, 10},
    .gutter = 10,
  };

  Element *content_panel = add_new_element(arena, layers_element);
  *content_panel = (Element){
    .background_type = background_type.color,
    .background.color = gray_1,
    .corner_radius = 25,
    .padding = (Padding){10, 10, 10, 10},
    .gutter = 10,
    .layout_direction = layout_direction.vertical,
    .overflow = overflow_type.scroll_y,
    .border = (Border){1, 1, 1, 1},
    .border_color = gray_2,
  };

  Element *text_panel = add_new_element(arena, content_panel);
  *text_panel = (Element){
    .background_type = background_type.color,
    .background.color = white,
    .padding = (Padding){6, 10, 6, 10},
    .layout_direction = layout_direction.vertical,
    .corner_radius = 15,
    .gutter = 6,
    .border = (Border){1, 1, 1, 1},
    .border_color = gray_2,
  };

  Element *layers_title = add_new_element(arena, text_panel);
  *layers_title = (Element){
    .text = to_s8("Layers"),
    .text_color = text_color,
    .overflow = overflow_type.scroll_x,
    .font_variant = font_variant.large,
  };

  Element *layers_description_1 = add_new_element(arena, text_panel);
  *layers_description_1 = (Element){
    .text = to_s8("C9 gui uses two rendering layers, one for the normal content and one for overlays."),
    .text_color = text_color,
  };

  Element *layers_description_2 = add_new_element(arena, text_panel);
  *layers_description_2 = (Element){
    .text = to_s8("The overlay layer is rendered on top of the content layer and is used for modals, popups, dropdowns, etc."),
    .text_color = text_color,
  };

  Element *open_overlay_button = add_new_element(arena, content_panel);
  *open_overlay_button = (Element){
    .text = to_s8("Open overlay"),
    .text_color = white,
    .text_align = text_align.center,
    .padding = (Padding){6, 10, 6, 10},
    .background_type = background_type.horizontal_gradient,
    .background.gradient = button_gradient,
    .corner_radius = 15,
    .overflow = overflow_type.scroll_x,
    .on_click = &click_overlay_button,
    .font_variant = font_variant.bold,
  };
}

#define LAYERS_COMPONENT
#endif