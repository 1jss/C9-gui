#ifndef SEARCH_OVERLAY_COMPONENT

#include "../constants/color_theme.h" // white
#include "../constants/element_tags.h" // search_panel_input_tag
#include "../helpers/style_helpers.h" // set_active_input_style, set_passive_input_style
#include "../include/arena.h" // Arena
#include "../include/element_tree.h" // Element, add_new_element, new_element, background_type, layout_direction, Padding, ElementTree
#include "../include/font.h" // font_variant
#include "../include/input.h" // clear_input
#include "../include/layout.h" // set_overlay_dimensions
#include "menu.h" // set_content_panel

Element *search_overlay_element = 0;

void close_search_overlay(ElementTree *tree, void *data) {
  (void)data;
  tree->overlay = 0;
  tree->active_element = 0;
}

void click_search_bar(ElementTree *tree, void *data) {
  (void)data;
  set_active_input_style(tree->active_element);
}

void blur_search_bar(ElementTree *tree, void *data) {
  (void)data;
  set_passive_input_style(tree->active_element);
}

void add_separator(Arena *arena, Element *parent) {
  Element *separator = add_new_element(arena, parent);
  *separator = (Element){
    .element_tag = search_result_separator_tag,
    .height = 1,
    .background_type = background_type.color,
    .background.color = gray_2,
  };
}

void click_result_item(ElementTree *tree, void *data) {
  (void)data;
  Element *new_content = 0;
  u8 item_tag = tree->active_element->element_tag;
  if (item_tag == border_menu_item) {
    if (border_element == 0) {
      create_border_element(tree->arena);
    }
    new_content = border_element;
  } else if (item_tag == background_menu_item) {
    if (background_element == 0) {
      create_background_element(tree->arena);
    }
    new_content = background_element;
  } else if (item_tag == text_menu_item) {
    if (text_element == 0) {
      create_text_element(tree->arena);
    }
    new_content = text_element;
  } else if (item_tag == table_menu_item) {
    if (table_element == 0) {
      create_table_element(tree->arena);
    }
    new_content = table_element;
  } else if (item_tag == layers_menu_item) {
    if (layers_element == 0) {
      create_layers_element(tree->arena);
    }
    new_content = layers_element;
  }
  if (new_content != 0) {
    set_menu(tree);
    set_content_panel(tree, new_content);
    close_search_overlay(tree, 0);
  }
}

// Fill search results
void fill_search_results(Arena *arena, Element *result_list, s8 search_value) {
  // Clear result list if initalized
  if (result_list->children != 0) {
    array_clear(result_list->children);
  }
  if (search_value.length == 0 || includes_s8(to_s8("border"), search_value)) {
    Element *border_item = add_new_element(arena, result_list);
    *border_item = (Element){
      .element_tag = border_menu_item,
      .padding = (Padding){8, 10, 8, 10},
      .text = to_s8("Border"),
      .text_color = text_color,
      .on_click = &click_result_item,
    };
    add_separator(arena, result_list);
  }
  if (search_value.length == 0 || includes_s8(to_s8("background"), search_value)) {
    Element *background_item = add_new_element(arena, result_list);
    *background_item = (Element){
      .element_tag = background_menu_item,
      .padding = (Padding){8, 10, 8, 10},
      .text = to_s8("Background"),
      .text_color = text_color,
      .on_click = &click_result_item,
    };
    add_separator(arena, result_list);
  }
  if (search_value.length == 0 || includes_s8(to_s8("text"), search_value)) {
    Element *text_item = add_new_element(arena, result_list);
    *text_item = (Element){
      .element_tag = text_menu_item,
      .padding = (Padding){8, 10, 8, 10},
      .text = to_s8("Text"),
      .text_color = text_color,
      .on_click = &click_result_item,
    };
    add_separator(arena, result_list);
  }
  if (search_value.length == 0 || includes_s8(to_s8("table"), search_value)) {
    Element *table_item = add_new_element(arena, result_list);
    *table_item = (Element){
      .element_tag = table_menu_item,
      .padding = (Padding){8, 10, 8, 10},
      .text = to_s8("Table"),
      .text_color = text_color,
      .on_click = &click_result_item,
    };
    add_separator(arena, result_list);
  }
  if (search_value.length == 0 || includes_s8(to_s8("layers"), search_value)) {
    Element *layers_item = add_new_element(arena, result_list);
    *layers_item = (Element){
      .element_tag = layers_menu_item,
      .padding = (Padding){8, 10, 8, 10},
      .text = to_s8("Layers"),
      .text_color = text_color,
      .on_click = &click_result_item,
    };
    add_separator(arena, result_list);
  }
  // If last item is a separator, remove it
  i32 last_index = array_last(result_list->children);
  if (last_index >= 0) {
    Element *last_item = array_get(result_list->children, last_index);
    if (last_item->element_tag == search_result_separator_tag) {
      array_pop(result_list->children);
    }
  }
}

void on_search_bar_input(ElementTree *tree, void *data) {
  char *text = (char *)data;
  if (strcmp(text, "ESCAPE") == 0) {
    close_search_overlay(tree, 0);
  } else {
    // Update search_result_list with items that match the search input value
    Element *search_result_list = get_element_by_tag(tree->overlay, search_result_list_tag);
    InputData *input = tree->active_element->input;
    if (search_result_list != 0 && input != 0) {
      // Clear the array of children
      array_clear(search_result_list->children);
      fill_search_results(tree->arena, search_result_list, input->text);
      // Add new search result items
      search_result_list->render.changed = true;
      set_dimensions(tree, tree->root->layout.max_width, tree->root->layout.max_height);
    }
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
    .element_tag = search_result_list_tag,
    .background_type = background_type.color,
    .background.color = white,
    .border = (Border){1, 1, 1, 1},
    .border_color = gray_2,
    .corner_radius = 15,
    .padding = (Padding){2, 0, 2, 0},
    .layout_direction = layout_direction.vertical,
  };

  fill_search_results(arena, search_result_list, search_input->input->text);
}

void open_search_overlay(ElementTree *tree) {
  if (search_overlay_element == 0) {
    create_search_overlay_element(tree->arena);
  }
  // Set input focus
  Element *search_input = get_element_by_tag(search_overlay_element, search_panel_input_tag);
  tree->active_element = search_input;
  // Clear input value
  clear_input(search_input->input);
  set_active_input_style(search_input);
  Element *search_result_list = get_element_by_tag(search_overlay_element, search_result_list_tag);
  fill_search_results(tree->arena, search_result_list, search_input->input->text);
  search_input->render.changed = true;
  set_root_element_dimensions(search_overlay_element, tree->root->layout.max_width, tree->root->layout.max_height);
  tree->overlay = search_overlay_element;
}

#define SEARCH_OVERLAY_COMPONENT
#endif