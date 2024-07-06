#ifndef C9_LAYOUT

#include "arena.h" // Arena
#include "array.h" // Array
#include "color.h" // RGBA, C9_Gradient, gradient
#include "string.h" // s8
#include "types.h" // u8, i32

const RGBA C9_default_background_color = 0xFFFFFFFF;
const RGBA C9_default_text_color = 0x000000FF;
const RGBA C9_default_border_color = 0x000000FF;

// Element type
typedef struct {
  u8 container; // Layout container
  u8 label; // Leaf element for label
  u8 button; // Element is clickable
  u8 input; // Element contains text and is editable
} ElementType;

static const ElementType element_type = {
  .container = 0,
  .label = 1,
  .button = 2,
  .input = 3,
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

// Function pointer typedef for onclick.
// The function takes a pointer to the clicked element and a pointer to any other data as arguments.
// Todo: Fix the type of the first pointer
typedef void (*OnClick)(void *, void *);

typedef struct {
  i32 x;
  i32 y;
  i32 max_width; // Flexible width
  i32 max_height; // Flexible height
  i32 scroll_x; // current horizontal scroll
  i32 scroll_y; // current vertical scroll
} LayoutProps;

const LayoutProps empty_layout_props = {
  .x = 0,
  .y = 0,
  .max_width = 0,
  .max_height = 0,
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
  u8 element_type;
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
  OnClick on_click; // Function pointer
  Padding padding;
  Border border;
  i32 border_radius;
  RGBA border_color;
  Array *children; // Flexible array of child elements of type Element
  u8 layout_direction;
  LayoutProps layout; // Props set by the layout engine
} Element;

Element empty_element = {
  .element_type = 0,
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
  .text = {0},
  .text_color = C9_default_text_color,
  .on_click = 0,
  .padding = {0, 0, 0, 0},
  .border = {0, 0, 0, 0},
  .border_radius = 0,
  .border_color = C9_default_border_color,
  .children = 0,
  .layout_direction = 0,
  .layout = {
    .x = 0,
    .y = 0,
    .max_width = 0,
    .max_height = 0,
    .scroll_x = 0,
    .scroll_y = 0,
  },
};

// root element
typedef struct {
  Arena *arena;
  Element *root;
  Element *active_element;
} ElementTree;

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
    .layout = {0, 0, 0, 0, 0, 0},
  };

  // Assign the root element to the tree
  tree->root = root;
  tree->active_element = 0;
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

// Recursively sets width of an element
i32 fill_min_width(Element *element) {
  i32 self_width = element->width > element->min_width ? element->width : element->min_width;
  i32 element_padding = element->padding.left + element->padding.right;
  i32 child_width = element_padding;
  Array *children = element->children;
  if (children == 0) {
    element->layout.max_width = self_width;
    return self_width;
  }
  for (size_t i = 0; i < array_length(children); i++) {
    Element *child = array_get(children, i);
    // Horizontal layout adds widths
    if (element->layout_direction == layout_direction.horizontal) {
      // Add gutter before all elements except the first
      if (i != 0) {
        child_width += element->gutter;
      }
      child_width += fill_min_width(child);
    }
    // Vertical layout uses largest width
    else {
      i32 current_child_width = fill_min_width(child);
      if (child_width < current_child_width) {
        child_width = current_child_width + element_padding;
      }
    }
  }
  if (child_width > self_width) {
    element->layout.max_width = child_width;
    return child_width;
  } else {
    element->layout.max_width = self_width;
    return self_width;
  }
}

// Recursively sets height of an element
i32 fill_min_height(Element *element) {
  i32 self_height = element->height > element->min_height ? element->height : element->min_height;
  i32 element_padding = element->padding.top + element->padding.bottom;
  i32 child_height = element_padding;
  Array *children = element->children;
  if (children == 0) {
    element->layout.max_height = self_height;
    return self_height;
  }
  for (size_t i = 0; i < array_length(children); i++) {
    Element *child = array_get(children, i);
    // Vertical layout adds heights
    if (element->layout_direction == layout_direction.vertical) {
      // Add gutter before all elements except the first
      if (i != 0) {
        child_height += element->gutter;
      }
      child_height += fill_min_height(child);
    }
    // Horizontal layout uses largest height
    else {
      i32 current_child_height = fill_min_height(child);
      if (child_height < current_child_height) {
        child_height = current_child_height + element_padding;
      }
    }
  }
  if (child_height > self_height) {
    element->layout.max_height = child_height;
    return child_height;
  } else {
    element->layout.max_height = self_height;
    return self_height;
  }
}

