#ifndef C9_GROWING_STRING

#include <string.h> // memcpy
#include "arena.h" // Arena, arena_fill
#include "status.h" // status
#include "types.h" // u8, u32, i32

typedef struct {
  u8 *data;
  u32 length; // Length of the string
  u32 capacity; // Current capacity of the string
} GrowingString;

// Get a growing string with initial space for 32 characters
GrowingString new_string(Arena *arena) {
  u8 *data = arena_fill(arena, sizeof(u8) * 32);
  data[0] = '\0';
  GrowingString string = {
    .data = data,
    .length = 0,
    .capacity = 32,
  };
  return string;
}

// Returns a growing string from a u8 pointer, index and length
GrowingString string_from_substring(Arena *arena, u8 *string, u32 index, u32 length) {
  u8 *data = arena_fill(arena, sizeof(u8) * length + 1);
  // Copy the replaced text
  memcpy(data, string + index, length);
  data[length] = '\0';
  GrowingString result = {
    .data = data,
    .length = length,
    .capacity = length + 1,
  };
  return result;
}

// Inserts a string into another string at a given index
i32 insert_into_string(Arena *arena, GrowingString *target, GrowingString substring, u32 index) {
  if (index > target->length) return status.ERROR;
  // Grow the string if needed
  if (target->length + substring.length + 1 > target->capacity) {
    // Increase the capacity until the new data fits
    while (target->length + substring.length + 1 > target->capacity) {
      target->capacity *= 2;
    }
    // New allocation for the data
    u8 *new_data = arena_fill(arena, sizeof(u8) * target->capacity);
    memcpy(new_data, target->data, target->length);
    new_data[target->length] = '\0';
    target->data = new_data;
  }
  // Signed loop counter so that index can be negative
  for (i32 i = (i32)target->length - 1; i >= (i32)index; --i) {
    target->data[i + substring.length] = target->data[i];
  }
  // Copy the substring onto the now free space
  memcpy(target->data + index, substring.data, substring.length);
  // Update the length
  target->length += substring.length;
  // Add null terminator
  target->data[target->length] = '\0';
  return status.OK;
}

// Deletes a string from another string at a given index
void delete_from_string(GrowingString *target, u32 index, u32 length) {
  // Shift the characters to the right of the deleted text to the left
  for (u32 i = index + length; i < target->length; ++i) {
    target->data[i - length] = target->data[i];
  }
  target->length = target->length - length;
  // Add null terminator
  target->data[target->length] = '\0';
}

#define C9_GROWING_STRING
#endif