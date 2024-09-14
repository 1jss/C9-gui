#ifndef C9_ELEMENT_TREE

#include <SDL2/SDL.h> // SDL_Texture
#include "arena.h" // Arena
#include "array.h" // Array
#include "color.h" // RGBA, C9_Gradient, gradient
#include "input.h" // InputData
#include "string.h" // s8
#include "types.h" // u8, i32
#include "types_draw.h" // Border, Padding
#include <stdbool.h> // bool

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
  u8 image; // Load image from URL
} BackgroundType;

const BackgroundType background_type = {
  .none = 0,
  .color = 1,
  .horizontal_gradient = 2,
  .vertical_gradient = 3,
  .image = 4,
};

// Text alignment
typedef struct {
  u8 start;
  u8 center;
  u8 end;
} TextAlign;

const TextAlign text_align = {
  .start = 0,
  .center = 1,
  .end = 2,
};

// Forward declaration of ElementTree
struct ElementTree;
typedef struct ElementTree ElementTree;

// Function pointer typedef for on_click and on_blur
// The function takes a pointer to the ElementTree and a void pointer to optional event data
typedef void (*OnEvent)(ElementTree *, void *);

// In most cases an i16 is enough, but for rendering very long documents an i32 is needed.
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

// SDL texture cache for rendering
typedef struct {
  SDL_Texture *texture;
  i32 width;
  i32 height;
  bool changed;
} RenderProps;

typedef struct {
  u8 available;
  u8 active;
  u8 blocked;
} ScrollState;

ScrollState scroll_state = {
  .available = 0,
  .active = 1,
  .blocked = 2,
};

// Scroll handling state
typedef struct {
  f32 last_x; // Last horizontal scroll position
  f32 last_y; // Last vertical scroll position
  u8 state; // If the user is scrolling
} ScrollProps;

// element tree nodes
typedef struct Element {
  LayoutProps layout; // Props set by the layout engine
  RenderProps render; // Cache for renderer
  union {
    RGBA color;
    C9_Gradient gradient;
    s8 image;
  } background;
  s8 text;
  Padding padding;
  Border border;
  InputData *input;
  Array *children; // Flexible array of child elements of type Element
  OnEvent on_click; // Function pointer
  OnEvent on_blur; // Function pointer
  OnEvent on_key_press; // Function pointer
  i32 width;
  i32 height;
  i32 min_width;
  i32 min_height;
  i32 gutter;
  i32 corner_radius;
  RGBA border_color;
  RGBA text_color;
  u8 layout_direction;
  u8 overflow;
  u8 element_tag; // Optional id or group id
  u8 background_type; // Key for background union
  u8 text_align;
  u8 font_variant;
} Element;

Element empty_element = {
  .element_tag = 0,
  .background_type = 0, // No background color
  .background.color = 0xFFFFFFFF,
  .width = 0,
  .height = 0,
  .min_width = 0,
  .min_height = 0,
  .gutter = 0,
  .text = {.data = 0, .length = 0},
  .text_align = 0,
  .font_variant = 0,
  .input = 0,
  .text_color = 0x000000FF,
  .on_click = 0,
  .on_blur = 0,
  .padding = {0, 0, 0, 0},
  .border = {0, 0, 0, 0},
  .corner_radius = 0,
  .border_color = 0x000000FF,
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
  .render = {
    .texture = 0,
    .width = 0,
    .height = 0,
    .changed = 0,
  },
};

// Create a new element and return a pointer to it
Element *new_element(Arena *arena) {
  Element *element = (Element *)arena_fill(arena, sizeof(Element));
  *element = empty_element;
  return element;
}

// Element Tree already typedefed
struct ElementTree {
  Arena *arena;
  Element *root;
  Element *overlay;
  Element *active_element;
  SDL_Texture *target_texture;
  ScrollProps scroll;
  bool rerender;
};

// Create a new element tree and return a pointer to it
ElementTree *new_element_tree(Arena *arena) {
  ElementTree *tree = (ElementTree *)arena_fill(arena, sizeof(ElementTree));
  tree->arena = arena;
  Element *root = new_element(arena);
  root->layout_direction = layout_direction.vertical;
  root->render.changed = true;

  // Assign the root element to the tree
  tree->root = root;
  tree->overlay = 0;
  tree->active_element = 0;
  tree->rerender = true;
  tree->target_texture = 0;
  tree->scroll = (ScrollProps){
    .last_x = 0,
    .last_y = 0,
    .state = 0,
  };
  return tree;
}

// Free all textures in the element tree
void free_textures(Element *element) {
  if (element->render.texture != 0) {
    SDL_DestroyTexture(element->render.texture);
    element->render.texture = 0;
  }
  if (element->children == 0) return;
  for (i32 i = 0; i < array_length(element->children); i++) {
    Element *child = array_get(element->children, i);
    free_textures(child);
  }
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

// Add existing element to a parent
void add_element(Arena *arena, Element *parent, Element *child) {
  // If the parent element has no children, create a new array
  if (parent->children == 0) {
    parent->children = array_create(arena, sizeof(Element));
  }
  // Add a new child element to the parent element
  array_push(parent->children, child);
}

// Recurses through the element children and returns the first element with the given tag
Element *get_element_by_tag(Element *element, u8 tag) {
  if (element->element_tag == tag) {
    return element;
  }
  Array *children = element->children;
  if (children == 0) {
    return 0;
  }
  for (i32 i = 0; i < array_length(children); i++) {
    Element *child = array_get(children, i);
    Element *selected = get_element_by_tag(child, tag);
    if (selected != 0) {
      return selected;
    }
  }
  return 0;
}

// Returns which root layer is currently active
Element *get_root(ElementTree *tree) {
  if (tree->overlay != 0) {
    return tree->overlay;
  }
  return tree->root;
}

// Returns a reference to the direct parent element of the given element. Searches recursively starting from the given root parent.
Element *get_parent(Element *parent, Element *element) {
  Array *children = parent->children;
  if (children == 0) {
    return 0;
  }
  // Loop through the children
  for (i32 i = 0; i < array_length(children); i++) {
    Element *child = array_get(children, i);
    // If child is the element, return the parent
    if (child == element) {
      return parent;
    }
    // Otherwise search for the element in the child
    Element *selected = get_parent(child, element);
    if (selected != 0) {
      return selected;
    }
  }
  return 0;
}

#define C9_ELEMENT_TREE
#endif