// Increases width of an element and it's children to fill the parent if element-sizing is set to grow
void fill_max_width(Element *element, i32 max_width) {
  i32 child_width = element->layout.max_width;
  if (element->width == 0) {
    element->layout.max_width = max_width;
  } else {
    element->layout.max_width = element->width;
    max_width = element->width;
  }

  Array *children = element->children;
  if (children == 0) return; // No children

  if (element->layout_direction == layout_direction.horizontal) {
    i32 extra_width = 0;
    if (max_width > child_width) {
      extra_width = max_width - child_width;
    }
    // How many children have flexible width
    i32 grow_count = 0;
    for (size_t i = 0; i < array_length(children); i++) {
      Element *child = array_get(children, i);
      if (child->width == 0) grow_count++;
    }
    // Split extra width between children
    if (extra_width > 0 && grow_count > 0) {
      extra_width = extra_width / grow_count;
    }
    // Set extra width for children
    for (size_t i = 0; i < array_length(children); i++) {
      Element *child = array_get(children, i);
      i32 new_max_width = child->layout.max_width + extra_width;
      fill_max_width(child, new_max_width);
    }
  }
  // If layout direction is vertical, set same width for all children
  else {
    for (size_t i = 0; i < array_length(children); i++) {
      Element *child = array_get(children, i);
      i32 child_width = max_width - element->padding.left - element->padding.right;
      fill_max_width(child, child_width);
    }
  }
}

// Increases height of an element and it's children to fill the parent if element-sizing is set to grow
void fill_max_height(Element *element, i32 max_height) {
  i32 child_height = element->layout.max_height;
  if (element->height == 0) {
    element->layout.max_height = max_height;
  } else {
    element->layout.max_height = element->height;
    max_height = element->height;
  }

  Array *children = element->children;
  if (children == 0) return; // No children

  if (element->layout_direction == layout_direction.vertical) {
    i32 extra_height = 0;
    if (max_height > child_height) {
      extra_height = max_height - child_height;
    }
    // How many children have flexible height
    i32 grow_count = 0;
    for (size_t i = 0; i < array_length(children); i++) {
      Element *child = array_get(children, i);
      if (child->height == 0) grow_count++;
    }
    // Split extra height between children
    if (extra_height > 0 && grow_count > 0) {
      extra_height = extra_height / grow_count;
    }
    // Set extra height for children
    for (size_t i = 0; i < array_length(children); i++) {
      Element *child = array_get(children, i);
      i32 new_max_height = child->layout.max_height + extra_height;
      fill_max_height(child, new_max_height);
    }
  }
  // If layout direction is horizontal, set same height for all children
  else {
    for (size_t i = 0; i < array_length(children); i++) {
      Element *child = array_get(children, i);
      i32 child_height = max_height - element->padding.top - element->padding.bottom;
      fill_max_height(child, child_height);
    }
  }
}

// Recursively sets element x position
i32 set_x(Element *element, i32 x) {
  element->layout.x = x;
  i32 child_x = x + element->layout.scroll_x + element->padding.left;
  Array *children = element->children;
  // No child array is initalized
  if (children == 0) {
    if (element->width > 0) {
      return x + element->width;
    } else {
      return x + element->layout.max_width;
    }
  };
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
  if (element->width > 0) {
    return x + element->width;
  } else {
    return x + element->layout.max_width;
  }
}

// Recursively sets element y position
i32 set_y(Element *element, i32 y) {
  element->layout.y = y;
  i32 child_y = y + element->layout.scroll_y + element->padding.top;
  Array *children = element->children;
  // No child array is initalized
  if (children == 0) {
    if (element->height > 0) {
      return y + element->height;
    } else {
      return y + element->layout.max_height;
    }
  };
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
  if (element->height > 0) {
    return y + element->height;
  } else {
    return y + element->layout.max_height;
  }
}

// Loop through element tree and set LayoutProp dimensions
void set_dimensions(ElementTree *tree, i32 window_width, i32 window_height) {
  fill_min_width(tree->root);
  fill_min_height(tree->root);
  fill_max_width(tree->root, window_width);
  fill_max_height(tree->root, window_height);
  set_x(tree->root, 0);
  set_y(tree->root, 0);
}

i32 get_min_width(Element *element) {
  Array *children = element->children;
  if (children == 0) {
    if(element->width > element->min_width) {
      return element->width;
    } else {
      return element->min_width;
    }
  }
  i32 element_padding = element->padding.left + element->padding.right;
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
}

i32 get_min_height(Element *element) {
  Array *children = element->children;
  if (children == 0) {
    if(element->height > element->min_height) {
      return element->height;
    } else {
      return element->min_height;
    }
  }
  i32 element_padding = element->padding.top + element->padding.bottom;
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
}

#define C9_LAYOUT
#endif