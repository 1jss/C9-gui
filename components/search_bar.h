#ifndef SEARCH_BAR_COMPONENT

#include "../constants/color_theme.h" // gray_2, white
#include "../constants/element_tags.h" // search_panel_tag
#include "../helpers/style_helpers.h" // set_active_input_style, set_passive_input_style
#include "../include/arena.h" // Arena
#include "../include/element_tree.h" // Element, ElementTree, add_new_element, new_element, overflow_type, background_type, layout_direction, Padding, get_element_by_tag
#include "../include/renderer.h" // bump_rerender
#include "../include/string.h" // to_s8

Element *search_bar = 0;

void click_search_bar(ElementTree *tree, void *data) {
  (void)data;
  set_active_input_style(tree->active_element);
  Element *search_panel = get_element_by_tag(tree->root, search_panel_tag);
  if (search_panel != 0) {
    bump_rerender(tree);
    tree->rerender_element = search_panel;
  }
}

void blur_search_bar(ElementTree *tree, void *data) {
  (void)data;
  set_passive_input_style(tree->active_element);
  Element *panel = get_element_by_tag(tree->root, search_panel_tag);
  if (panel != 0) {
    bump_rerender(tree);
    tree->rerender_element = panel;
  }
}

void on_search_bar_input(ElementTree *tree, void *data) {
  char *text = (char *)data;
  handle_text_input(tree->active_element->input, text);
  Element *panel = get_element_by_tag(tree->root, search_panel_tag);
  if (panel != 0) {
    bump_rerender(tree);
    tree->rerender_element = panel;
  }
}

void create_search_bar_element(Arena *arena) {
  search_bar = new_element(arena);
  *search_bar = (Element){
    .min_width = 100,
    .background_type = background_type.color,
    .background_color = white,
    .padding = (Padding){5, 10, 5, 10},
    .corner_radius = 15,
    .border_color = border_color,
    .border = (Border){1, 1, 1, 1},
    .input = new_input(arena),
    .text_color = text_color,
    .on_click = &click_search_bar,
    .on_blur = &blur_search_bar,
    .on_key_press = &on_search_bar_input,
    .overflow = overflow_type.scroll_x
  };
}

#define SEARCH_BAR_COMPONENT
#endif
