#ifndef SEARCH_BAR_COMPONENT

#include "../constants/color_theme.h" // gray_2, white
#include "../constants/element_tags.h" // search_panel_tag
#include "../include/arena.h" // Arena
#include "../include/layout.h" // Element, add_new_element, new_element, overflow_type, background_type, layout_direction, Padding
#include "../include/string.h" // to_s8

Element *search_bar = 0;

void set_active_input_style(Element *element) {
  element->border_color = border_color_active;
  element->text_color = text_color_active;
}

void set_passive_input_style(Element *element) {
  element->border_color = border_color;
  element->text_color = text_color;
}

void click_search_bar(ElementTree *tree) {
  set_active_input_style(tree->active_element);
  Element *panel = select_element_by_tag(tree->root, search_panel_tag);
  if (panel != 0) {
    bump_rerender(tree);
    tree->rerender_element = panel;
  }
}

void blur_search_bar(ElementTree *tree) {
  set_passive_input_style(tree->active_element);
  Element *panel = select_element_by_tag(tree->root, search_panel_tag);
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
    .border_radius = 15,
    .border_color = border_color,
    .border = (Border){1, 1, 1, 1},
    .text = to_s8("Search"),
    .text_color = text_color,
    .on_click = &click_search_bar,
    .on_blur = &blur_search_bar,
  };
}

#define SEARCH_BAR_COMPONENT
#endif
