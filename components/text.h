#ifndef TEXT_COMPONENT

#include "../constants/color_theme.h" // gray_1, white
#include "../constants/element_tags.h" // content_panel_tag
#include "../helpers/style_helpers.h" // set_active_input_style, set_passive_input_style
#include "../include/arena.h" // Arena
#include "../include/element_tree.h" // Element, add_new_element, new_element, overflow_type, background_type, layout_direction, Padding, ElementTree, get_element_by_tag
#include "../include/font.h" // font_variant
#include "../include/input.h" // new_input
#include "../include/renderer.h" // bump_rerender

Element *text_element = 0;

void click_text_input(ElementTree *tree, void *data) {
  (void)data;
  set_active_input_style(tree->active_element);
  Element *content_panel = get_element_by_tag(tree->root, content_panel_tag);
  if (content_panel != 0) {
    bump_rerender(tree);
    tree->rerender_element = content_panel;
  }
}

void blur_text_input(ElementTree *tree, void *data) {
  (void)data;
  set_passive_input_style(tree->active_element);
  Element *content_panel = get_element_by_tag(tree->root, content_panel_tag);
  if (content_panel != 0) {
    bump_rerender(tree);
    tree->rerender_element = content_panel;
  }
}

void create_text_element(Arena *arena) {
  text_element = new_element(arena);
  *text_element = (Element){
    .background_type = background_type.color,
    .background.color = white,
    .layout_direction = layout_direction.vertical,
    .overflow = overflow_type.scroll_y,
    .padding = (Padding){10, 10, 10, 10},
    .gutter = 10,
  };

  Element *font_variant_panel = add_new_element(arena, text_element);
  *font_variant_panel = (Element){
    .background_type = background_type.color,
    .background.color = gray_1,
    .corner_radius = 25,
    .padding = (Padding){10, 10, 10, 10},
    .gutter = 10,
    .layout_direction = layout_direction.vertical,
    .overflow = overflow_type.scroll_y,
  };

  Element *font_variant_text_panel = add_new_element(arena, font_variant_panel);
  *font_variant_text_panel = (Element){
    .background_type = background_type.color,
    .background.color = white,
    .padding = (Padding){6, 10, 6, 10},
    .layout_direction = layout_direction.vertical,
    .corner_radius = 15,
   .gutter = 6,
  };

  Element *font_variant_title = add_new_element(arena, font_variant_text_panel);
  *font_variant_title = (Element){
    .text = to_s8("Font Variants"),
    .text_color = text_color,
    .overflow = overflow_type.scroll_x,
    .font_variant = font_variant.large,
  };

  Element *font_variant_description = add_new_element(arena, font_variant_text_panel);
  *font_variant_description = (Element){
    .text = to_s8("C9 gui uses four font variants: regular, bold, large, and small."),
    .text_color = text_color,
  };

  Element *font_variant_example_panel = add_new_element(arena, font_variant_panel);
  *font_variant_example_panel = (Element){
    .padding = (Padding){6, 10, 6, 10},
    .layout_direction = layout_direction.vertical,
    .gutter = 10,
  };

  Element *regular_text = add_new_element(arena, font_variant_example_panel);
  *regular_text = (Element){
    .text = to_s8("Regular text"),
    .font_variant = font_variant.regular,
    .background_type = background_type.color,
    .background.color = gray_1,
  };

  Element *bold_text = add_new_element(arena, font_variant_example_panel);
  *bold_text = (Element){
    .text = to_s8("Bold text"),
    .font_variant = font_variant.bold,
    .background_type = background_type.color,
    .background.color = gray_1,
  };

  Element *large_text = add_new_element(arena, font_variant_example_panel);
  *large_text = (Element){
    .text = to_s8("Large text"),
    .font_variant = font_variant.large,
    .background_type = background_type.color,
    .background.color = gray_1,
  };

  Element *small_text = add_new_element(arena, font_variant_example_panel);
  *small_text = (Element){
    .text = to_s8("Small text"),
    .font_variant = font_variant.small,
    .background_type = background_type.color,
    .background.color = gray_1,
  };

  Element *input_panel = add_new_element(arena, text_element);
  *input_panel = (Element){
    .background_type = background_type.color,
    .background.color = gray_1,
    .corner_radius = 25,
    .padding = (Padding){10, 10, 10, 10},
    .layout_direction = layout_direction.vertical,
  };

  Element *input_text_panel = add_new_element(arena, input_panel);
  *input_text_panel = (Element){
    .background_type = background_type.color,
    .background.color = white,
    .padding = (Padding){6, 10, 6, 10},
    .layout_direction = layout_direction.vertical,
    .corner_radius = 15,
   .gutter = 6,
  };

  Element *input_text_title = add_new_element(arena, input_text_panel);
  *input_text_title = (Element){
    .text = to_s8("Text Input"),
    .text_color = text_color,
    .overflow = overflow_type.scroll_x,
    .font_variant = font_variant.large,
  };

  Element *input_text_description_1 = add_new_element(arena, input_text_panel);
  *input_text_description_1 = (Element){
    .text = to_s8("There are two types of text inputs: single line and multiline. Vertical overflow setting is used to determine the type. If the text is allowed to scroll horizontally, a single line input is used, otherwise a multiline input is used."),
    .text_color = text_color,
  };

  Element *input_text_description_2 = add_new_element(arena, input_text_panel);
  *input_text_description_2 = (Element){
    .text = to_s8("Minimum height can be used to set initial height of a growing muliline input."),
    .text_color = text_color,
  };

  Element *single_line_input_title = add_new_element(arena, input_panel);
  *single_line_input_title = (Element){
    .text = to_s8("SINGLE LINE"),
    .text_color = text_color,
    .font_variant = font_variant.small,
    .background_type = background_type.color,
    .background.color = gray_1,
    .padding = (Padding){10, 0, 6, 10},
  };

  Element *single_line_text_input = add_new_element(arena, input_panel);
  *single_line_text_input = (Element){
    .background_type = background_type.color,
    .background.color = white,
    .padding = (Padding){6, 10, 6, 10},
    .corner_radius = 15,
    .border_color = border_color,
    .border = (Border){1, 1, 1, 1},
    .input = new_input(arena),
    .text_color = text_color,
    .on_click = &click_text_input,
    .on_blur = &blur_text_input,
    .overflow = overflow_type.scroll_x,
  };

  Element *multi_line_input_title = add_new_element(arena, input_panel);
  *multi_line_input_title = (Element){
    .text = to_s8("MULTILINE"),
    .text_color = text_color,
    .font_variant = font_variant.small,
    .background_type = background_type.color,
    .background.color = gray_1,
    .padding = (Padding){10, 0, 6, 10},
  };

  Element *multi_line_text_input = add_new_element(arena, input_panel);
  *multi_line_text_input = (Element){
    .background_type = background_type.color,
    .background.color = white,
    .padding = (Padding){6, 10, 6, 10},
    .corner_radius = 15,
    .border_color = border_color,
    .border = (Border){1, 1, 1, 1},
    .input = new_input(arena),
    .text_color = text_color,
    .on_click = &click_text_input,
    .on_blur = &blur_text_input,
  };

  Element *multi_line_input_title_2 = add_new_element(arena, input_panel);
  *multi_line_input_title_2 = (Element){
    .text = to_s8("MINIMUM HEIGHT"),
    .text_color = text_color,
    .font_variant = font_variant.small,
    .background_type = background_type.color,
    .background.color = gray_1,
    .padding = (Padding){10, 0, 6, 10},
  };

  Element *multi_line_text_input_2 = add_new_element(arena, input_panel);
  *multi_line_text_input_2 = (Element){
    .min_height = 100,
    .background_type = background_type.color,
    .background.color = white,
    .padding = (Padding){6, 10, 6, 10},
    .corner_radius = 15,
    .border_color = border_color,
    .border = (Border){1, 1, 1, 1},
    .input = new_input(arena),
    .text_color = text_color,
    .on_click = &click_text_input,
    .on_blur = &blur_text_input,
  };

  Element *text_box_panel = add_new_element(arena, text_element);
  *text_box_panel = (Element){
    .background_type = background_type.color,
    .background.color = gray_1,
    .corner_radius = 25,
    .padding = (Padding){10, 10, 10, 10},
    .layout_direction = layout_direction.vertical,
    .overflow = overflow_type.scroll_y,
  };

  Element *text_box_text_panel = add_new_element(arena, text_box_panel);
  *text_box_text_panel = (Element){
    .background_type = background_type.color,
    .background.color = white,
    .padding = (Padding){6, 10, 6, 10},
    .layout_direction = layout_direction.vertical,
    .corner_radius = 15,
   .gutter = 6,
  };

  Element *text_box_text_title = add_new_element(arena, text_box_text_panel);
  *text_box_text_title = (Element){
    .text = to_s8("Text Boxes"),
    .text_color = text_color,
    .overflow = overflow_type.scroll_x,
    .font_variant = font_variant.large,
  };

  Element *text_box_text_description = add_new_element(arena, text_box_text_panel);
  *text_box_text_description = (Element){
    .text = to_s8("As with text input, text boxes can be either vertically scrolling or multiline with automatic and manual linebreaks. The overflow setting determines which type of box is used."),
    .text_color = text_color,
  };

  Element *scrolling_text_title = add_new_element(arena, text_box_panel);
  *scrolling_text_title = (Element){
    .text = to_s8("VERTICALLY SCROLLING"),
    .text_color = text_color,
    .font_variant = font_variant.small,
    .background_type = background_type.color,
    .background.color = gray_1,
    .padding = (Padding){10, 0, 6, 10},
  };
  Element *scrolling_text = add_new_element(arena, text_box_panel);
  *scrolling_text = (Element){
    .text = to_s8("This is a scrolling text that overflows its parent. Scrolling horizontally on this line will reveal the rest of its content."),
    .text_color = text_color,
    .background_type = background_type.color,
    .background.color = white,
    .padding = (Padding){6, 10, 6, 10},
    .corner_radius = 15,
    .overflow = overflow_type.scroll_x,
    .font_variant = font_variant.regular,
  };

  Element *multiline_text_title = add_new_element(arena, text_box_panel);
  *multiline_text_title = (Element){
    .text = to_s8("AUTOMATIC LINEBREAKS"),
    .text_color = text_color,
    .font_variant = font_variant.small,
    .background_type = background_type.color,
    .background.color = gray_1,
    .padding = (Padding){10, 0, 6, 10},
  };
  Element *multiline_text = add_new_element(arena, text_box_panel);
  *multiline_text = (Element){
    .text = to_s8("This is a long text that does not scroll horizontally. Instead it reflows to the next line."),
    .text_color = text_color,
    .background_type = background_type.color,
    .background.color = white,
    .padding = (Padding){6, 10, 6, 10},
    .corner_radius = 15,
    .overflow = overflow_type.contain,
    .font_variant = font_variant.regular,
  };

  Element *linebreak_text_title = add_new_element(arena, text_box_panel);
  *linebreak_text_title = (Element){
    .text = to_s8("MANUAL LINEBREAKS"),
    .text_color = text_color,
    .font_variant = font_variant.small,
    .background_type = background_type.color,
    .background.color = gray_1,
    .padding = (Padding){10, 0, 6, 10},
  };

  Element *linebreak_text = add_new_element(arena, text_box_panel);
  *linebreak_text = (Element){
    .text = to_s8("This is a long\nmanually broken\ntext that does not\nscroll horizontally.\nInstead it reflows\nto the next line."),
    .text_color = text_color,
    .background_type = background_type.color,
    .background.color = white,
    .padding = (Padding){6, 10, 6, 10},
    .corner_radius = 15,
    .overflow = overflow_type.contain,
    .font_variant = font_variant.regular,
  };
}

#define TEXT_COMPONENT
#endif