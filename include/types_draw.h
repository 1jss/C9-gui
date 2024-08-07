#ifndef C9_TYPES_DRAW

// Types common for several steps in the drawing process

#include "types.h" // i32

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

#define C9_TYPES_DRAW
#endif