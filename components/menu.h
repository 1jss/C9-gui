#ifndef MENU_COMPONENT

#include "../components/form.h" // form_element, create_form_element
#include "../components/home.h" // home_element, create_home_element
#include "../components/table.h" // table_element, create_table_element
#include "../constants/color_theme.h" // text_color, text_color_active, menu_active_color
#include "../constants/element_tags.h" // side_panel_tag, content_panel_tag
#include "../include/arena.h" // Arena
#include "../include/layout.h" // Element, Padding, set_dimensions, bump_rerender, background_type
#include "../include/string.h" // to_s8

void reset_menu_elements(Element *side_panel) {
  // Loop through all children and set background color to none
  Array *children = side_panel->children;
  if (children == 0) {
    return;
  }
  for (size_t i = 0; i < array_length(children); i++) {
    Element *child = array_get(children, i);
    child->background_type = background_type.none;
    child->text_color = text_color;
  }
}

void set_active_menu_element(Element *element) {
  element->background_type = background_type.color;
  element->text_color = text_color_active;
}

void set_menu(ElementTree *tree) {
  Element *active_element = tree->active_element;
  Element *side_panel = get_element_by_tag(tree->root, side_panel_tag);
  if (active_element != 0 && side_panel != 0) {
    reset_menu_elements(side_panel);
    set_active_menu_element(active_element);
    bump_rerender(tree);
    tree->rerender_element = side_panel;
  }
}

// Replace content of content panel
void set_content_panel(ElementTree *tree, Element *element) {
  Element *content_panel = get_element_by_tag(tree->root, content_panel_tag);
  if (content_panel != 0) {
    // Clear children and add new element
    content_panel->children = array_create(tree->arena, sizeof(Element));
    array_push(content_panel->children, element);

    // Reset scroll position
    content_panel->layout.scroll_x = 0;
    content_panel->layout.scroll_y = 0;

    // Recalculate content layout
    set_dimensions(tree, tree->root->layout.max_width, tree->root->layout.max_height);

    // Set rerendering
    bump_rerender(tree);
    tree->rerender_element = content_panel;
  }
}

void click_item_1(ElementTree *tree, void *data) {
  (void)data;
  set_menu(tree);
  if (home_element == 0) {
    create_home_element(tree->arena);
  }
  set_content_panel(tree, home_element);
}

void click_item_2(ElementTree *tree, void *data) {
  (void)data;
  set_menu(tree);
  if (form_element == 0) {
    create_form_element(tree->arena);
  }
  set_content_panel(tree, form_element);
}

void click_item_3(ElementTree *tree, void *data) {
  (void)data;
  set_menu(tree);
  if (table_element == 0) {
    create_table_element(tree->arena);
  }
  set_content_panel(tree, table_element);
}

// Fill side panel with menu items
void add_menu_items(Arena *arena, Element *side_panel) {
  Element *menu_item = add_new_element(arena, side_panel);
  *menu_item = (Element){
    .background_type = background_type.none,
    .background_color = menu_active_color,
    .padding = (Padding){5, 10, 5, 10},
    .corner_radius = 15,
    .text = to_s8("Border"),
    .text_color = text_color,
    .on_click = &click_item_1,
  };

  Element *menu_item_2 = add_new_element(arena, side_panel);
  *menu_item_2 = (Element){
    .background_type = background_type.none,
    .background_color = menu_active_color,
    .padding = (Padding){5, 10, 5, 10},
    .corner_radius = 15,
    .text = to_s8("Input"),
    .text_color = text_color,
    .on_click = &click_item_2,
  };

  Element *menu_item_3 = add_new_element(arena, side_panel);
  *menu_item_3 = (Element){
    .background_type = background_type.none,
    .background_color = menu_active_color,
    .padding = (Padding){5, 10, 5, 10},
    .corner_radius = 15,
    .text = to_s8("Table"),
    .text_color = text_color,
    .on_click = &click_item_3,
  };
}

#define MENU_COMPONENT
#endif