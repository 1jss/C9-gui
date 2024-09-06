#ifndef BORDER_COMPONENT

#include "../constants/color_theme.h" // gray_1, white
#include "../include/arena.h" // Arena
#include "../include/element_tree.h" // Element, add_new_element, new_element, overflow_type, background_type, layout_direction, Padding
#include "../include/font.h" // font_variant

Element *border_element = 0;

void create_border_element(Arena *arena) {
  border_element = new_element(arena);
  *border_element = (Element){
    .background_type = background_type.color,
    .background.color = white,
    .layout_direction = layout_direction.vertical,
    .overflow = overflow_type.scroll_y,
    .padding = (Padding){10, 10, 10, 10},
    .gutter = 10,
  };

  Element *border_width_panel = add_new_element(arena, border_element);
  *border_width_panel = (Element){
    .background_type = background_type.color,
    .background.color = gray_1,
    .corner_radius = 25,
    .padding = (Padding){10, 10, 10, 10},
    .layout_direction = layout_direction.vertical,
    .gutter = 10,
  };

Element *border_width_text_panel = add_new_element(arena, border_width_panel);
  *border_width_text_panel = (Element){
    .background_type = background_type.color,
    .background.color = white,
    .padding = (Padding){6, 10, 6, 10},
    .layout_direction = layout_direction.vertical,
    .corner_radius = 15,
   .gutter = 6,
  };

  Element *border_width_title = add_new_element(arena, border_width_text_panel);
  *border_width_title = (Element){
    .text = to_s8("Border Width"),
    .text_color = text_color,
    .overflow = overflow_type.scroll_x,
    .font_variant = font_variant.large,
  };

  Element *border_width_description = add_new_element(arena, border_width_text_panel);
  *border_width_description = (Element){
    .text = to_s8("Border width can be set individually for each side of an element."),
    .text_color = text_color,
  };

  Element *border_width_example_panel = add_new_element(arena, border_width_panel);
  *border_width_example_panel = (Element){
    .layout_direction = layout_direction.horizontal,
    .overflow = overflow_type.scroll_x,
    .gutter = 10,
  };

  Element *border_1 = add_new_element(arena, border_width_example_panel);
  *border_1 = (Element){
    .width = 100,
    .height = 100,
    .background_type = background_type.color,
    .background.color = white,
    .border = (Border){1, 1, 1, 1},
    .border_color = border_color,
  };

  Element *border_2 = add_new_element(arena, border_width_example_panel);
  *border_2 = (Element){
    .width = 100,
    .height = 100,
    .background_type = background_type.color,
    .background.color = white,
    .border = (Border){0, 6, 6, 0},
    .border_color = border_color,
  };
 
  Element *border_3 = add_new_element(arena, border_width_example_panel);
  *border_3 = (Element){
    .width = 100,
    .height = 100,
    .background_type = background_type.color,
    .background.color = white,
    .border = (Border){6, 6, 6, 6},
    .border_color = border_color,
  };

  Element *corner_radius_panel = add_new_element(arena, border_element);
  *corner_radius_panel = (Element){
    .background_type = background_type.color,
    .background.color = gray_1,
    .corner_radius = 25,
    .padding = (Padding){10, 10, 10, 10},
    .layout_direction = layout_direction.vertical,
    .gutter = 10,
  };

  Element *corner_radius_text_panel = add_new_element(arena, corner_radius_panel);
  *corner_radius_text_panel = (Element){
    .background_type = background_type.color,
    .background.color = white,
    .padding = (Padding){6, 10, 6, 10},
    .layout_direction = layout_direction.vertical,
    .corner_radius = 15,
   .gutter = 6,
  };

  Element *corner_radius_title = add_new_element(arena, corner_radius_text_panel);
  *corner_radius_title = (Element){
    .text = to_s8("Corner Radius"),
    .text_color = text_color,
    .overflow = overflow_type.scroll_x,
    .font_variant = font_variant.large,
  };

  Element *corner_radius_description = add_new_element(arena, corner_radius_text_panel);
  *corner_radius_description = (Element){
    .text = to_s8("Corner radius can be set from zero, resulting in a square, up to half the width or height of the element, resulting in a superellipse."),
    .text_color = text_color,
  };

  Element *corner_radius_example_panel = add_new_element(arena, corner_radius_panel);
  *corner_radius_example_panel = (Element){
    .layout_direction = layout_direction.horizontal,
    .overflow = overflow_type.scroll_x,
    .gutter = 10,
  };

  Element *corner_radius_1 = add_new_element(arena, corner_radius_example_panel);
  *corner_radius_1 = (Element){
    .width = 100,
    .height = 100,
    .background_type = background_type.color,
    .background.color = white,
    .border = (Border){2, 2, 2, 2},
    .corner_radius = 0,
    .border_color = border_color,
  };
  Element *corner_radius_2 = add_new_element(arena, corner_radius_example_panel);
  *corner_radius_2 = (Element){
    .width = 100,
    .height = 100,
    .background_type = background_type.color,
    .background.color = white,
    .border = (Border){2, 2, 2, 2},
    .corner_radius = 30,
    .border_color = border_color,
  };
  Element *corner_radius_3 = add_new_element(arena, corner_radius_example_panel);
  *corner_radius_3 = (Element){
    .width = 100,
    .height = 100,
    .background_type = background_type.color,
    .background.color = white,
    .border = (Border){2, 2, 2, 2},
    .corner_radius = 50,
    .border_color = border_color,
  };
}

#define BORDER_COMPONENT
#endif