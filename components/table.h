#ifndef TABLE_COMPONENT

#include "../constants/color_theme.h" // gray_2, white
#include "../helpers/style_helpers.h" // set_table_style
#include "../include/arena.h" // Arena
#include "../include/layout.h" // Element, add_new_element, new_element, overflow_type, background_type, layout_direction, Padding
#include "../include/string.h" // to_s8

Element *table_element = 0;

Element *add_column(Arena *arena, Element *table, i32 min_width) {
  Element *column = add_new_element(arena, table);
  column->min_width = min_width;
  column->layout_direction = layout_direction.vertical;
  return column;
}

void add_cell(Arena *arena, Element *column, char *text) {
  Element *cell = add_new_element(arena, column);
  cell->text = to_s8(text);
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
  Element *type_column = add_column(arena, table_element, 110);
  Element *name_column = add_column(arena, table_element, 180);
  Element *description_column = add_column(arena, table_element, 360);

  // Title row
  add_cell(arena, type_column, "Type");
  add_cell(arena, name_column, "Name");
  add_cell(arena, description_column, "Description");

  // element_tag row
  add_cell(arena, type_column, "u8");
  add_cell(arena, name_column, "element_tag");
  add_cell(arena, description_column, "id or group id");

  // background_type row
  add_cell(arena, type_column, "u8");
  add_cell(arena, name_column, "background_type");
  add_cell(arena, description_column, "color, horizontal_gradient, vertical_gradient");

  // background_color row
  add_cell(arena, type_column, "RGBA");
  add_cell(arena, name_column, "background_color");
  add_cell(arena, description_column, "used if background_type is color");

  // background_gradient row
  add_cell(arena, type_column, "C9_Gradient");
  add_cell(arena, name_column, "background_gradient");
  add_cell(arena, description_column, "used if background_type is gradient");

  // width row
  add_cell(arena, type_column, "i32");
  add_cell(arena, name_column, "width");
  add_cell(arena, description_column, "fixed width of the element");

  // height row
  add_cell(arena, type_column, "i32");
  add_cell(arena, name_column, "height");
  add_cell(arena, description_column, "fixed height of the element");

  // min_width row
  add_cell(arena, type_column, "i32");
  add_cell(arena, name_column, "min_width");
  add_cell(arena, description_column, "minimum width of the element");

  // min_height row
  add_cell(arena, type_column, "i32");
  add_cell(arena, name_column, "min_height");
  add_cell(arena, description_column, "minimum height of the element");

  // gutter row
  add_cell(arena, type_column, "i32");
  add_cell(arena, name_column, "gutter");
  add_cell(arena, description_column, "space between children");

  // text row
  add_cell(arena, type_column, "s8");
  add_cell(arena, name_column, "text");
  add_cell(arena, description_column, "text label");

  // input row
  add_cell(arena, type_column, "InputData*");
  add_cell(arena, name_column, "input");
  add_cell(arena, description_column, "text input object (new_input)");

  // text_color row
  add_cell(arena, type_column, "RGBA");
  add_cell(arena, name_column, "text_color");
  add_cell(arena, description_column, "color of rendered text");

  // on_click row
  add_cell(arena, type_column, "OnEvent");
  add_cell(arena, name_column, "on_click");
  add_cell(arena, description_column, "function pointer called on click");

  // on_blur row
  add_cell(arena, type_column, "OnEvent");
  add_cell(arena, name_column, "on_blur");
  add_cell(arena, description_column, "function pointer called on blur");

  // on_key_press row
  add_cell(arena, type_column, "OnEvent");
  add_cell(arena, name_column, "on_key_press");
  add_cell(arena, description_column, "function pointer called on input");

  // padding row
  add_cell(arena, type_column, "Padding");
  add_cell(arena, name_column, "padding");
  add_cell(arena, description_column, "padding inside element (4 values)");

  // border row
  add_cell(arena, type_column, "Border");
  add_cell(arena, name_column, "border");
  add_cell(arena, description_column, "border around element (4 values)");

  // border_radius row
  add_cell(arena, type_column, "i32");
  add_cell(arena, name_column, "border_radius");
  add_cell(arena, description_column, "radius of superellipse corners");

  // border_color row
  add_cell(arena, type_column, "RGBA");
  add_cell(arena, name_column, "border_color");
  add_cell(arena, description_column, "color of border");

  // children row
  add_cell(arena, type_column, "Array*");
  add_cell(arena, name_column, "children");
  add_cell(arena, description_column, "flexible array of child elements");

  // layout_direction row
  add_cell(arena, type_column, "u8");
  add_cell(arena, name_column, "layout_direction");
  add_cell(arena, description_column, "direction of flex layout (contain)");

  // overflow row
  add_cell(arena, type_column, "u8");
  add_cell(arena, name_column, "overflow");
  add_cell(arena, description_column, "contain, scroll, scroll_x, scroll_y");

  // layout row
  add_cell(arena, type_column, "LayoutProps");
  add_cell(arena, name_column, "layout");
  add_cell(arena, description_column, "props set by the layout engine");

  set_table_style(table_element);
}

#define TABLE_COMPONENT
#endif