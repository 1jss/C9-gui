#ifndef C9_TYPES_COMMON

// Types common for several steps in the drawing process

#include "types.c" // i16

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

typedef struct {
  i32 start_index;
  i32 end_index;
} Line;

#define C9_TYPES_COMMON
#endif