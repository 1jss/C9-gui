#ifndef C9_FIXED_STRING

#include "arena.h" // Arena, arena_fill
#include "types.h" // u8, i32

typedef struct {
  u8 *data;
  u32 length;
  u32 capacity;
} fs8;

// Returns a fixed string from a char pointer
fs8 to_fs8(char *string) {
  size_t length = 0;
  while (string[length] != '\0') {
    length++;
  }
  fs8 result = {
    .data = (u8 *)string,
    .length = length,
    .capacity = length
  };
  return result;
}

// Get a fixed string with place for a number of characters
fs8 new_fs8(Arena *arena, u32 max_length) {
  u8 *data = arena_fill(arena, sizeof(u8) * max_length);
  data[0] = '\0';
  fs8 string = {
    .data = data,
    .length = 0,
    .capacity = max_length
  };
  return string;
}

// Inserts a string into another string at a given index
void insert_fs8(fs8 *source, fs8 target, u32 index) {
  if (index > source->length ||
      source->length + target.length + 1 > source->capacity) {
    return;
  }
  // Ensure the loop is safe for when source->length is 0
  // Convert loop counter to signed and loop condition to check against signed version of index
  for (int i = (int)source->length - 1; i >= (int)index; --i) {
    source->data[i + target.length] = source->data[i];
  }
  // Copy the target onto the now free space
  for (u32 i = 0; i < target.length; ++i) {
    source->data[index + i] = target.data[i];
  }
  // Update the length
  source->length += target.length;
  // Add null terminator
  source->data[source->length] = '\0';
}

// Deletes a string from another string at a given index
void delete_fs8(fs8 *source, u32 index, u32 length) {
  // Shift the characters to the right of the deleted text to the left
  for (u32 i = index + length; i < source->length; ++i) {
    source->data[i - length] = source->data[i];
  }
  source->length = source->length - length;
  // Add null terminator
  source->data[source->length] = '\0';
}

#define C9_FIXED_STRING
#endif