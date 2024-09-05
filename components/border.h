#ifndef BORDER_COMPONENT

#include "../constants/color_theme.h" // gray_2, white
#include "../include/arena.h" // Arena
#include "../include/element_tree.h" // Element, add_new_element, new_element, overflow_type, background_type, layout_direction, Padding

Element *border_element = 0;

void create_border_element(Arena *arena) {
  border_element = new_element(arena);
  *border_element = (Element){
    .background_type = background_type.color,
    .background.color = white,
    .layout_direction = layout_direction.vertical,
    .overflow = overflow_type.scroll,
    .padding = (Padding){10, 10, 10, 10},
    .gutter = 10,
  };

  Element *content_panel = add_new_element(arena, border_element);
  *content_panel = (Element){
    .background_type = background_type.color,
    .background.color = gray_2,
    .corner_radius = 25,
    .padding = (Padding){10, 10, 10, 10},
    .layout_direction = layout_direction.horizontal,
    .gutter = 10,
  };

  Element *content_panel_content = add_new_element(arena, content_panel);
  *content_panel_content = (Element){
    .width = 100,
    .height = 100,
    .background_type = background_type.color,
    .background.color = white,
    .border = (Border){2, 2, 2, 2},
    .border_color = border_color,
  };

  Element *content_panel_content_2 = add_new_element(arena, content_panel);
  *content_panel_content_2 = (Element){
    .width = 100,
    .height = 100,
    .background_type = background_type.color,
    .background.color = white,
    .border = (Border){2, 6, 6, 2},
    .border_color = border_color,
  };

  Element *content_panel_content_3 = add_new_element(arena, content_panel);
  *content_panel_content_3 = (Element){
    .width = 100,
    .height = 100,
    .background_type = background_type.color,
    .background.color = white,
    .border = (Border){2, 2, 2, 2},
    .corner_radius = 30,
    .border_color = border_color,
  };
  Element *content_panel_content_4 = add_new_element(arena, content_panel);
  *content_panel_content_4 = (Element){
    .width = 100,
    .height = 100,
    .background_type = background_type.color,
    .background.color = white,
    .border = (Border){2, 2, 2, 2},
    .corner_radius = 60,
    .border_color = border_color,
  };
}

#define BORDER_COMPONENT
#endif