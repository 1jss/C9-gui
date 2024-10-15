#ifndef C9_LAYOUT

#include <stdbool.h> // bool
#include "arena.h" // Arena
#include "array.h" // Array
#include "element_tree.h" // Element, ElementTree
#include "font.h" // get_sft
#include "font_layout.h" // get_text_block_height, split_string_at_width
#include "schrift.h" // SFT, SFT_text_width
#include "string.h" // s8
#include "types.h" // i32

void force_input_rerender(Element *element) {
  if (element->input != 0) {
    element->changed = true;
  }
  Array *children = element->children;
  if (children != 0) {
    for (i32 i = 0; i < array_length(children); i++) {
      Element *child = array_get(children, i);
      force_input_rerender(child);
    }
  }
}

// Create children from input text, one child per newline
void populate_input_text(Arena *arena, Element *element) {
  // If the element is an input
  if (element->input != 0 && element->input->text.data != 0) {
    s8 input_text = element->input->text;
    i32 max_width = element->layout.max_width - element->padding.left - element->padding.right;
    if (element->overflow == overflow_type.scroll || element->overflow == overflow_type.scroll_x) {
      max_width = 0;
    }
    // If no child has been set up yet
    if (element->children == 0) {
      element->layout_direction = layout_direction.vertical;
      element->gutter = line_spacing;
      if (element->height > 0) {
        element->overflow = overflow_type.scroll_y;
      }

      // Split into lines and save them for later
      Array *indexes = split_string_at_width(arena, 0, input_text, max_width);
      element->input->lines = indexes;

      // Populate children with text lines
      element->children = array_create_width(arena, sizeof(Element), 4);
      // Add one child per line
      for (i32 i = 0; i < array_length(indexes); i++) {
        Line *line = array_get(indexes, i);
        s8 line_data = {
          .data = input_text.data + line->start_index,
          .length = line->end_index - line->start_index,
        };
        Element *text_element = add_new_element(arena, element);
        text_element->text = line_data;
        text_element->overflow = overflow_type.scroll_x;
        text_element->changed = true;
      }
    }
    // If the child array already exists
    // If the input element has been changed
    else if (element->changed) {
      // Split into lines and save them for later
      Array *indexes = split_string_at_width(arena, 0, input_text, max_width);
      element->input->lines = indexes;
      // Loop through all children and lines and update the changed lines, add lines that are new and remove old lines.
      i32 line_count = array_length(indexes);
      i32 child_count = array_length(element->children);
      // Update the text of the children that already exist
      for (i32 i = 0; i < line_count && i < child_count; i++) {
        Line *line = array_get(indexes, i);
        s8 line_data = {
          .data = input_text.data + line->start_index,
          .length = line->end_index - line->start_index,
        };
        Element *text_element = array_get(element->children, i);
        if (!equal_s8(text_element->text, line_data)) {
          text_element->text = line_data;
          text_element->overflow = overflow_type.scroll_x;
          text_element->changed = true;
        }
      }
      // Add elements if there are more lines than children
      if (line_count > child_count) {
        for (i32 i = child_count; i < line_count; i++) {
          Line *line = array_get(indexes, i);
          s8 line_data = {
            .data = input_text.data + line->start_index,
            .length = line->end_index - line->start_index,
          };
          Element *text_element = add_new_element(arena, element);
          text_element->text = line_data;
          text_element->changed = true;
        }
      }
      // Remove children if there are more children than lines
      else if (line_count < child_count) {
        for (i32 i = line_count; i < child_count; i++) {
          array_pop(element->children);
        }
      }
    }
  }
  // Recursively populate children if the element is not an input
  else if (element->input == 0 && element->children != 0) {
    for (i32 i = 0; i < array_length(element->children); i++) {
      Element *child = array_get(element->children, i);
      populate_input_text(arena, child);
    }
  }
  // TODO: Find out if this is needed
  else if (element->input != 0 && element->input->text.length == 0) {
    printf("No text in input\n");
    if (element->children != 0 && array_length(element->children) > 0) {
      array_clear(element->children);
    }
    if (element->input->lines != 0 && array_length(element->input->lines) > 0) {
      array_clear(element->input->lines);
    }
  }
}

