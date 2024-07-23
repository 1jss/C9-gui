#ifndef C9_LAYOUT

#include <stdbool.h> // bool
#include "SDL_ttf.h" // TTF_Font, TTF_SizeUTF8
#include "array.h" // Array
#include "font.h" // get_font
#include "types.h" // i32
#include "element_tree.h" // Element, ElementTree

// Recursively sets width of an element
i32 fill_scroll_width(Element *element) {
  i32 self_width = element->width;
  if (self_width == 0) {
    self_width = element->min_width;
  }
  i32 element_padding = element->padding.left + element->padding.right;
  i32 child_width = element_padding;
  Array *children = element->children;
  if (children != 0) {
    for (size_t i = 0; i < array_length(children); i++) {
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
        if (child_width < current_child_width) {
          child_width = current_child_width + element_padding;
        }
      }
    }
  } else if (element->text.length != 0) {
    TTF_Font *font = get_font();
    i32 text_w;
    TTF_SizeUTF8(font, (char *)element->text.data, &text_w, NULL);
    child_width += text_w;
  }
  if (child_width > self_width) {
    element->layout.scroll_width = child_width;
    return child_width;
  } else {
    element->layout.scroll_width = self_width;
    return self_width;
  }
}

// Recursively sets height of an element
i32 fill_scroll_height(Element *element) {
  i32 self_height = element->height;
  if (self_height == 0) {
    self_height = element->min_height;
  }
  i32 element_padding = element->padding.top + element->padding.bottom;
  i32 child_height = element_padding;
  Array *children = element->children;
  if (children != 0) {
    for (size_t i = 0; i < array_length(children); i++) {
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
        if (child_height < current_child_height) {
          child_height = current_child_height + element_padding;
        }
      }
    }
  } else if (element->text.length != 0) {
    TTF_Font *font = get_font();
    i32 text_h;
    TTF_SizeUTF8(font, (char *)element->text.data, NULL, &text_h);
    child_height += text_h;
  }
  if (child_height > self_height) {
    element->layout.scroll_height = child_height;
    return child_height;
  } else {
    element->layout.scroll_height = self_height;
    return self_height;
  }
}

