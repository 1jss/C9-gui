#ifndef BACKGROUND_COMPONENT

#include "../constants/color_theme.h" // gray_1, white
#include "../include/arena.h" // Arena
#include "../include/element_tree.h" // Element, add_new_element, new_element, overflow_type, background_type, layout_direction, Padding
#include "../include/font.h" // font_variant

Element *background_element = 0;

void create_background_element(Arena *arena) {
  background_element = new_element(arena);
  *background_element = (Element){
    .background_type = background_type.color,
    .background.color = white,
    .layout_direction = layout_direction.vertical,
    .overflow = overflow_type.scroll_y,
    .padding = (Padding){10, 10, 0, 10},
    .gutter = 10,
  };

  Element *background_type_panel = add_new_element(arena, background_element);
  *background_type_panel = (Element){
    .background_type = background_type.color,
    .background.color = gray_1,
    .corner_radius = 25,
    .padding = (Padding){10, 10, 5, 10},
    .layout_direction = layout_direction.vertical,
    .gutter = 10,
    .border = (Border){1, 1, 1, 1},
    .border_color = gray_2,
  };

  Element *background_type_text_panel = add_new_element(arena, background_type_panel);
  *background_type_text_panel = (Element){
    .background_type = background_type.color,
    .background.color = white,
    .padding = (Padding){6, 10, 6, 10},
    .layout_direction = layout_direction.vertical,
    .corner_radius = 15,
    .gutter = 6,
    .border = (Border){1, 1, 1, 1},
    .border_color = gray_2,
  };

  Element *background_type_title = add_new_element(arena, background_type_text_panel);
  *background_type_title = (Element){
    .text = to_s8("Background Type"),
    .text_color = text_color,
    .overflow = overflow_type.scroll_x,
    .font_variant = font_variant.large,
  };

  Element *background_type_description_1 = add_new_element(arena, background_type_text_panel);
  *background_type_description_1 = (Element){
    .text = to_s8("The background type can be solid color, horizontal gradient, vertical gradient or image."),
    .text_color = text_color,
  };
  Element *background_type_description_2 = add_new_element(arena, background_type_text_panel);
  *background_type_description_2 = (Element){
    .text = to_s8("The gradients are smoothed using a blue noise dithering algorithm reducing banding artifacts."),
    .text_color = text_color,
  };
  Element *background_type_description_3 = add_new_element(arena, background_type_text_panel);
  *background_type_description_3 = (Element){
    .text = to_s8("If the background is set to image, no border or corner radius will be drawn on that element."),
    .text_color = text_color,
  };

  Element *background_type_example_panel = add_new_element(arena, background_type_panel);
  *background_type_example_panel = (Element){
    .layout_direction = layout_direction.horizontal,
    .overflow = overflow_type.scroll_x,
    .gutter = 10,
    .padding = (Padding){0, 0, 5, 0},
  };

  Element *solid_background_box = add_new_element(arena, background_type_example_panel);
  *solid_background_box = (Element){
    .width = 100,
    .height = 100,
    .background_type = background_type.color,
    .background.color = white,
    .border = (Border){2, 2, 2, 2},
    .corner_radius = 15,
    .border_color = border_color,
  };

  Element *vertical_gradient_box = add_new_element(arena, background_type_example_panel);
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

  Element *horizontal_gradient_box = add_new_element(arena, background_type_example_panel);
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

  Element *image_border_box = add_new_element(arena, background_type_example_panel);
  *image_border_box = (Element){
    .width = 100,
    .height = 100,
    .background_type = background_type.color,
    .background.color = white,
    .border = (Border){2, 2, 2, 2},
    .padding = (Padding){
      .top = 35,
      .bottom = 35,
      .left = 32,
      .right = 32,
    },
    .corner_radius = 15,
    .border_color = border_color,
  };
  Element *image_background_box = add_new_element(arena, image_border_box);
  *image_background_box = (Element){
    .width = 36,
    .height = 31,
    .background_type = background_type.image,
    .background.image = to_s8("C9_segment_small.png"),
  };
}

#define BACKGROUND_COMPONENT
#endif