// Recursively sets maximum width of an element
void fill_max_width(Element *element, i32 max_width) {
  if (element->width == 0) {
    element->layout.max_width = max_width;
  } else {
    element->layout.max_width = element->width;
    max_width = element->width;
  }

  Array *children = element->children;
  if (children != 0) {
    i32 element_padding = element->padding.left + element->padding.right;
    i32 child_width = 0;

    // Only enforce max on non scrolling children
    if (element->overflow != overflow_type.scroll &&
        element->overflow != overflow_type.scroll_x) {
      child_width = max_width - element_padding;
      // How many children have flexible width
      if (element->layout_direction == layout_direction.horizontal) {
        i32 split_count = 0;
        for (i32 i = 0; i < array_length(children); i++) {
          Element *child = array_get(children, i);
          if (child->width == 0) {
            split_count++;
          } else {
            child_width -= child->width;
          }
          if (i != 0) {
            child_width -= element->gutter;
          }
        }
        if (child_width < 0) {
          child_width = 0;
        }
        // Split width between children
        if (split_count > 0) {
          child_width = child_width / split_count;
        }
      }
      if (child_width < 0) {
        child_width = 0;
      }
    }

    // Set new width for children
    for (i32 i = 0; i < array_length(children); i++) {
      Element *child = array_get(children, i);
      fill_max_width(child, child_width);
    }
  }
}

// Recursively sets maximum height of an element
void fill_max_height(Element *element, i32 max_height) {
  if (element->height == 0) {
    element->layout.max_height = max_height;
  } else {
    element->layout.max_height = element->height;
    max_height = element->height;
  }

  Array *children = element->children;
  if (children != 0) {
    i32 element_padding = element->padding.top + element->padding.bottom;
    i32 child_height = 0;

    // Only enforce max on non scrolling children
    if (element->overflow != overflow_type.scroll &&
        element->overflow != overflow_type.scroll_y) {
      child_height = max_height - element_padding;
      // How many children have flexible height
      if (element->layout_direction == layout_direction.vertical) {
        i32 split_count = 0;
        for (i32 i = 0; i < array_length(children); i++) {
          Element *child = array_get(children, i);
          if (child->height == 0) {
            split_count++;
          } else {
            child_height -= child->height;
          }
          if (i != 0) {
            child_height -= element->gutter;
          }
        }
        if (child_height < 0) {
          child_height = 0;
        }
        // Split height between children
        if (split_count > 0) {
          child_height = child_height / split_count;
        }
      }
      if (child_height < 0) {
        child_height = 0;
      }
    }

    // Set new height for children
    for (i32 i = 0; i < array_length(children); i++) {
      Element *child = array_get(children, i);
      fill_max_height(child, child_height);
    }
  }
}

// Recursively sets scroll width of an element
i32 fill_scroll_width(Element *element) {
  i32 self_width = element->width;
  i32 element_padding = element->padding.left + element->padding.right;
  i32 child_width = element_padding;
  Array *children = element->children;
  if (children != 0) {
    for (i32 i = 0; i < array_length(children); i++) {
      Element *child = array_get(children, i);
      // Horizontal layout adds widths
      if (element->layout_direction == layout_direction.horizontal) {
        // Add gutter before all elements except the first
        if (i != 0) {
          child_width += element->gutter;
        }
        child_width += fill_scroll_width(child);
      }
      // Vertical layout uses largest width
      else {
        i32 current_child_width = fill_scroll_width(child);
        if (current_child_width + element_padding > child_width) {
          child_width = current_child_width + element_padding;
        }
      }
    }
  }
  // text is always the last child
  else if (element->text.data != 0) {
    SFT *text_font = get_sft(element->font_variant);
    i32 text_width = 0;
    SFT_text_width(text_font, element->text.data, &text_width);
    if (element->layout.max_width > 0 &&
        element->overflow != overflow_type.scroll &&
        element->overflow != overflow_type.scroll_x) {
      if (text_width < element->layout.max_width) {
        child_width += text_width;
      } else {
        child_width = element->layout.max_width;
        i32 text_max_width = element->layout.max_width - element_padding;
        i32 text_height = get_text_block_height(element->font_variant, element->text, text_max_width);
        element->layout.scroll_height = text_height + element->padding.top + element->padding.bottom;
      }
    } else {
      if (text_width > 0) {
        child_width += text_width + 1; // Add 1 for cursor
      } else {
        child_width += 2; // Add 2 for cursor
      }
      element->layout.scroll_height = 0;
    }
  }
  // Recalculate scroll for input elements
  if (element->input != 0 && (element->overflow == overflow_type.scroll || element->overflow == overflow_type.scroll_x)) {
    // Reset scroll if text is smaller than parent
    if (child_width < element->layout.max_width) {
      element->layout.scroll_x = 0;
    }
    // Make sure scroll is decresed when text is subtracted
    else if (child_width > element->layout.max_width && child_width + element->layout.scroll_x < element->layout.max_width) {
      element->layout.scroll_x = element->layout.max_width - child_width;
    }
    // Scroll to end if cursor is at the end and outside of view
    else if (child_width + element->layout.scroll_x > element->layout.max_width && element->input->selection.end_index == element->input->text.length) {
      element->layout.scroll_x = element->layout.max_width - child_width;
    }
  }
  if (child_width > self_width) {
    element->layout.scroll_width = child_width;
  } else {
    element->layout.scroll_width = self_width;
  }
  if (self_width > 0) {
    return self_width;
  } else {
    return child_width;
  }
}

