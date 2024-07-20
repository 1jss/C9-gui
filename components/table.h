#ifndef TABLE_COMPONENT

#include "../constants/color_theme.h" // gray_2, white
#include "../include/arena.h" // Arena
#include "../include/layout.h" // Element, add_new_element, new_element, overflow_type, background_type, layout_direction, Padding
#include "../include/string.h" // to_s8

Element *table_element = 0;

void title_style(Element *element) {
  *element = (Element){
    .height = 30,
    .background_type = background_type.color,
    .background_color = gray_1,
    .border_radius = 15,
    .padding = (Padding){5, 10, 5, 10},
  };
}

void content_style(Element *element) {
  *element = (Element){
    .height = 30,
    .padding = (Padding){5, 10, 5, 10},
  };
}

void create_table_element(Arena *arena) {
  table_element = new_element(arena);
  *table_element = (Element){
    .layout_direction = layout_direction.horizontal,
    .overflow = overflow_type.scroll,
    .padding = (Padding){10, 10, 10, 10},
    .gutter = 10,
  };

  // Columns
  Element *type_column = add_new_element(arena, table_element);
  *type_column = (Element){
    .min_width = 110,
    .layout_direction = layout_direction.vertical,
  };
  Element *name_column = add_new_element(arena, table_element);
  *name_column = (Element){
    .min_width = 200,
    .layout_direction = layout_direction.vertical,
  };
  Element *description_column = add_new_element(arena, table_element);
  *description_column = (Element){
    .min_width = 400,
    .layout_direction = layout_direction.vertical,
  };

  // Title row
  Element *type_title = add_new_element(arena, type_column);
  title_style(type_title);
  type_title->text = to_s8("Type");
  Element *name_title = add_new_element(arena, name_column);
  title_style(name_title);
  name_title->text = to_s8("Name");
  Element *description_title = add_new_element(arena, description_column);
  title_style(description_title);
  description_title->text = to_s8("Description");

  // element_tag row
  Element *type_tag = add_new_element(arena, type_column);
  content_style(type_tag);
  type_tag->text = to_s8("u8");
  Element *name_tag = add_new_element(arena, name_column);
  content_style(name_tag);
  name_tag->text = to_s8("element_tag");
  Element *description_tag = add_new_element(arena, description_column);
  content_style(description_tag);
  description_tag->text = to_s8("id or group id");

  // background_type row
  Element *type_background_type = add_new_element(arena, type_column);
  content_style(type_background_type);
  type_background_type->text = to_s8("u8");
  Element *name_background_type = add_new_element(arena, name_column);
  content_style(name_background_type);
  name_background_type->text = to_s8("background_type");
  Element *description_background_type = add_new_element(arena, description_column);
  content_style(description_background_type);
  description_background_type->text = to_s8("color, horizontal_gradient, vertical_gradient");

  // background_color row
  Element *type_background_color = add_new_element(arena, type_column);
  content_style(type_background_color);
  type_background_color->text = to_s8("RGBA");
  Element *name_background_color = add_new_element(arena, name_column);
  content_style(name_background_color);
  name_background_color->text = to_s8("background_color");
  Element *description_background_color = add_new_element(arena, description_column);
  content_style(description_background_color);
  description_background_color->text = to_s8("used if background_type is color ");

  // background_gradient row
  Element *type_background_gradient = add_new_element(arena, type_column);
  content_style(type_background_gradient);
  type_background_gradient->text = to_s8("C9_Gradient");
  Element *name_background_gradient = add_new_element(arena, name_column);
  content_style(name_background_gradient);
  name_background_gradient->text = to_s8("background_gradient");
  Element *description_background_gradient = add_new_element(arena, description_column);
  content_style(description_background_gradient);
  description_background_gradient->text = to_s8("used if background_type is gradient");

  // width row
  Element *type_width = add_new_element(arena, type_column);
  content_style(type_width);
  type_width->text = to_s8("i32");
  Element *name_width = add_new_element(arena, name_column);
  content_style(name_width);
  name_width->text = to_s8("width");
  Element *description_width = add_new_element(arena, description_column);
  content_style(description_width);
  description_width->text = to_s8("fixed width of the element");

  // height row
  Element *type_height = add_new_element(arena, type_column);
  content_style(type_height);
  type_height->text = to_s8("i32");
  Element *name_height = add_new_element(arena, name_column);
  content_style(name_height);
  name_height->text = to_s8("height");
  Element *description_height = add_new_element(arena, description_column);
  content_style(description_height);
  description_height->text = to_s8("fixed height of the element");

  // min_width row
  Element *type_min_width = add_new_element(arena, type_column);
  content_style(type_min_width);
  type_min_width->text = to_s8("i32");
  Element *name_min_width = add_new_element(arena, name_column);
  content_style(name_min_width);
  name_min_width->text = to_s8("min_width");
  Element *description_min_width = add_new_element(arena, description_column);
  content_style(description_min_width);
  description_min_width->text = to_s8("minimum width of the element");

  // min_height row
  Element *type_min_height = add_new_element(arena, type_column);
  content_style(type_min_height);
  type_min_height->text = to_s8("i32");
  Element *name_min_height = add_new_element(arena, name_column);
  content_style(name_min_height);
  name_min_height->text = to_s8("min_height");
  Element *description_min_height = add_new_element(arena, description_column);
  content_style(description_min_height);
  description_min_height->text = to_s8("minimum height of the element");

  // gutter row
  Element *type_gutter = add_new_element(arena, type_column);
  content_style(type_gutter);
  type_gutter->text = to_s8("i32");
  Element *name_gutter = add_new_element(arena, name_column);
  content_style(name_gutter);
  name_gutter->text = to_s8("gutter");
  Element *description_gutter = add_new_element(arena, description_column);
  content_style(description_gutter);
  description_gutter->text = to_s8("space between children");

  // text row
  Element *type_text = add_new_element(arena, type_column);
  content_style(type_text);
  type_text->text = to_s8("s8");
  Element *name_text = add_new_element(arena, name_column);
  content_style(name_text);
  name_text->text = to_s8("text");
  Element *description_text = add_new_element(arena, description_column);
  content_style(description_text);
  description_text->text = to_s8("text label");

  // input row
  Element *type_input = add_new_element(arena, type_column);
  content_style(type_input);
  type_input->text = to_s8("InputData*");
  Element *name_input = add_new_element(arena, name_column);
  content_style(name_input);
  name_input->text = to_s8("input");
  Element *description_input = add_new_element(arena, description_column);
  content_style(description_input);
  description_input->text = to_s8("text input object (new_input)");

  // text_color row
  Element *type_text_color = add_new_element(arena, type_column);
  content_style(type_text_color);
  type_text_color->text = to_s8("RGBA");
  Element *name_text_color = add_new_element(arena, name_column);
  content_style(name_text_color);
  name_text_color->text = to_s8("text_color");
  Element *description_text_color = add_new_element(arena, description_column);
  content_style(description_text_color);
  description_text_color->text = to_s8("color of rendered text");

  // on_click row
  Element *type_on_click = add_new_element(arena, type_column);
  content_style(type_on_click);
  type_on_click->text = to_s8("OnEvent");
  Element *name_on_click = add_new_element(arena, name_column);
  content_style(name_on_click);
  name_on_click->text = to_s8("on_click");
  Element *description_on_click = add_new_element(arena, description_column);
  content_style(description_on_click);
  description_on_click->text = to_s8("function pointer called on click");

  // on_blur row
  Element *type_on_blur = add_new_element(arena, type_column);
  content_style(type_on_blur);
  type_on_blur->text = to_s8("OnEvent");
  Element *name_on_blur = add_new_element(arena, name_column);
  content_style(name_on_blur);
  name_on_blur->text = to_s8("on_blur");
  Element *description_on_blur = add_new_element(arena, description_column);
  content_style(description_on_blur);
  description_on_blur->text = to_s8("function pointer called on blur");

  // on_key_press row
  Element *type_on_key_press = add_new_element(arena, type_column);
  content_style(type_on_key_press);
  type_on_key_press->text = to_s8("On Event");
  Element *name_on_key_press = add_new_element(arena, name_column);
  content_style(name_on_key_press);
  name_on_key_press->text = to_s8("on_key_press");
  Element *description_on_key_press = add_new_element(arena, description_column);
  content_style(description_on_key_press);
  description_on_key_press->text = to_s8("function pointer called on input");

  // padding row
  Element *type_padding = add_new_element(arena, type_column);
  content_style(type_padding);
  type_padding->text = to_s8("Padding");
  Element *name_padding = add_new_element(arena, name_column);
  content_style(name_padding);
  name_padding->text = to_s8("padding");
  Element *description_padding = add_new_element(arena, description_column);
  content_style(description_padding);
  description_padding->text = to_s8("padding inside element (4 values)");

  // border row
  Element *type_border = add_new_element(arena, type_column);
  content_style(type_border);
  type_border->text = to_s8("Border");
  Element *name_border = add_new_element(arena, name_column);
  content_style(name_border);
  name_border->text = to_s8("border");
  Element *description_border = add_new_element(arena, description_column);
  content_style(description_border);
  description_border->text = to_s8("border around element (4 values)");

  // border_radius row
  Element *type_border_radius = add_new_element(arena, type_column);
  content_style(type_border_radius);
  type_border_radius->text = to_s8("i32");
  Element *name_border_radius = add_new_element(arena, name_column);
  content_style(name_border_radius);
  name_border_radius->text = to_s8("border_radius");
  Element *description_border_radius = add_new_element(arena, description_column);
  content_style(description_border_radius);
  description_border_radius->text = to_s8("radius of superellipse corners");

  // border_color row
  Element *type_border_color = add_new_element(arena, type_column);
  content_style(type_border_color);
  type_border_color->text = to_s8("RGBA");
  Element *name_border_color = add_new_element(arena, name_column);
  content_style(name_border_color);
  name_border_color->text = to_s8("border_color");
  Element *description_border_color = add_new_element(arena, description_column);
  content_style(description_border_color);
  description_border_color->text = to_s8("color of border");

  // children row
  Element *type_children = add_new_element(arena, type_column);
  content_style(type_children);
  type_children->text = to_s8("Array*");
  Element *name_children = add_new_element(arena, name_column);
  content_style(name_children);
  name_children->text = to_s8("children");
  Element *description_children = add_new_element(arena, description_column);
  content_style(description_children);
  description_children->text = to_s8("flexible array of child elements");

  // layout_direction row
  Element *type_layout_direction = add_new_element(arena, type_column);
  content_style(type_layout_direction);
  type_layout_direction->text = to_s8("u8");
  Element *name_layout_direction = add_new_element(arena, name_column);
  content_style(name_layout_direction);
  name_layout_direction->text = to_s8("layout_direction");
  Element *description_layout_direction = add_new_element(arena, description_column);
  content_style(description_layout_direction);
  description_layout_direction->text = to_s8("direction of flex layout");

  // overflow row
  Element *type_overflow = add_new_element(arena, type_column);
  content_style(type_overflow);
  type_overflow->text = to_s8("u8");
  Element *name_overflow = add_new_element(arena, name_column);
  content_style(name_overflow);
  name_overflow->text = to_s8("overflow");
  Element *description_overflow = add_new_element(arena, description_column);
  content_style(description_overflow);
  description_overflow->text = to_s8("contain or scroll children");

  // layout row
  Element *type_layout = add_new_element(arena, type_column);
  content_style(type_layout);
  type_layout->text = to_s8("LayoutProps");
  Element *name_layout = add_new_element(arena, name_column);
  content_style(name_layout);
  name_layout->text = to_s8("layout");
  Element *description_layout = add_new_element(arena, description_column);
  content_style(description_layout);
  description_layout->text = to_s8("props set by the layout engine");
}

#define TABLE_COMPONENT
#endif