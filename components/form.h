#ifndef FORM_COMPONENT

#include "../constants/color_theme.h" // gray_2, white
#include "../constants/element_tags.h" // content_panel_tag
#include "../helpers/style_helpers.h" // set_active_input_style, set_passive_input_style
#include "../include/arena.h" // Arena
#include "../include/element_tree.h" // Element, add_new_element, new_element, overflow_type, background_type, layout_direction, Padding, ElementTree, get_element_by_tag
#include "../include/input.h"
#include "../include/renderer.h" // bump_rerender
#include "overlay.h" // open_overlay

Element *form_element = 0;

void click_text_input(ElementTree *tree, void *data) {
  (void)data;
  set_active_input_style(tree->active_element);
  Element *content_panel = get_element_by_tag(tree->root, content_panel_tag);
  if (content_panel != 0) {
    bump_rerender(tree);
    tree->rerender_element = content_panel;
  }
}

void blur_text_input(ElementTree *tree, void *data) {
  (void)data;
  set_passive_input_style(tree->active_element);
  Element *content_panel = get_element_by_tag(tree->root, content_panel_tag);
  if (content_panel != 0) {
    bump_rerender(tree);
    tree->rerender_element = content_panel;
  }
}

void on_text_input(ElementTree *tree, void *data) {
  char *text = (char *)data;
  handle_text_input(tree->active_element->input, text);
  Element *content_panel = get_element_by_tag(tree->root, content_panel_tag);
  if (content_panel != 0) {
    bump_rerender(tree);
    tree->rerender_element = content_panel;
  }
}

void click_overlay_button(ElementTree *tree, void *data) {
  (void)data;
  open_overlay(tree);
}

void create_form_element(Arena *arena) {
  form_element = new_element(arena);
  *form_element = (Element){
    .background_type = background_type.color,
    .background.color = white,
    .layout_direction = layout_direction.vertical,
    .overflow = overflow_type.scroll_y,
    .padding = (Padding){10, 10, 10, 10},
    .gutter = 10,
  };

  Element *content_panel_top = add_new_element(arena, form_element);
  *content_panel_top = (Element){
    .background_type = background_type.color,
    .background.color = gray_2,
    .corner_radius = 25,
    .padding = (Padding){10, 10, 10, 10},
  };

  Element *text_input = add_new_element(arena, content_panel_top);
  *text_input = (Element){
    .height = 30,
    .background_type = background_type.color,
    .background.color = white,
    .padding = (Padding){6, 10, 6, 10},
    .corner_radius = 15,
    .border_color = border_color,
    .border = (Border){1, 1, 1, 1},
    .input = new_input(arena),
    .text_color = text_color,
    .on_click = &click_text_input,
    .on_blur = &blur_text_input,
    .on_key_press = &on_text_input,
  };

  Element *content_panel_bottom = add_new_element(arena, form_element);
  *content_panel_bottom = (Element){
    .background_type = background_type.color,
    .background.color = gray_2,
    .corner_radius = 25,
    .padding = (Padding){10, 10, 10, 10},
    .gutter = 10,
    .layout_direction = layout_direction.vertical,
    .overflow = overflow_type.scroll_y,
  };

  Element *open_overlay_button = add_new_element(arena, content_panel_bottom);
  *open_overlay_button = (Element){
    .text = to_s8("Open overlay"),
    .text_color = white,
    .text_align = text_align.center,
    .padding = (Padding){5, 10, 5, 10},
    .background_type = background_type.horizontal_gradient,
    .background.gradient = button_gradient,
    .corner_radius = 15,
    .overflow = overflow_type.scroll_x,
    .on_click = &click_overlay_button,
    .font_variant = font_variant.bold,
  };

}

#define FORM_COMPONENT
#endif