// Recursively sets scroll height of an element
i32 fill_scroll_height(Element *element) {
  i32 self_height = element->height;
  i32 element_padding = element->padding.top + element->padding.bottom;
  i32 child_height = element_padding;
  Array *children = element->children;
  if (children != 0 && array_length(children) > 0) {
    for (i32 i = 0; i < array_length(children); i++) {
      Element *child = array_get(children, i);
      // Vertical layout adds heights
      if (element->layout_direction == layout_direction.vertical) {
        // Add gutter before all elements except the first
        if (i != 0) {
          child_height += element->gutter;
        }
        child_height += fill_scroll_height(child);
      }
      // Horizontal layout uses largest height
      else {
        i32 current_child_height = fill_scroll_height(child);
        if (current_child_height + element_padding > child_height) {
          child_height = current_child_height + element_padding;
        }
      }
    }
  } else if (element->text.data != 0 || element->input != 0) {
    i32 text_height = get_font_height(element->font_variant);
    child_height += text_height;
    // This can already be set by fill_scroll_width if the text is multiline
    if (element->layout.scroll_height > child_height) {
      child_height = element->layout.scroll_height;
    }
  }
  if (child_height > self_height) {
    element->layout.scroll_height = child_height;
  } else {
    element->layout.scroll_height = self_height;
  }
  if (self_height > 0) {
    return self_height;
  } else {
    return child_height;
  }
}

// Sets the max width and height of all scrolled elements
void set_max_on_scrolled(Element *element) {
  if (element->layout.max_height == 0) {
    element->layout.max_height = element->layout.scroll_height;
  }
  if (element->layout.max_width == 0) {
    element->layout.max_width = element->layout.scroll_width;
  }
  Array *children = element->children;
  if (children != 0) {
    for (i32 i = 0; i < array_length(children); i++) {
      Element *child = array_get(children, i);
      set_max_on_scrolled(child);
    }
  }
}

// Cap scroll if it's out of bounds
void cap_scroll(Element *element) {
  if (element->layout.scroll_x < 0 &&
      element->layout.scroll_width + element->layout.scroll_x < element->layout.max_width) {
    element->layout.scroll_x = element->layout.max_width - element->layout.scroll_width;
  } else if (element->layout.scroll_x > 0) {
    element->layout.scroll_x = 0;
  }
  if (element->layout.scroll_y < 0 &&
      element->layout.scroll_height + element->layout.scroll_y < element->layout.max_height) {
    element->layout.scroll_y = element->layout.max_height - element->layout.scroll_height;
  } else if (element->layout.scroll_y > 0) {
    element->layout.scroll_y = 0;
  }
  Array *children = element->children;
  if (children != 0) {
    for (i32 i = 0; i < array_length(children); i++) {
      Element *child = array_get(children, i);
      cap_scroll(child);
    }
  }
}

// Recursively sets element x position
i32 set_x(Element *element, i32 x) {
  element->layout.x = x;
  Array *children = element->children;
  // If child array is initalized
  if (children != 0) {
    i32 child_x = x + element->layout.scroll_x + element->padding.left;
    for (i32 i = 0; i < array_length(children); i++) {
      Element *child = array_get(children, i);
      // Horizontal layout sets children after each another
      if (element->layout_direction == layout_direction.horizontal) {
        child_x = set_x(child, child_x);
        child_x += element->gutter;
      }
      // Vertical layout sets same x for all children
      else {
        set_x(child, child_x);
      }
    }
  };
  return x + element->layout.max_width;
}

// Recursively sets element y position
i32 set_y(Element *element, i32 y) {
  element->layout.y = y;
  Array *children = element->children;
  // If child array is initalized
  if (children != 0) {
    i32 child_y = y + element->layout.scroll_y + element->padding.top;
    for (i32 i = 0; i < array_length(children); i++) {
      Element *child = array_get(children, i);
      // Vertical layout sets children after each another
      if (element->layout_direction == layout_direction.vertical) {
        child_y = set_y(child, child_y);
        child_y += element->gutter;
      }
      // Horizontal layout sets same y for all children
      else {
        set_y(child, child_y);
      }
    }
  };
  return y + element->layout.max_height;
}

// Sets dimensions for a root element
void set_root_element_dimensions(Element *element, i32 window_width, i32 window_height) {
  if (element != 0) {
    fill_max_width(element, window_width);
    fill_max_height(element, window_height);
    fill_scroll_width(element);
    fill_scroll_height(element);
    set_max_on_scrolled(element);
    cap_scroll(element);
    set_x(element, 0);
    set_y(element, 0);
  }
}

