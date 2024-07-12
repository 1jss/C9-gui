#ifndef FORM_COMPONENT

#include "../include/arena.h" // Arena
#include "../include/layout.h" // Element, add_new_element, new_element, overflow_type, background_type, layout_direction, Padding
#include "../constants/color_theme.h" // gray_2, white

Element *form_element = 0;

void create_form_element(Arena *arena) {
  form_element = new_element(arena);
  *form_element = (Element){
    .background_type = background_type.color,
    .background_color = 0xFFFFFFFF,
    .layout_direction = layout_direction.vertical,
    .overflow = overflow_type.scroll_y,
    .gutter = 10,
  };

  Element *content_panel_top = add_new_element(arena, form_element);
  *content_panel_top = (Element){
    .background_type = background_type.color,
    .background_color = gray_2,
    .border_radius = 15,
    .padding = (Padding){10, 10, 10, 10},
  };

  Element *content_panel_bottom = add_new_element(arena, form_element);
  *content_panel_bottom = (Element){
    .background_type = background_type.color,
    .background_color = gray_2,
    .border_radius = 15,
    .padding = (Padding){10, 10, 10, 10},
  };

  Element *content_panel_top_content = add_new_element(arena, content_panel_bottom);
  *content_panel_top_content = (Element){
    .width = 100,
    .height = 600,
    .background_type = background_type.color,
    .background_color = white,
    .border_radius = 15,
  };
}

#define FORM_COMPONENT
#endif