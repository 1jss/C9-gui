#ifndef HOME_COMPONENT

#include <string.h> // strcmp
#include "../constants/color_theme.h" // gray_2, white
#include "../constants/element_tags.h" // content_panel_tag
#include "../helpers/style_helpers.h" // set_active_input_style, set_passive_input_style
#include "../include/arena.h" // Arena
#include "../include/input.h"
#include "../include/layout.h" // Element, add_new_element, new_element, overflow_type, background_type, layout_direction, Padding

Element *home_element = 0;

void create_home_element(Arena *arena) {
  home_element = new_element(arena);
  *home_element = (Element){
    .background_type = background_type.color,
    .background_color = white,
    .layout_direction = layout_direction.vertical,
    .overflow = overflow_type.scroll_y,
    .padding = (Padding){10, 10, 10, 10},
    .gutter = 10,
  };

  Element *content_panel = add_new_element(arena, home_element);
  *content_panel = (Element){
    .background_type = background_type.color,
    .background_color = gray_2,
    .corner_radius = 15,
    .padding = (Padding){10, 10, 10, 10},
    .layout_direction = layout_direction.horizontal,
    .gutter = 10,
  };

  Element *content_panel_content = add_new_element(arena, content_panel);
  *content_panel_content = (Element){
    .width = 100,
    .height = 600,
    .background_type = background_type.color,
    .background_color = white,
    .border = (Border){1, 1, 1, 1},
    .corner_radius = 15,
  };

  Element *content_panel_content_2 = add_new_element(arena, content_panel);
  *content_panel_content_2 = (Element){
    .width = 100,
    .height = 600,
    .background_type = background_type.color,
    .background_color = white,
    .border = (Border){2, 2, 2, 2},
    .corner_radius = 30,
  };
}

#define HOME_COMPONENT
#endif