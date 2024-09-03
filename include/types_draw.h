#ifndef C9_TYPES_DRAW

// Types common for several steps in the drawing process

#include "types.h" // i16

typedef struct {
  i16 top;
  i16 right;
  i16 bottom;
  i16 left;
} Padding;

typedef struct {
  i16 top;
  i16 right;
  i16 bottom;
  i16 left;
} Border;

typedef struct {
  i32 x;
  i32 y;
} Position;

#define C9_TYPES_DRAW
#endif