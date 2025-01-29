#ifndef C9_FONT_LAYOUT

#include <stdbool.h> // bool
#include "arena.c" // Arena
#include "array.c" // Array
#include "element_tree.c" // Element
#include "font.c" // get_sft, get_font_height
#include "schrift.c" // SFT, SFT_MeasureUTF8, SFT_text_width
#include "status.c" // status
#include "string.c" // s8
#include "types.c" // i32, u8
#include "types_common.c" // Position, Line

// Check if a string contains a newline character
bool contains_newline(s8 text) {
  for (i32 i = 0; i < text.length; i++) {
    if (text.data[i] == '\n') {
      return true;
    }
  }
  return false;
}

// Check if a byte has a continuation byte prefix
bool has_continuation_byte(u8 byte) {
  return (byte & 0b11000000) == 0b10000000;
}

// Splits a string into lines based on a maximum width. Returns an array of indexes.
Array *split_string_at_width(Arena *arena, u8 font_variant, s8 text, i32 max_width) {
  SFT *sft = get_sft(font_variant);
  Array *lines = array_create_width(arena, sizeof(s8), 4);

  // The text has no width limit
  if (max_width == 0 || text.length <= 0) {
    Line line = {
      .start_index = 0,
      .end_index = text.length,
    };
    array_push(lines, &line);
    return lines;
  }

  // Loop through the text
  i32 start_index = 0;
  while (start_index < text.length) {
    Line line = {
      .start_index = start_index,
    };

    i32 sft_character_count = 0;
    i32 sft_character_width = 0;
    SFT_MeasureUTF8(sft, text.data + start_index, max_width, &sft_character_width, &sft_character_count);

    // Step through the characters to break at the last space
    i32 last_space = -1;
    i32 newline_position = -1;
    i32 read_index = start_index;
    while (sft_character_count > 0 && read_index < text.length) {
      if (text.data[read_index] == '\n') {
        newline_position = read_index;
        sft_character_count = 0; // Break the loop
      } else if (text.data[read_index] == ' ') {
        last_space = read_index;
      }
      // Step index by full utf-8 characters
      read_index += 1;
      while (read_index < text.length &&
             has_continuation_byte(text.data[read_index])) {
        read_index += 1;
      }
      sft_character_count -= 1;
    }
    // Break at the first newline
    if (newline_position != -1) {
      line.end_index = newline_position;
      array_push(lines, &line);
      start_index = newline_position + 1;
    }
    // Break at the last space if we are not at string end
    else if (last_space != -1 && read_index < text.length) {
      line.end_index = last_space + 1; // Include the space
      array_push(lines, &line);
      start_index = last_space + 1;
    }
    // No space found, break at the last character
    else {
      line.end_index = read_index;
      array_push(lines, &line);
      start_index = read_index;
    }
  }
  // If the last character is a newline, add an empty line
  if (text.data[text.length - 1] == '\n') {
    Line empty_line = {
      .start_index = text.length,
      .end_index = text.length,
    };
    array_push(lines, &empty_line);
  }
  return lines;
}

const i32 line_spacing = 2;

// Returns the height of a single text line
i32 get_text_line_height(u8 font_variant) {
  return get_font_height(font_variant) + line_spacing;
}

// Returns the height of a text block based on a maximum width
i32 get_text_block_height(u8 font_variant, s8 text, i32 max_width) {
  i32 font_height = get_font_height(font_variant);
  // The text has no width limit
  if (max_width == 0 || text.length <= 0) {
    return font_height;
  }

  // Loop through the text
  SFT *font = get_sft(font_variant);
  i32 start_index = 0;
  i32 rows = 0;
  while (start_index < text.length) {
    i32 character_count = 0;
    i32 character_width = 0;
    SFT_MeasureUTF8(font, &text.data[start_index], max_width, &character_width, &character_count);

    // Step through the characters to break at the last space
    i32 last_space = -1;
    i32 newline_position = -1;
    i32 read_index = start_index;
    for (i32 i = 0; i < character_count; i++) {
      if (text.data[read_index] == '\n') {
        newline_position = read_index;
        i = character_count; // Break the loop
      } else if (text.data[read_index] == ' ') {
        last_space = read_index;
      }
      // Step index by full utf-8 characters
      read_index += 1;
      while (has_continuation_byte(text.data[read_index])) {
        read_index += 1;
      }
    }
    // Break at the first newline
    if (newline_position != -1) {
      start_index = newline_position + 1;
    }
    // Break at the last space if we are not at string end
    else if (last_space != -1 && read_index < text.length) {
      start_index = last_space + 1;
    }
    // No space found, break at the last character
    else {
      start_index = read_index;
    }
    rows += 1;
  }
  // If the last character is a newline add one extra row
  if (text.data[text.length - 1] == '\n') {
    rows += 1;
  }
  return font_height * rows + line_spacing * (rows - 1);
}

i32 get_next_character_width(SFT *font, u8 *text, i32 next_character_position) {
  i32 character_index = 0;
  // Step over previous characters to find the start of the character
  for (i32 i = 0; i < next_character_position; i++) {
    character_index++;
    while (has_continuation_byte(text[character_index])) {
      character_index++;
    }
  }
  // Step over the next character and store its length
  i32 character_length = 1;
  while (has_continuation_byte(text[character_index + character_length])) {
    character_length++;
  }

  // Temporarily null-terminate the next byte to measure the character
  i32 next_byte_index = character_index + character_length;
  u8 next_byte_backup = text[next_byte_index];
  text[next_byte_index] = '\0';
  i32 character_width = 0;
  SFT_text_width(font, &text[character_index], &character_width);
  // Restore the next byte
  text[next_byte_index] = next_byte_backup;

  return character_width;
}