// Increases width of an element and it's children to fill the parent
void fill_max_width(Element *element, i32 max_width) {
  if (element->width == 0) {
    element->layout.max_width = max_width;
  } else {
    element->layout.max_width = element->width;
    max_width = element->width;
  }

  // Cap scroll if it's out of bounds
  if (element->layout.scroll_x < 0 &&
      element->layout.scroll_width + element->layout.scroll_x < max_width) {
    element->layout.scroll_x = element->layout.max_width - element->layout.scroll_width;
  }
  if (element->layout.scroll_x > 0) {
    element->layout.scroll_x = 0;
  }

  Array *children = element->children;
  if (children != 0) {
    i32 element_padding = element->padding.left + element->padding.right;
    i32 child_width = max_width - element_padding;

    if (element->layout_direction == layout_direction.horizontal) {
      // How many children have flexible width
      i32 split_count = 0;
      for (size_t i = 0; i < array_length(children); i++) {
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

    // Set new width for children
    for (size_t i = 0; i < array_length(children); i++) {
      Element *child = array_get(children, i);
      if (element->overflow == overflow_type.scroll ||
          element->overflow == overflow_type.scroll_x) {
        // scroll_width is either width of children or min_width
        fill_max_width(child, child->layout.scroll_width);
      } else {
        fill_max_width(child, child_width);
      }
    }
  }
}

// Increases height of an element and it's children to fill the parent
void fill_max_height(Element *element, i32 max_height) {
  if (element->height == 0) {
    element->layout.max_height = max_height;
  } else {
    element->layout.max_height = element->height;
    max_height = element->height;
  }

  // Cap scroll if it's out of bounds
  if (element->layout.scroll_y < 0 &&
      element->layout.scroll_height + element->layout.scroll_y < max_height) {
    element->layout.scroll_y = element->layout.max_height - element->layout.scroll_height;
  }
  if (element->layout.scroll_y > 0) {
    element->layout.scroll_y = 0;
  }

  Array *children = element->children;
  if (children != 0) {
    i32 element_padding = element->padding.top + element->padding.bottom;
    i32 child_height = max_height - element_padding;

    if (element->layout_direction == layout_direction.vertical) {
      // How many children have flexible height
      i32 split_count = 0;
      for (size_t i = 0; i < array_length(children); i++) {
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

    // Set new height for children
    for (size_t i = 0; i < array_length(children); i++) {
      Element *child = array_get(children, i);
      if (element->overflow == overflow_type.scroll ||
          element->overflow == overflow_type.scroll_y) {
        // scroll_height is either height of children or min_height
        fill_max_height(child, child->layout.scroll_height);
      } else {
        fill_max_height(child, child_height);
      }
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
    for (size_t i = 0; i < array_length(children); i++) {
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
    for (size_t i = 0; i < array_length(children); i++) {
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

// Loop through element tree and set LayoutProp dimensions
void set_dimensions(ElementTree *tree, i32 window_width, i32 window_height) {
  fill_scroll_width(tree->root);
  fill_scroll_height(tree->root);
  fill_max_width(tree->root, window_width);
  fill_max_height(tree->root, window_height);
  set_x(tree->root, 0);
  set_y(tree->root, 0);
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
    for (size_t i = 0; i < array_length(children); i++) {
      Element *child = array_get(children, i);
      if (is_pointer_in_element(child, x, y)) {
        return get_clickable_element_at(child, x, y);
      }
    }
  }
  return 0;
};

// Recursively finds out the minimum width of an element before layout
i32 get_min_width(Element *element) {
  i32 element_padding = element->padding.left + element->padding.right;
  if (element->width > 0) {
    return element->width;
  } else if (element->min_width > 0) {
    return element->min_width;
  } else if (element->overflow == overflow_type.contain ||
             element->overflow == overflow_type.scroll_y) {
    Array *children = element->children;
    if (children == 0) return 0;
    i32 width = element_padding;
    for (size_t i = 0; i < array_length(children); i++) {
      Element *child = array_get(children, i);
      i32 child_width = get_min_width(child);
      if (element->layout_direction == layout_direction.horizontal) {
        width += child_width;
        if (i != 0) {
          width += element->gutter;
        }
      } else if (child_width + element_padding > width) {
        width = child_width + element_padding;
      }
    }
    return width;
  } else {
    return element_padding;
  }
}

// Recursively finds out the minimum height of an element before layout
i32 get_min_height(Element *element) {
  i32 element_padding = element->padding.top + element->padding.bottom;
  if (element->height > 0) {
    return element->height;
  } else if (element->min_height > 0) {
    return element->min_height;
  } else if (element->overflow == overflow_type.contain ||
             element->overflow == overflow_type.scroll_x) {
    Array *children = element->children;
    if (children == 0) return 0;
    i32 height = element_padding;
    for (size_t i = 0; i < array_length(children); i++) {
      Element *child = array_get(children, i);
      i32 child_height = get_min_height(child);
      if (element->layout_direction == layout_direction.vertical) {
        height += child_height;
        if (i != 0) {
          height += element->gutter;
        }
      } else if (child_height + element_padding > height) {
        height = child_height + element_padding;
      }
    }
    return height;
  } else {
    return element_padding;
  }
}

// Recursively scrolls the elements under the pointer starting with the children
i32 scroll_x(Element *element, i32 x, i32 y, i32 scroll_delta) {
  // Check if the pointer is within the element
  if (is_pointer_in_element(element, x, y)) {
    Array *children = element->children;
    if (children != 0) {
      for (size_t i = 0; i < array_length(children); i++) {
        Element *child = array_get(children, i);
        scroll_delta = scroll_x(child, x, y, scroll_delta);
      }
    }
    // Check if the element is scrollable
    if ((element->overflow == overflow_type.scroll ||
         element->overflow == overflow_type.scroll_x) &&
        element->layout.scroll_width > element->layout.max_width) {
      i32 max_scroll_x = element->layout.max_width - element->layout.scroll_width;
      i32 new_scroll_x = element->layout.scroll_x + scroll_delta;

      // Scroll the element left or right to the min or max
      if (new_scroll_x < max_scroll_x) {
        scroll_delta = new_scroll_x + max_scroll_x;
        element->layout.scroll_x = max_scroll_x;
      } else if (new_scroll_x > 0) {
        scroll_delta = new_scroll_x;
        element->layout.scroll_x = 0;
      } else {
        scroll_delta = 0;
        element->layout.scroll_x = new_scroll_x;
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
      for (size_t i = 0; i < array_length(children); i++) {
        Element *child = array_get(children, i);
        scroll_delta = scroll_y(child, x, y, scroll_delta);
      }
    }
    // Check if the element is scrollable
    if ((element->overflow == overflow_type.scroll ||
         element->overflow == overflow_type.scroll_y) &&
        element->layout.scroll_height > element->layout.max_height) {
      i32 max_scroll_y = element->layout.max_height - element->layout.scroll_height;
      i32 new_scroll_y = element->layout.scroll_y + scroll_delta;

      // Scroll the element up or down to the min or max
      if (new_scroll_y < max_scroll_y) {
        scroll_delta = new_scroll_y + max_scroll_y;
        element->layout.scroll_y = max_scroll_y;
      } else if (new_scroll_y > 0) {
        scroll_delta = new_scroll_y;
        element->layout.scroll_y = 0;
      } else {
        scroll_delta = 0;
        element->layout.scroll_y = new_scroll_y;
      }
    }
  }
  return scroll_delta;
}

#define C9_LAYOUT
#endif