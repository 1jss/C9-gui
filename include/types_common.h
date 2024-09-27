#ifndef C9_TYPES_COMMON

// Types common for several steps in the drawing process

#include "types.h" // i16

typedef struct {
  u8 top;
  u8 right;
  u8 bottom;
  u8 left;
} Padding;

typedef struct {
  u8 top;
  u8 right;
  u8 bottom;
  u8 left;
} Border;

typedef struct {
  i32 x;
  i32 y;
} Position;

#define C9_TYPES_COMMON
#endif