i32 index_from_x(u8 font_variant, s8 *text, i32 position) {
  SFT *font = get_sft(font_variant);
  // Make sure the position and text is valid
  if (position <= 0 || text->length == 0) return 0;

  i32 character_index = 0;
  i32 character_width = 0; // Width of characters in pixels
  i32 character_count = 0; // How many characters fit in the width
  SFT_MeasureUTF8(font, text->data, position, &character_width, &character_count);

  // Check if position is closer to the next character
  i32 next_character_width = get_next_character_width(font, text->data, character_count);
  if (position - character_width > next_character_width / 2) {
    character_count += 1;
  }
  // Step by character to find the character index
  for (i32 i = 0; i < character_count; i++) {
    character_index += 1;
    while (has_continuation_byte(text->data[character_index])) {
      character_index += 1;
    }
  }
  if (character_index > text->length) {
    character_index = text->length;
  }
  // If the last character is a newline, step back one character
  if (character_index > 0 && text->data[character_index - 1] == '\n') {
    character_index -= 1;
  }
  // If the character is the last character and it's a space, step back one character
  if (character_index == text->length &&
      text->data[character_index - 1] == ' ') {
    character_index -= 1;
  }
  return character_index;
}

// Binary search to find the child element at coordinate y
i32 get_child_order_at(Element *parent, i32 y) {
  if (parent->children == 0) return -1;
  i32 number_of_children = array_length(parent->children);
  i32 low = 0;
  i32 high = number_of_children - 1;
  while (low <= high) {
    i32 mid = (low + high) / 2;
    Element *child = array_get(parent->children, mid);
    if (y <= child->layout.y + child->layout.max_height) {
      if (mid == 0) {
        return mid;
      }
      Element *previous_child = array_get(parent->children, mid - 1);
      if (y > previous_child->layout.y + previous_child->layout.max_height) {
        return mid;
      }
      high = mid - 1;
    } else {
      low = mid + 1;
    }
  }
  return number_of_children - 1;
}

// Returns a character index from a global position
i32 index_from_position(Position cursor, Element *element) {
  // Relative position
  Position position = {
    .x = cursor.x - element->layout.x - element->padding.left - element->layout.scroll_x,
    .y = cursor.y - element->layout.y - element->padding.top - element->layout.scroll_y,
  };
  if (position.y <= 0) return 0;

  // Find out which line was clicked
  i32 line_number = get_child_order_at(element, cursor.y);
  if (line_number < 0) return 0;

  if (line_number >= array_length(element->input->lines)) {
    printf("Line number out of bounds\n");
    return element->input->text.length;
  }
  // Get the indexes for that line
  Line *indexes = array_get(element->input->lines, line_number);
  Arena *temp_arena = arena_open(1024);
  s8 line_text = {
    .data = element->input->text.data + indexes->start_index,
    .length = indexes->end_index - indexes->start_index,
  };

  i32 index_in_line = index_from_x(element->font_variant, &line_text, position.x);

  arena_close(temp_arena);
  return indexes->start_index + index_in_line;
}

// Returns a global position from a character index
Position position_from_index(i32 index, Element *element) {
  Arena *temp_arena = arena_open(512);
  Position position = {0, 0};

  s8 element_text = string_from_substring(temp_arena, element->input->text.data, 0, element->input->text.length);
  // Text is only one line
  if (element->overflow == overflow_type.scroll ||
      element->overflow == overflow_type.scroll_x) {
    // Copy element text to a new string to avoid modifying the original string
    i32 width = 0;
    SFT_text_width(get_sft(element->font_variant), element_text.data, &width);
    position = (Position){
      .x = element->layout.x + element->padding.left + width,
      .y = element->layout.y + element->padding.top,
    };
  }
  // Text is multiline
  else {
    // Find out which child element contains the index
    Array *indexes = element->input->lines;
    Line *line = 0;
    i32 line_index = 0;
    for (i32 i = 0; i < array_length(indexes); i++) {
      Line *current_line = array_get(indexes, i);
      if (current_line->end_index >= index && current_line->start_index <= index) {
        line = current_line;
        line_index = i;
      }
    }
    if (line == 0) {
      printf("No line found\n");
      line_index = array_last(indexes);
      line = array_get(indexes, line_index);
    }
    if (line == 0 || element->children == 0) return position;

    Element *child_element = array_get(element->children, line_index);
    if (child_element == 0) return position;

    i32 relative_end = index - line->start_index;
    if (relative_end < 0) {
      relative_end = 0;
    }
    s8 line_text = string_from_substring(temp_arena, element_text.data, line->start_index, index - line->start_index);

    i32 width = 0;
    i32 height = get_text_line_height(element->font_variant);
    SFT_text_width(get_sft(element->font_variant), line_text.data, &width);

    position = (Position){
      .x = child_element->layout.x + width,
      .y = child_element->layout.y + height / 2,
    };
  }
  arena_close(temp_arena);
  return position;
}

#define C9_FONT_LAYOUT
#endif