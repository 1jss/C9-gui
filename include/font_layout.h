#ifndef C9_FONT_LAYOUT

#include <stdbool.h> // bool
#include "SDL_ttf.h" // TTF_Font, TTF_SizeUTF8, TTF_MeasureUTF8
#include "arena.h" // Arena
#include "array.h" // Array
#include "element_tree.h" // Element
#include "font.h" // get_font, has_continuation_byte, get_next_character_width
#include "status.h" // status
#include "string.h" // s8
#include "types.h" // i32, u8
#include "types_draw.h" // Position

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

// Splits a string into lines based on a maximum width. Returns an array of strings but does not allocate memory for the new strings but only points to places in the original string.
Array *split_string_by_width(Arena *arena, u8 font_variant, s8 text, i32 max_width) {
  TTF_Font *font = get_font(font_variant);
  Array *lines = array_create_width(arena, sizeof(s8), 4);

  // The text has no width limit
  if (max_width == 0) {
    array_push(lines, &text);
    return lines;
  }

  // The text has no newline characters
  else if (!contains_newline(text)) {
    i32 text_width = 0;
    i32 text_height = 0;
    TTF_SizeUTF8(font, (char *)text.data, &text_width, &text_height);
    // The text fits in the width
    if (text_width <= max_width) {
      array_push(lines, &text);
      return lines;
    }
  }

  // Loop through the text
  i32 start_index = 0;
  while (start_index < text.length) {
    s8 current_line = {
      .data = text.data + start_index,
    };
    i32 character_count = 0; // How many characters fit in the width
    i32 character_width = 0; // Width of the characters in pixels
    TTF_MeasureUTF8(font, (char *)text.data + start_index, max_width, &character_width, &character_count);

    // Step through the characters to break at the last space
    i32 last_space = -1;
    i32 newline_position = -1;
    i32 read_index = start_index;
    while (character_count > 0 && read_index < text.length) {
      if (text.data[read_index] == '\n') {
        newline_position = read_index;
        character_count = 0; // Break the loop
      } else if (text.data[read_index] == ' ') {
        last_space = read_index;
      }
      // Step index by full utf-8 characters
      read_index += 1;
      while (read_index < text.length &&
             has_continuation_byte(text.data[read_index])) {
        read_index += 1;
      }
      character_count -= 1;
    }
    // Break at the first newline
    if (newline_position != -1) {
      current_line.length = newline_position - start_index + 1; // Include the newline
      array_push(lines, &current_line);
      start_index = newline_position + 1;
    }
    // Break at the last space if we are not at string end
    else if (last_space != -1 && read_index < text.length) {
      current_line.length = last_space - start_index + 1; // Include the space
      array_push(lines, &current_line);
      start_index = last_space + 1;
    }
    // No space found, break at the last character
    else {
      current_line.length = read_index - start_index;
      array_push(lines, &current_line);
      start_index = read_index;
    }
  }
  // If the last character is a newline, add an empty line
  if (text.data[text.length - 1] == '\n') {
    s8 empty_line = {
      .data = 0,
      .length = 0,
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
  if (max_width == 0) {
    return font_height;
  }
  // The text has no linebreaks
  else if (!contains_newline(text)) {
    i32 text_width = 0;
    i32 text_height = 0;
    TTF_Font *font = get_font(font_variant);
    TTF_SizeUTF8(font, (char *)text.data, &text_width, &text_height);
    if (text_width <= max_width) {
      return font_height;
    }
  }

  // Loop through the text
  TTF_Font *font = get_font(font_variant);
  i32 start_index = 0;
  i32 rows = 0;
  while (start_index < text.length) {
    i32 character_count = 0;
    i32 character_width = 0;
    TTF_MeasureUTF8(font, (char *)text.data + start_index, max_width, &character_width, &character_count);

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

i32 get_next_character_width(TTF_Font *font, char *text, i32 next_character_position) {
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
  char next_byte_backup = text[next_byte_index];
  text[next_byte_index] = '\0';
  i32 character_width = 0;
  TTF_SizeUTF8(font, &text[character_index], &character_width, NULL);
  // Restore the next byte
  text[next_byte_index] = next_byte_backup;

  return character_width;
}

i32 index_from_x(u8 font_variant, s8 *text, i32 position) {
  TTF_Font *font = get_font(font_variant);
  // Make sure the position and text is valid
  if (position <= 0 || text->length == 0) return 0;

  i32 character_index = 0;
  i32 character_width = 0; // Width of characters in pixels
  i32 character_count = 0; // How many characters fit in the width
  TTF_MeasureUTF8(font, (char *)text->data, position, &character_width, &character_count);

  // Check if position is closer to the next character
  i32 next_character_width = get_next_character_width(font, (char *)text->data, character_count);
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

// Returns a character index from a global position
i32 index_from_position(Position cursor, Element *element) {
  // Relative position
  Position position = {
    .x = cursor.x - element->layout.x - element->padding.left - element->layout.scroll_x,
    .y = cursor.y - element->layout.y - element->padding.top - element->layout.scroll_y,
  };
  if (position.y <= 0) return 0;

  i32 line_height = get_text_line_height(element->font_variant);
  i32 row = position.y / line_height;

  Arena *temp_arena = arena_open(1024);
  // Copy element text to a new string to avoid modifying the original string
  s8 text = string_from_substring(temp_arena, element->input->text.data, 0, element->input->text.length);

  i32 max_width = 0;
  if (element->overflow != overflow_type.scroll &&
      element->overflow != overflow_type.scroll_x) {
    max_width = element->layout.max_width - element->padding.left - element->padding.right;
  }
  // Split the text into lines
  Array *lines = split_string_by_width(temp_arena, element->font_variant, text, max_width);
  i32 number_of_lines = array_length(lines);
  // Return the last character index if the row is out of bounds
  if (row >= number_of_lines) return text.length;

  i32 string_index = 0;
  // Add up length of previous lines
  for (i32 i = 0; i < row; i++) {
    s8 *line = array_get(lines, i);
    if (row < number_of_lines) {
      string_index += line->length;
    }
  }
  // Find the index of the character in the clicked line
  if (row < number_of_lines) {
    s8 *line = array_get(lines, row);
    i32 index = index_from_x(element->font_variant, line, position.x);
    string_index += index;
  }
  arena_close(temp_arena);
  return string_index;
}

// Returns a global position from a character index
Position position_from_index(i32 index, Element *element) {
  Arena *temp_arena = arena_open(512);
  Position position = {0, 0};
  s8 element_text = element->input->text;
  // Copy element text to a new string to avoid modifying the original string
  s8 text = string_from_substring(temp_arena, element_text.data, 0, element_text.length);
  // Text is only one line
  if (element->overflow == overflow_type.scroll ||
      element->overflow == overflow_type.scroll_x) {
    i32 width = 0;
    i32 height = 0;
    TTF_SizeUTF8(get_font(element->font_variant), to_char(text), &width, &height);
    position = (Position){
      .x = element->layout.x + element->padding.left + width,
      .y = element->layout.y + element->padding.top,
    };
  }
  // Text is multiline
  else {
    i32 max_width = element->layout.max_width - element->padding.left - element->padding.right;
    // Split the text into lines
    Array *lines = split_string_by_width(temp_arena, element->font_variant, text, max_width);
    i32 number_of_lines = array_length(lines);
    i32 counting_index = 0;
    i32 row = 0;
    while (row < number_of_lines) {
      s8 *line = array_get(lines, row);
      // Last index of the line is on the next line except for the last line
      if ((index >= counting_index && index < counting_index + line->length) ||
          (index == counting_index + line->length && row == number_of_lines - 1)) {
        s8 substring = string_from_substring(temp_arena, line->data, 0, index - counting_index);
        i32 width = 0;
        TTF_SizeUTF8(get_font(element->font_variant), to_char(substring), &width, 0);
        i32 line_height = get_text_line_height(element->font_variant);
        position = (Position){
          .x = element->layout.x + element->padding.left + width,
          .y = element->layout.y + element->padding.top + row * line_height + line_height / 2,
        };
        row = number_of_lines; // Break the loop
      }
      counting_index += line->length;
      row += 1;
    }
  }
  arena_close(temp_arena);
  return position;
}

#define C9_FONT_LAYOUT
#endif