#ifndef C9_STYLE_HELPERS

#include "../constants/color_theme.h" // border_color, border_color_active, text_color, text_color_active
#include "../include/layout.h" // Element

void set_active_input_style(Element *element) {
  element->border_color = border_color_active;
  element->text_color = text_color_active;
}

void set_passive_input_style(Element *element) {
  element->border_color = border_color;
  element->text_color = text_color;
}

void table_title_style(Element *element) {
  element->background_type = background_type.color;
  element->background_color = gray_1;
  element->border_radius = 15;
  element->padding = (Padding){5, 10, 5, 10};
}

void table_content_style(Element *element) {
  element->padding = (Padding){5, 10, 5, 10};
}

void set_table_style(Element *element) {
  if (element->children == 0) return;
  // Get child elements (columns) of the table
  for (size_t i = 0; i < array_length(element->children); i++) {
    Element *column = array_get(element->children, i);
    if (column->children == 0) return;
   // Loop over the column's children (cells)
    for (size_t j = 0; j < array_length(column->children); j++) {
      Element *cell = array_get(column->children, j);
      // Set the first cell in each column to table_title_style
      if (j == 0) {
        table_title_style(cell);
      }
      // Set the rest of the cells to table_content_style
      else {
        table_content_style(cell);
      }
    }
  }
}

#define C9_STYLE_HELPERS
#endif