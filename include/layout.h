#ifndef C9_LAYOUT

#include "arena.h" // Arena
#include "array.h" // Array
#include "color.h" // RGBA, C9_Gradient, gradient
#include "string.h" // s8
#include "types.h" // u8, i32

const RGBA C9_default_background_color = 0xFFFFFFFF;
const RGBA C9_default_text_color = 0x000000FF;
const RGBA C9_default_border_color = 0x000000FF;

// Type of rerender
typedef struct {
  u8 none;
  u8 all;
  u8 selected;
} RerenderType;

const RerenderType rerender_type = {
  .none = 0,
  .all = 1,
  .selected = 2,
};

// Layout alignment
typedef struct {
  u8 start; // Starts at parent 0,0 or where sibling ends
  u8 end; // Starts at x and y relative to parent 0,0
  u8 spread; // Starts at x and y relative to global 0,0
} LayoutAlign;

const LayoutAlign layout_align = {
  .start = 0,
  .end = 1,
  .spread = 2,
};

// Layout direction
typedef struct {
  u8 horizontal;
  u8 vertical;
} LayoutDirection;

const LayoutDirection layout_direction = {
  .horizontal = 0,
  .vertical = 1,
};

// Overflow type
typedef struct {
  u8 contain;
  u8 scroll;
  u8 scroll_x;
  u8 scroll_y;
} OverflowType;

const OverflowType overflow_type = {
  .contain = 0,
  .scroll = 1,
  .scroll_x = 2,
  .scroll_y = 3,
};

// Background type
typedef struct {
  u8 none; // No background color
  u8 color; // Single color
  u8 horizontal_gradient; // Gradient from left to right
  u8 vertical_gradient; // Gradient from top to bottom
} BackgroundType;

const BackgroundType background_type = {
  .none = 0,
  .color = 1,
  .horizontal_gradient = 2,
  .vertical_gradient = 3,
};

// Forward declaration of ElementTree
struct ElementTree;
typedef struct ElementTree ElementTree;

// Function pointer typedef for on_click and on_blur
// The function takes a pointer to the ElementTree.
typedef void (*OnEvent)(ElementTree *);

typedef struct {
  i32 x;
  i32 y;
  i32 max_width; // Flexible width
  i32 max_height; // Flexible height
  i32 scroll_width; // Width of children
  i32 scroll_height; // Height of children
  i32 scroll_x; // current horizontal scroll
  i32 scroll_y; // current vertical scroll
} LayoutProps;

const LayoutProps empty_layout_props = {
  .x = 0,
  .y = 0,
  .max_width = 0,
  .max_height = 0,
  .scroll_width = 0,
  .scroll_height = 0,
  .scroll_x = 0,
  .scroll_y = 0,
};

typedef struct {
  i32 top;
  i32 right;
  i32 bottom;
  i32 left;
} Padding;

typedef struct {
  i32 top;
  i32 right;
  i32 bottom;
  i32 left;
} Border;

// element tree nodes
typedef struct Element {
  u8 element_tag; // Optional id or group id
  u8 background_type;
  RGBA background_color;
  C9_Gradient background_gradient;
  i32 width;
  i32 height;
  i32 min_width;
  i32 min_height;
  i32 gutter;
  s8 text;
  RGBA text_color;
  OnEvent on_click; // Function pointer
  OnEvent on_blur; // Function pointer
  Padding padding;
  Border border;
  i32 border_radius;
  RGBA border_color;
  Array *children; // Flexible array of child elements of type Element
  u8 layout_direction;
  u8 overflow;
  LayoutProps layout; // Props set by the layout engine
} Element;

Element empty_element = {
  .element_tag = 0,
  .background_type = 0, // No background color
  .background_color = C9_default_background_color,
  .background_gradient = {
    .start_color = 0xFFFFFFFF,
    .end_color = 0xFFFFFFFF,
    .start_at = 0,
    .end_at = 1,
  },
  .width = 0,
  .height = 0,
  .min_width = 0,
  .min_height = 0,
  .gutter = 0,
  .text = {.data = 0, .length = 0},
  .text_color = C9_default_text_color,
  .on_click = 0,
  .on_blur = 0,
  .padding = {0, 0, 0, 0},
  .border = {0, 0, 0, 0},
  .border_radius = 0,
  .border_color = C9_default_border_color,
  .children = 0,
  .layout_direction = 0,
  .overflow = 0,
  .layout = {
    .x = 0,
    .y = 0,
    .max_width = 0,
    .max_height = 0,
    .scroll_width = 0,
    .scroll_height = 0,
    .scroll_x = 0,
    .scroll_y = 0,
  },
};

