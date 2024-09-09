#ifndef SEARCH_OVERLAY_COMPONENT

#include "../constants/color_theme.h" // white
#include "../constants/element_tags.h" // search_panel_input_tag
#include "../helpers/style_helpers.h" // set_active_input_style, set_passive_input_style
#include "../include/arena.h" // Arena
#include "../include/element_tree.h" // Element, add_new_element, new_element, background_type, layout_direction, Padding, ElementTree
#include "../include/font.h" // font_variant
#include "../include/input.h" // clear_input
#include "../include/layout.h" // set_overlay_dimensions

Element *search_overlay_element = 0;

void close_search_overlay(ElementTree *tree, void *data) {
  (void)data;
  tree->overlay = 0;
  tree->active_element = 0;
  tree->rerender = rerender_type.all;
}

void click_search_bar(ElementTree *tree, void *data) {
  (void)data;
  set_active_input_style(tree->active_element);
  Element *input_panel = get_parent(tree->overlay, tree->active_element);
  if (input_panel != 0) {
    bump_rerender(tree);
    tree->rerender_element = input_panel;
  }
}

void blur_search_bar(ElementTree *tree, void *data) {
  (void)data;
  set_passive_input_style(tree->active_element);
  Element *input_panel = get_parent(tree->overlay, tree->active_element);
  if (input_panel != 0) {
    bump_rerender(tree);
    tree->rerender_element = input_panel;
  }
}

void on_search_bar_input(ElementTree *tree, void *data) {
  char *text = (char *)data;
  if (strcmp(text, "ESCAPE") == 0) {
    close_search_overlay(tree, 0);
  }
}

void create_search_overlay_element(Arena *arena) {
  search_overlay_element = new_element(arena);

  Element *sidebar_shade = add_new_element(arena, search_overlay_element);
  *sidebar_shade = (Element){
    .width = 200,
    .background_type = background_type.color,
    .background.color = 0x00000080,
    .on_click = &close_search_overlay,
  };

  Element *content_panel = add_new_element(arena, search_overlay_element);
  *content_panel = (Element){
    .background_type = background_type.color,
    .background.color = white,
    .layout_direction = layout_direction.vertical,
  };

  Element *input_panel = add_new_element(arena, content_panel);
  *input_panel = (Element){
    .height = 50,
    .background_type = background_type.color,
    .background.color = white,
    .padding = (Padding){10, 10, 10, 10},
    .border_color = border_color,
    .border = (Border){0, 0, 1, 0},
  };

  Element *search_input = add_new_element(arena, input_panel);
  *search_input = (Element){
    .element_tag = search_panel_input_tag,
    .min_width = 100,
    .background_type = background_type.color,
    .background.color = white,
    .padding = (Padding){6, 10, 6, 10},
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

  Element *result_panel = add_new_element(arena, content_panel);
  *result_panel = (Element){
    .background_type = background_type.color,
    .background.color = gray_1,
    .layout_direction = layout_direction.vertical,
    .overflow = overflow_type.scroll_y,
    .padding = (Padding){10, 10, 10, 10},
    .gutter = 10,
  };

  Element *search_result_list = add_new_element(arena, result_panel);
  *search_result_list = (Element){
    .background_type = background_type.color,
    .background.color = white,
    .border = (Border){1, 1, 1, 1},
    .border_color = gray_2,
    .corner_radius = 15,
    .padding = (Padding){2, 0, 2, 0},
    .layout_direction = layout_direction.vertical,
  };

  Element *search_result_item = add_new_element(arena, search_result_list);
  *search_result_item = (Element){
    .padding = (Padding){8, 10, 8, 10},
    .text = to_s8("Search result item"),
    .text_color = text_color,
  };

  Element *search_result_separator = add_new_element(arena, search_result_list);
  *search_result_separator = (Element){
    .height = 1,
    .background_type = background_type.color,
    .background.color = gray_2,
  };

  Element *search_result_item_2 = add_new_element(arena, search_result_list);
  *search_result_item_2 = (Element){
    .padding = (Padding){8, 10, 8, 10},
    .text = to_s8("Search result item 2"),
    .text_color = text_color,
  };
}

void open_search_overlay(ElementTree *tree) {
  if (search_overlay_element == 0) {
    create_search_overlay_element(tree->arena);
  }
  // Set input focus
  tree->active_element = get_element_by_tag(search_overlay_element, search_panel_input_tag);
  // Clear input value
  clear_input(tree->active_element->input);
  set_active_input_style(tree->active_element);
  tree->active_element->render.changed = 1;
  set_root_element_dimensions(search_overlay_element, tree->root->layout.max_width, tree->root->layout.max_height);
  tree->overlay = search_overlay_element;
  tree->rerender = rerender_type.all;
}

#define SEARCH_OVERLAY_COMPONENT
#endif