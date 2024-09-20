#ifndef C9_LAYOUT

#include <stdbool.h> // bool
#include "SDL_ttf.h" // TTF_Font, TTF_SizeUTF8
#include "array.h" // Array
#include "element_tree.h" // Element, ElementTree
#include "font.h" // get_font
#include "font_layout.h" // get_text_block_height
#include "string.h" // s8
#include "types.h" // i32

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
  } else if (element->text.length != 0) {
    if (element->layout.max_width > 0 &&
        element->overflow != overflow_type.scroll &&
        element->overflow != overflow_type.scroll_x) {
      child_width = element->layout.max_width;
      i32 text_max_width = element->layout.max_width - element_padding;
      i32 text_height = get_text_block_height(element->font_variant, element->text, text_max_width);
      element->layout.scroll_height = text_height + element->padding.top + element->padding.bottom;
    } else {
      TTF_Font *text_font = get_font(element->font_variant);
      i32 text_width = 0;
      TTF_SizeUTF8(text_font, (char *)element->text.data, &text_width, NULL);
      child_width += text_width;
      element->layout.scroll_height = 0;
    }
  } else if (element->input != 0) {
    if (element->layout.max_width > 0 &&
        element->overflow != overflow_type.scroll &&
        element->overflow != overflow_type.scroll_x) {
      child_width = element->layout.max_width; // child width will later be used to set scroll width
      i32 text_max_width = element->layout.max_width - element_padding;
      i32 text_height = get_text_block_height(element->font_variant, element->input->text, text_max_width);
      element->layout.scroll_height = text_height + element->padding.top + element->padding.bottom;
    } else {
      TTF_Font *input_font = get_font(font_variant.regular);
      i32 text_width = 0;
      TTF_SizeUTF8(input_font, (char *)element->input->text.data, &text_width, NULL);
      if (text_width > 0) {
        child_width += text_width + 1; // Add 1 for cursor
      } else {
        child_width += 2; // Add 2 for cursor
      }
    }

    // Reset scroll if input is smaller than parent
    if (child_width < element->layout.max_width) {
      element->layout.scroll_x = 0;
    }
    // Make sure scroll is decresed when text is subtracted
    else if (child_width > element->layout.max_width &&
             child_width + element->layout.scroll_x < element->layout.max_width) {
      element->layout.scroll_x = element->layout.max_width - child_width;
    }
    // Scroll to end if cursor is at the end and outside of view
    else if (child_width + element->layout.scroll_x > element->layout.max_width &&
             element->input->selection.end_index == element->input->text.length) {
      element->layout.scroll_x = element->layout.max_width - child_width;
    }
  }
  if (child_width > self_width) {
    element->layout.scroll_width = child_width;
    return child_width;
  } else {
    element->layout.scroll_width = self_width;
    return self_width;
  }
}

// Recursively sets scroll height of an element
i32 fill_scroll_height(Element *element) {
  i32 self_height = element->height;
  i32 element_padding = element->padding.top + element->padding.bottom;
  i32 child_height = element_padding;
  Array *children = element->children;
  if (children != 0) {
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
  } else if (element->text.length != 0 || element->input != 0) {
    i32 text_height = get_font_height(element->font_variant);
    child_height += text_height;
    // This can already be set by fill_scroll_width if the text is multiline
    if (element->layout.scroll_height > child_height) {
      child_height = element->layout.scroll_height;
    }
  }
  if (child_height > self_height) {
    element->layout.scroll_height = child_height;
    if (element->height == 0) {
      // Only grow element with flexible height
      return child_height;
    } else {
      return self_height;
    }
  } else {
    element->layout.scroll_height = self_height;
    return self_height;
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
          element->render.changed = true;
        } else if (new_scroll_x > 0) {
          scroll_delta = new_scroll_x;
          element->layout.scroll_x = 0;
          element->render.changed = true;
        } else {
          scroll_delta = 0;
          element->layout.scroll_x = new_scroll_x;
          element->render.changed = true;
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
          element->render.changed = true;
          element->layout.scroll_y = max_scroll_y;
        } else if (new_scroll_y > 0) {
          scroll_delta = new_scroll_y;
          element->layout.scroll_y = 0;
          element->render.changed = true;
        } else {
          scroll_delta = 0;
          element->layout.scroll_y = new_scroll_y;
          element->render.changed = true;
        }
      }
    }
  }
  return scroll_delta;
}

#define C9_LAYOUT
#endif