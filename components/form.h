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

void click_content(ElementTree *tree, void *data) {
  (void)data;
  Element *content_panel = get_element_by_tag(tree->root, content_panel_tag);
  Element *panel_top_content = tree->active_element;
  if (panel_top_content != 0 && content_panel != 0) {
    panel_top_content->background_color = gray_1;
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
    .background_color = white,
    .layout_direction = layout_direction.vertical,
    .overflow = overflow_type.scroll_y,
    .padding = (Padding){10, 10, 10, 10},
    .gutter = 10,
  };

  Element *content_panel_top = add_new_element(arena, form_element);
  *content_panel_top = (Element){
    .background_type = background_type.color,
    .background_color = gray_2,
    .corner_radius = 15,
    .padding = (Padding){10, 10, 10, 10},
  };

  Element *content_panel_bottom = add_new_element(arena, form_element);
  *content_panel_bottom = (Element){
    .background_type = background_type.color,
    .background_color = gray_2,
    .corner_radius = 15,
    .padding = (Padding){10, 10, 10, 10},
    .gutter = 10,
  };

  Element *color_change_bar = add_new_element(arena, content_panel_bottom);
  *color_change_bar = (Element){
    .width = 100,
    .height = 600,
    .background_type = background_type.color,
    .background_color = white,
    .corner_radius = 15,
    .on_click = &click_content,
  };

  Element *open_overlay_button = add_new_element(arena, content_panel_bottom);
  *open_overlay_button = (Element){
    .height = 30,
    .text = to_s8("Open overlay"),
    .text_color = white,
    .text_align = text_align.center,
    .padding = (Padding){5, 10, 5, 10},
    .background_type = background_type.horizontal_gradient,
    .background_gradient = button_gradient,
    .corner_radius = 15,
    .overflow = overflow_type.scroll_x,
    .on_click = &click_overlay_button,
  };

  Element *text_input = add_new_element(arena, content_panel_top);
  *text_input = (Element){
    .height = 30,
    .background_type = background_type.color,
    .background_color = white,
    .padding = (Padding){5, 10, 5, 10},
    .corner_radius = 15,
    .border_color = border_color,
    .border = (Border){1, 1, 1, 1},
    .input = new_input(arena),
    .text_color = text_color,
    .on_click = &click_text_input,
    .on_blur = &blur_text_input,
    .on_key_press = &on_text_input,
  };
}

#define FORM_COMPONENT
#endif