#ifndef SEARCH_BAR_COMPONENT

#include "../constants/color_theme.h" // white, border_color
#include "../include/arena.h" // Arena
#include "../include/element_tree.h" // Element, ElementTree, new_element, overflow_type, background_type, Padding
#include "../include/string.h" // to_s8
#include "search_overlay.h" // open_serach_overlay

Element *search_bar = 0;

void click_open_overlay(ElementTree *tree, void *data) {
  (void)data;
  open_search_overlay(tree);
}

void create_search_bar_element(Arena *arena) {
  search_bar = new_element(arena);
  *search_bar = (Element){
    .min_width = 100,
    .background_type = background_type.color,
    .background.color = white,
    .padding = (Padding){6, 10, 6, 10},
    .corner_radius = 15,
    .border_color = border_color,
    .border = (Border){1, 1, 1, 1},
    .text = to_s8("Search..."),
    .text_color = text_color_muted,
    .on_click = &click_open_overlay,
    .overflow = overflow_type.scroll_x
  };
}

#define SEARCH_BAR_COMPONENT
#endif