// Sets rerender flag on all input elements
// Useful when the window size has changed and rows need to be recalculated
void rerender_inputs(ElementTree *tree) {
  force_input_rerender(tree->root);
  if (tree->overlay != 0) {
    force_input_rerender(tree->overlay);
  }
}

void populate_inputs(ElementTree *tree) {
  populate_input_text(tree->arena, tree->root);
  if (tree->overlay != 0) {
    populate_input_text(tree->arena, tree->overlay);
  }
}

// Loop through element tree and set LayoutProp dimensions
void set_dimensions(ElementTree *tree) {
  set_root_element_dimensions(tree->root, tree->size.width, tree->size.height);
  if (tree->overlay != 0) {
    set_root_element_dimensions(tree->overlay, tree->size.width, tree->size.height);
  }
}

// Checks if a pointer position is within an element
bool is_pointer_in_element(Element *element, i32 x, i32 y) {
  i32 element_width = element->layout.max_width;
  i32 element_height = element->layout.max_height;
  return x >= element->layout.x &&
         x <= element->layout.x + element_width &&
         y >= element->layout.y &&
         y <= element->layout.y + element_height;
}

// Get clickable element at a given position
Element *get_clickable_element_at(Element *element, i32 x, i32 y) {
  if (element->on_click != 0) {
    return element;
  }
  Array *children = element->children;
  if (children != 0) {
    for (i32 i = 0; i < array_length(children); i++) {
      Element *child = array_get(children, i);
      if (is_pointer_in_element(child, x, y)) {
        return get_clickable_element_at(child, x, y);
      }
    }
  }
  return 0;
};

// Recursively scrolls the elements under the pointer starting with the children
i32 scroll_x(Element *element, i32 x, i32 y, i32 scroll_delta) {
  // Check if the pointer is within the element
  if (is_pointer_in_element(element, x, y)) {
    Array *children = element->children;
    if (children != 0) {
      for (i32 i = 0; i < array_length(children); i++) {
        Element *child = array_get(children, i);
        scroll_delta = scroll_x(child, x, y, scroll_delta);
      }
    }
    // Check if the element is scrollable
    if ((element->overflow == overflow_type.scroll ||
         element->overflow == overflow_type.scroll_x) &&
        element->layout.scroll_width > element->layout.max_width) {
      i32 max_scroll_x = element->layout.max_width - element->layout.scroll_width;

      // Only scroll if delta is positive scroll has not reached max
      if (scroll_delta > 0 || element->layout.scroll_x > max_scroll_x) {
        i32 new_scroll_x = element->layout.scroll_x + scroll_delta;
        // Scroll the element left or right to the min or max
        if (new_scroll_x < max_scroll_x) {
          scroll_delta = new_scroll_x + max_scroll_x;
          element->layout.scroll_x = max_scroll_x;
          element->changed = true;
        } else if (new_scroll_x > 0) {
          scroll_delta = new_scroll_x;
          element->layout.scroll_x = 0;
          element->changed = true;
        } else {
          scroll_delta = 0;
          element->layout.scroll_x = new_scroll_x;
          element->changed = true;
        }
      }
    }
  }
  return scroll_delta;
}

// Recursively scrolls the elements under the pointer starting with the children
i32 scroll_y(Element *element, i32 x, i32 y, i32 scroll_delta) {
  // Check if the pointer is within the element
  if (is_pointer_in_element(element, x, y)) {
    Array *children = element->children;
    if (children != 0) {
      for (i32 i = 0; i < array_length(children); i++) {
        Element *child = array_get(children, i);
        scroll_delta = scroll_y(child, x, y, scroll_delta);
      }
    }
    // Check if the element is scrollable
    if ((element->overflow == overflow_type.scroll ||
         element->overflow == overflow_type.scroll_y) &&
        element->layout.scroll_height > element->layout.max_height) {
      i32 max_scroll_y = element->layout.max_height - element->layout.scroll_height;
      // Only scroll if delta is positive or scroll has not reached max
      if (scroll_delta > 0 || element->layout.scroll_y > max_scroll_y) {
        i32 new_scroll_y = element->layout.scroll_y + scroll_delta;
        // Scroll the element up or down to the min or max
        if (new_scroll_y < max_scroll_y) {
          scroll_delta = new_scroll_y + max_scroll_y;
          element->changed = true;
          element->layout.scroll_y = max_scroll_y;
        } else if (new_scroll_y > 0) {
          scroll_delta = new_scroll_y;
          element->layout.scroll_y = 0;
          element->changed = true;
        } else {
          scroll_delta = 0;
          element->layout.scroll_y = new_scroll_y;
          element->changed = true;
        }
      }
    }
  }
  return scroll_delta;
}

#define C9_LAYOUT
#endif