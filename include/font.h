#ifndef C9_FONT

#include <stdbool.h> // bool
#include "SDL_ttf.h" // TTF_Font, TTF_Init, TTF_OpenFont, TTF_CloseFont, TTF_GetError, TTF_Quit
#include "status.h" // status
#include "types.h" // i32, u8

bool font_initialized = false;
TTF_Font *inter_regular = 0;
TTF_Font *inter_bold = 0;
TTF_Font *inter_small = 0;
TTF_Font *inter_large = 0;

typedef struct {
  u8 regular;
  u8 bold;
  u8 small;
  u8 large;
} FontVariants;

FontVariants font_variant = {
  .regular = 0,
  .bold = 1,
  .small = 2,
  .large = 3,
};

i32 init_font(void) {
  if (TTF_Init()) {
    printf("TTF_Init: %s\n", TTF_GetError());
    return status.ERROR;
  }
  if (inter_regular == 0) {
    inter_regular = TTF_OpenFont("Inter-Regular.ttf", 14);
    if (inter_regular == 0) {
      printf("inter_regular failed: %s\n", TTF_GetError());
      return status.ERROR;
    }
    TTF_SetFontHinting(inter_regular, TTF_HINTING_LIGHT_SUBPIXEL);
  }
  if (inter_bold == 0) {
    inter_bold = TTF_OpenFont("Inter-Medium.ttf", 14);
    if (inter_bold == 0) {
      printf("inter_bold failed: %s\n", TTF_GetError());
      return status.ERROR;
    }
    TTF_SetFontHinting(inter_bold, TTF_HINTING_LIGHT_SUBPIXEL);
  }
  if (inter_small == 0) {
    inter_small = TTF_OpenFont("Inter-Medium.ttf", 12);
    if (inter_small == 0) {
      printf("inter_small failed: %s\n", TTF_GetError());
      return status.ERROR;
    }
    TTF_SetFontHinting(inter_small, TTF_HINTING_LIGHT_SUBPIXEL);
  }
  if (inter_large == 0) {
    inter_large = TTF_OpenFont("Inter-Regular.ttf", 18);
    if (inter_large == 0) {
      printf("inter_large failed: %s\n", TTF_GetError());
      return status.ERROR;
    }
    TTF_SetFontHinting(inter_large, TTF_HINTING_LIGHT_SUBPIXEL);
  }
  return status.OK;
}

TTF_Font *get_font(u8 variant) {
  if (font_initialized == false) {
    if (init_font() == status.OK) {
      font_initialized = true;
    } else {
      return 0; // null pointer
    }
  }
  if (variant == font_variant.regular) {
    return inter_regular;
  }
  if (variant == font_variant.bold) {
    return inter_bold;
  } else if (variant == font_variant.small) {
    return inter_small;
  } else if (variant == font_variant.large) {
    return inter_large;
  } else {
    return inter_regular;
  }
}

bool has_continuation_byte(u8 byte) {
  return (byte & 0b11000000) == 0b10000000;
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

i32 index_from_x(u8 variant, s8 text, i32 position) {
  TTF_Font *font = get_font(variant);
  i32 character_index = 0;
  i32 character_count = 0; // How many characters fit in the width
  i32 character_width = 0; // Width of characters in pixels
  TTF_MeasureUTF8(font, (char *)text.data, position, &character_width, &character_count);

  // Check if position is closer to the next character
  i32 next_character_width = get_next_character_width(font, (char *)text.data, character_count);
  if (position - character_width > next_character_width / 2) {
    character_count += 1;
  }

  // Step by character to find the character index
  for (i32 i = 0; i < character_count; i++) {
    character_index += 1;
    while (has_continuation_byte(text.data[character_index])) {
      character_index += 1;
    }
  }
  return character_index;
}

// Check if a string contains a newline character
bool contains_newline(s8 text) {
  for (i32 i = 0; i < text.length; i++) {
    if (text.data[i] == '\n') {
      return true;
    }
  }
  return false;
}

// Splits a string into lines based on a maximum width
Array *split_string_by_width(Arena *arena, u8 variant, s8 text, i32 max_width) {
  TTF_Font *font = get_font(variant);
  Array *lines = array_create(arena, sizeof(s8));

  // Check for manual linebreaks
  if (!contains_newline(text)) {
    // The text has no width limit
    if (max_width == 0) {
      array_push(lines, &text);
      return lines;
    };
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
  i32 start_position = 0;
  while (start_position < text.length) {
    s8 current_line = {
      .data = text.data + start_position,
    };
    i32 character_count = 0; // How many characters fit in the width
    i32 character_width = 0; // Width of the characters in pixels
    TTF_MeasureUTF8(font, (char *)text.data + start_position, max_width, &character_width, &character_count);

    // Step through the characters to break at the last space
    i32 last_space = -1;
    i32 newline_position = -1;
    i32 next_index = start_position;
    for (i32 i = 0; i < character_count; i++) {
      if (text.data[next_index] == '\n') {
        newline_position = next_index;
        i = character_count; // Break the loop
      } else if (text.data[next_index] == ' ') {
        last_space = next_index;
      }
      // Step index by full utf-8 characters
      next_index += 1;
      while (has_continuation_byte(text.data[next_index])) {
        next_index += 1;
      }
    }
    // Break at the first newline
    if (newline_position != -1) {
      current_line.length = newline_position - start_position;
      array_push(lines, &current_line);
      start_position = newline_position + 1;
    }
    // Break at the last space if we are not at string end
    else if (last_space != -1 && next_index < text.length) {
      current_line.length = last_space - start_position;
      array_push(lines, &current_line);
      start_position = last_space + 1;
    }
    // No space found, break at the last character
    else {
      current_line.length = next_index - start_position;
      array_push(lines, &current_line);
      start_position = next_index;
    }
  }
  return lines;
}

i32 get_font_height(u8 variant) {
  if (variant == font_variant.regular) {
    return 18;
  }
  if (variant == font_variant.bold) {
    return 18;
  } else if (variant == font_variant.small) {
    return 15;
  } else if (variant == font_variant.large) {
    return 23;
  } else {
    return 18;
  }
}

// Returns the height of a text block based on number of rows
i32 get_text_block_height(u8 variant, i32 rows) {
  return get_font_height(variant) * rows + 2 * (rows - 1);
}

// Returns the height of a single text line
i32 get_text_line_height(u8 variant) {
  return get_font_height(variant) + 2;
}

void close_font(void) {
  if (font_initialized == true) {
    TTF_CloseFont(inter_regular);
    TTF_CloseFont(inter_bold);
    TTF_CloseFont(inter_small);
    TTF_CloseFont(inter_large);
    TTF_Quit();
  }
}

#define C9_FONT
#endif