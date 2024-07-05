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

// Element sizing
typedef struct {
  u8 none;
  u8 fixed; // Uses element width and height
  u8 shrink; // Uses the size of children
  u8 grow; // Fills the remaining space
} ElementSizing;

const ElementSizing element_sizing = {
  .none = 0,
  .fixed = 1,
  .shrink = 2,
  .grow = 3,
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
  i32 scroll_width; // combined width of children
  i32 scroll_height; // combined height of children
  i32 scroll_x; // current horizontal scroll
  i32 scroll_y; // current vertical scroll
} LayoutProps;

const LayoutProps empty_layout_props = {
  .x = 0,
  .y = 0,
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
  u8 element_type;
  u8 background_type;
  RGBA background_color;
  C9_Gradient background_gradient;
  u8 element_sizing;
  i32 width;
  i32 height;
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
  .element_sizing = 0,
  .width = 0,
  .height = 0,
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
    .scroll_width = 0,
    .scroll_height = 0,
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
    .element_sizing = element_sizing.grow,
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
i32 set_width(Element *element) {
  i32 self_width = element->width;
  i32 element_padding = element->padding.left + element->padding.right;
  i32 child_width = element_padding;
  Array *children = element->children;
  // No child array is initalized
  if (children == 0) return self_width;
  for (size_t i = 0; i < array_length(children); i++) {
    Element *child = array_get(children, i);
    // Horizontal layout adds widths
    if (element->layout_direction == layout_direction.horizontal) {
      // Add gutter before all elements except the first
      if (i != 0) {
        child_width += element->gutter;
      }
      child_width += set_width(child);
    }
    // Vertical layout uses largest width
    else {
      i32 current_child_width = set_width(child);
      if (child_width < current_child_width) {
        child_width = current_child_width + element_padding;
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
i32 set_height(Element *element) {
  i32 self_height = element->height;
  i32 element_padding = element->padding.top + element->padding.bottom;
  i32 child_height = element_padding;
  Array *children = element->children;
  // No child array is initalized
  if (children == 0) return self_height;
  for (size_t i = 0; i < array_length(children); i++) {
    Element *child = array_get(children, i);
    // Vertical layout adds heights
    if (element->layout_direction == layout_direction.vertical) {
      // Add gutter before all elements except the first
      if (i != 0) {
        child_height += element->gutter;
      }
      child_height += set_height(child);
    }
    // Horizontal layout uses largest height
    else {
      i32 current_child_height = set_height(child);
      if (child_height < current_child_height) {
        child_height = current_child_height + element_padding;
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

// Recursively sets element x position
i32 set_x(Element *element, i32 x) {
  element->layout.x = x;
  i32 child_x = x + element->layout.scroll_x + element->padding.left;
  Array *children = element->children;
  // No child array is initalized
  if (children == 0) return x + element->width;
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
  if (element->element_sizing == element_sizing.fixed) {
    return x + element->width;
  } else {
    return x + element->layout.scroll_width;
  }
}

// Recursively sets element y position
i32 set_y(Element *element, i32 y) {
  element->layout.y = y;
  i32 child_y = y + element->layout.scroll_y + element->padding.top;
  Array *children = element->children;
  // No child array is initalized
  if (children == 0) return y + element->height;
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
  if (element->element_sizing == element_sizing.fixed) {
    return y + element->height;
  } else {
    return y + element->layout.scroll_height;
  }
}

// Loop through element tree and set LayoutProp dimensions
void set_dimensions(ElementTree *tree) {
  set_width(tree->root);
  set_height(tree->root);
  set_x(tree->root, 0);
  set_y(tree->root, 0);
}

#define C9_LAYOUT
#endif