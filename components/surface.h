#ifndef SURFACE_COMPONENT

#include "../constants/color_theme.h" // gray_2, white
#include "../include/arena.h" // Arena
#include "../include/element_tree.h" // Element, add_new_element, new_element, overflow_type, background_type, layout_direction, Padding

Element *surface_element = 0;

void create_surface_element(Arena *arena) {
  surface_element = new_element(arena);
  *surface_element = (Element){
    .background_type = background_type.color,
    .background.color = white,
    .layout_direction = layout_direction.vertical,
    .overflow = overflow_type.scroll,
    .padding = (Padding){10, 10, 10, 10},
    .gutter = 10,
  };

  Element *content_panel = add_new_element(arena, surface_element);
  *content_panel = (Element){
    .background_type = background_type.color,
    .background.color = gray_2,
    .corner_radius = 15,
    .padding = (Padding){10, 10, 10, 10},
    .layout_direction = layout_direction.horizontal,
    .gutter = 10,
  };

  Element *solid_background_box = add_new_element(arena, content_panel);
  *solid_background_box = (Element){
    .width = 100,
    .height = 100,
    .background_type = background_type.color,
    .background.color = white,
    .border = (Border){2, 2, 2, 2},
    .corner_radius = 15,
    .border_color = border_color,
  };

  Element *vertical_gradient_box = add_new_element(arena, content_panel);
  *vertical_gradient_box = (Element){
    .width = 100,
    .height = 100,
    .background_type = background_type.vertical_gradient,
    .background.gradient = (C9_Gradient){
      .start_color = white,
      .end_color = white_2,
    },
    .border = (Border){2, 2, 2, 2},
    .corner_radius = 15,
    .border_color = border_color,
  };

  Element *horizontal_gradient_box = add_new_element(arena, content_panel);
  *horizontal_gradient_box = (Element){
    .width = 100,
    .height = 100,
    .background_type = background_type.horizontal_gradient,
    .background.gradient = (C9_Gradient){
      .start_color = white,
      .end_color = white_2,
    },
    .border = (Border){2, 2, 2, 2},
    .corner_radius = 15,
    .border_color = border_color,
  };
}

#define SURFACE_COMPONENT
#endif