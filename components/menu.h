#ifndef MENU_COMPONENT

#include "../components/border.h" // border_element, create_border_element
#include "../components/layers.h" // layers_element, create_layers_element
#include "../components/background.h" // background_element, create_background_element
#include "../components/table.h" // table_element, create_table_element
#include "../components/text.h" // text_element, create_text_element
#include "../constants/color_theme.h" // text_color, text_color_active, menu_active_color
#include "../constants/element_tags.h" // side_panel_tag, content_panel_tag
#include "../include/arena.h" // Arena
#include "../include/element_tree.h" // Element, ElementTree, get_element_by_tag, add_new_element, Padding, background_type
#include "../include/layout.h" //  set_dimensions
#include "../include/renderer.h" // bump_rerender
#include "../include/string.h" // to_s8
#include "../include/types.h" // i32

void reset_menu_elements(Element *side_panel) {
  // Loop through all children and set background color to none
  Array *children = side_panel->children;
  if (children == 0) {
    return;
  }
  for (i32 i = 0; i < array_length(children); i++) {
    Element *child = array_get(children, i);
    child->background_type = background_type.none;
    child->text_color = text_color;
    child->font_variant = font_variant.regular;
    child->render.changed = 1;
  }
}

void set_active_menu_element(Element *element) {
  element->background_type = background_type.color;
  element->text_color = text_color_active;
  element->font_variant = font_variant.bold;
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
  if (border_element == 0) {
    create_border_element(tree->arena);
  }
  set_content_panel(tree, border_element);
}

void click_item_2(ElementTree *tree, void *data) {
  (void)data;
  set_menu(tree);
  if (background_element == 0) {
    create_background_element(tree->arena);
  }
  set_content_panel(tree, background_element);
}

void click_item_3(ElementTree *tree, void *data) {
  (void)data;
  set_menu(tree);
  if (text_element == 0) {
    create_text_element(tree->arena);
  }
  set_content_panel(tree, text_element);
}

void click_item_4(ElementTree *tree, void *data) {
  (void)data;
  set_menu(tree);
  if (table_element == 0) {
    create_table_element(tree->arena);
  }
  set_content_panel(tree, table_element);
}

void click_item_5(ElementTree *tree, void *data) {
  (void)data;
  set_menu(tree);
  if (layers_element == 0) {
    create_layers_element(tree->arena);
  }
  set_content_panel(tree, layers_element);
}

// Fill side panel with menu items
void add_menu_items(Arena *arena, Element *side_panel) {
  Element *menu_item_1 = add_new_element(arena, side_panel);
  *menu_item_1 = (Element){
    .background_type = background_type.color,
    .background.color = menu_active_color,
    .padding = (Padding){6, 10, 6, 10},
    .corner_radius = 15,
    .text = to_s8("Border"),
    .text_color = text_color,
    .on_click = &click_item_1,
    .font_variant = font_variant.bold,
  };

  Element *menu_item_2 = add_new_element(arena, side_panel);
  *menu_item_2 = (Element){
    .background_type = background_type.none,
    .background.color = menu_active_color,
    .padding = (Padding){6, 10, 6, 10},
    .corner_radius = 15,
    .text = to_s8("Background"),
    .text_color = text_color,
    .on_click = &click_item_2,
  };

  Element *menu_item_3 = add_new_element(arena, side_panel);
  *menu_item_3 = (Element){
    .background_type = background_type.none,
    .background.color = menu_active_color,
    .padding = (Padding){6, 10, 6, 10},
    .corner_radius = 15,
    .text = to_s8("Text"),
    .text_color = text_color,
    .on_click = &click_item_3,
  };

  Element *menu_item_4 = add_new_element(arena, side_panel);
  *menu_item_4 = (Element){
    .background_type = background_type.none,
    .background.color = menu_active_color,
    .padding = (Padding){6, 10, 6, 10},
    .corner_radius = 15,
    .text = to_s8("Table"),
    .text_color = text_color,
    .on_click = &click_item_4,
  };

  Element *menu_item_5 = add_new_element(arena, side_panel);
  *menu_item_5 = (Element){
    .background_type = background_type.none,
    .background.color = menu_active_color,
    .padding = (Padding){6, 10, 6, 10},
    .corner_radius = 15,
    .text = to_s8("Layers"),
    .text_color = text_color,
    .on_click = &click_item_5,
  };
}

#define MENU_COMPONENT
#endif