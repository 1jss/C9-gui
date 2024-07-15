#ifndef STYLE_HELPERS

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

#define STYLE_HELPERS
#endif