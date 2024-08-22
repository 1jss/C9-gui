#ifndef C9_STRING

#include <stdbool.h> // bool
#include <stdio.h> // printf
#include "arena.h" // Arena, arena_fill
#include "types.h" // u8, u32, i32

#if 0

This header defines a string type and helper functions for it. The string type is implemented like a simple struct, ie no pointer tricks. The creative helper functions, like copy and replace are returning new strings, not modifying the input strings. These functions also take an arena as an argument, where it allocates memory for the new string. The same arena can preferably be used for many strings and be freed all at once.
- s8: a struct that represents a string
- to_s8: a function that converts a char array to an s8
- print_s8: a function that prints an s8
- equal_s8: a function that compares two s8s
- indexof_s8: a function that finds the index of a substring in an s8
- includes_s8: a function that finds a substring in an s8
- concat_s8: a function that concatenates two s8 strings
- concat3_s8: a function that concatenates three s8 strings
- replace_s8: a function that replaces all occurrences of a substring in an s8 string

#endif

const i32 INVALID_STRING_INDEX = -1;

typedef struct {
  u8 *data;
  i32 length;
} s8;

s8 to_s8(char *string) {
  i32 length = 0;
  while (string[length] != '\0') {
    length++;
  }
  return (s8){
    .data = (u8 *)string,
    .length = length
  };
}

char *to_char(s8 string) {
  return (char *)(string.data);
}

void print_s8(s8 string) {
  for (i32 i = 0; i < string.length; i++) {
    printf("%c", string.data[i]);
  }
}

bool equal_s8(s8 a, s8 b) {
  if (a.length != b.length) return false;
  for (i32 i = 0; i < a.length; i++) {
    if (a.data[i] != b.data[i]) {
      return false;
    }
  }
  return true;
}

i32 indexof_s8(s8 source, s8 target) {
  if (source.length < target.length) return INVALID_STRING_INDEX;
  for (i32 i = 0; i < source.length - target.length; i++) {
    bool found = true;
    for (i32 j = 0; j < target.length; j++) {
      if (source.data[i + j] != target.data[j]) {
        found = false;
        break;
      }
    }
    if (found) return i;
  }
  return INVALID_STRING_INDEX;
}

bool includes_s8(s8 source, s8 target) {
  return indexof_s8(source, target) != INVALID_STRING_INDEX;
}

// Concatenate two strings
s8 concat_s8(Arena *arena, s8 a, s8 b) {
  i32 total_length = a.length + b.length;
  u8 *data = (u8 *)arena_fill(arena, total_length);
  for (i32 i = 0; i < a.length; i++) {
    data[i] = a.data[i];
  }
  for (i32 i = 0; i < b.length; i++) {
    data[a.length + i] = b.data[i];
  }
  return (s8){data, total_length};
}

// Concatenate three strings (for internal use only)
static s8 concat3_s8(Arena *arena, s8 a, s8 b, s8 c) {
  i32 total_length = a.length + b.length + c.length;
  u8 *data = (u8 *)arena_fill(arena, total_length);
  for (i32 i = 0; i < a.length; i++) {
    data[i] = a.data[i];
  }
  for (i32 i = 0; i < b.length; i++) {
    data[a.length + i] = b.data[i];
  }
  for (i32 i = 0; i < c.length; i++) {
    data[a.length + b.length + i] = c.data[i];
  }
  return (s8){
    .data = data,
    .length = total_length
  };
}

s8 replace_s8(Arena *arena, s8 source, s8 target, s8 replacement) {
  i32 index = indexof_s8(source, target);
  if (index == INVALID_STRING_INDEX) return source;
  s8 prefix = {
    .data = source.data,
    .length = index
  };
  s8 rest = {
    .data = source.data + index + target.length,
    .length = source.length - index - target.length
  };
  s8 suffix = replace_s8(arena, rest, target, replacement);
  return concat3_s8(arena, prefix, replacement, suffix);
}

#define C9_STRING
#endif