// Element Tree already typedefed
struct ElementTree {
  Arena *arena;
  Element *root;
  Element *active_element;
  Element *rerender_element;
  u8 rerender;
};

// Create a new element tree and return a pointer to it
ElementTree *new_element_tree(Arena *arena) {
  ElementTree *tree = (ElementTree *)arena_fill(arena, sizeof(ElementTree));
  tree->arena = arena;
  // Get memory for the root element
  Element *root = (Element *)arena_fill(arena, sizeof(Element));
  // Initialize the root element
  *root = (Element){
    .padding = {0, 0, 0, 0},
    .border = {0, 0, 0, 0},
    .children = 0,
    .layout_direction = layout_direction.vertical,
    .layout = {
      .x = 0,
      .y = 0,
      .max_width = 0,
      .max_height = 0,
      .scroll_width = 0,
      .scroll_height = 0,
      .scroll_x = 0,
      .scroll_y = 0,
    },
  };

  // Assign the root element to the tree
  tree->root = root;
  tree->active_element = 0;
  tree->rerender_element = 0;
  tree->rerender = rerender_type.all;
  return tree;
}

// Add a new child element to a parent and return a pointer to it
Element *add_new_element(Arena *arena, Element *parent) {
  // If the parent element has no children, create a new array
  if (parent->children == 0) {
    parent->children = array_create(arena, sizeof(Element));
  }
  // Add a new child element to the parent element
  array_push(parent->children, &empty_element);
  // Return a pointer to the child element
  return array_get(parent->children, array_last(parent->children));
}

// Create a new element and return a pointer to it
Element *new_element(Arena *arena) {
  Element *element = (Element *)arena_fill(arena, sizeof(Element));
  *element = empty_element;
  return element;
}

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
    i32 child_width = child_width = max_width - element_padding;

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
  if (element->width > 0) {
    return x + element->width;
  } else {
    return x + element->layout.max_width;
  }
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
  if (element->height > 0) {
    return y + element->height;
  } else {
    return y + element->layout.max_height;
  }
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

// Get the leaf element at a given position
Element *get_element_at(Element *element, i32 x, i32 y) {
  Array *children = element->children;
  if (children == 0) {
    return element;
  }
  for (size_t i = 0; i < array_length(children); i++) {
    Element *child = array_get(children, i);
    i32 child_x = child->layout.x;
    i32 child_y = child->layout.y;
    i32 child_width = child->width > 0 ? child->width : child->layout.max_width;
    i32 child_height = child->height > 0 ? child->height : child->layout.max_height;
    if (x >= child_x && x <= child_x + child_width && y >= child_y && y <= child_y + child_height) {
      return get_element_at(child, x, y);
    }
  }
  return element;
};

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

// Recursively scrolls the elements under the pointer children first
i32 scroll_x(Element *element, i32 x, i32 y, i32 scroll_delta) {
  // Check if the pointer is within the element
  if (x >= element->layout.x &&
      x <= element->layout.x + element->layout.max_width &&
      y >= element->layout.y &&
      y <= element->layout.y + element->layout.max_height) {
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

// Recursively scrolls the elements under the pointer children first
i32 scroll_y(Element *element, i32 x, i32 y, i32 scroll_delta) {
  // Check if the pointer is within the element
  if (x >= element->layout.x &&
      x <= element->layout.x + element->layout.max_width &&
      y >= element->layout.y &&
      y <= element->layout.y + element->layout.max_height) {
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

void click_handler(ElementTree *tree) {
  Element *element = tree->active_element;
  if (element != 0 && element->on_click != 0) {
    element->on_click(tree);
  }
}

void blur_handler(ElementTree *tree) {
  Element *element = tree->active_element;
  if (element != 0 && element->on_blur != 0) {
    element->on_blur(tree);
  }
}

// Recurses through the element children and returns the first element with the given tag
Element *select_element_by_tag(Element *element, u8 tag) {
  if (element->element_tag == tag) {
    return element;
  }
  Array *children = element->children;
  if (children == 0) {
    return 0;
  }
  for (size_t i = 0; i < array_length(children); i++) {
    Element *child = array_get(children, i);
    Element *selected = select_element_by_tag(child, tag);
    if (selected != 0) {
      return selected;
    }
  }
  return 0;
}

#define C9_LAYOUT
#endif