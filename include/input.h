#ifndef C9_INPUT

#include <SDL2/SDL.h> // SDL_SetClipboardText, SDL_GetClipboardText
#include <stdbool.h> // bool
#include <string.h> // memcpy, strcmp
#include "SDL_ttf.h" // TTF_Font, TTF_SizeUTF8
#include "arena.h" // Arena
#include "array.h" // Array
#include "string.h" // s8, new_string, insert_into_string, delete_from_string, string_from_substring
#include "font.h" // get_font, font_variant
#include "status.h" // status
#include "types.h" // u8, i32

typedef struct {
  u8 insert;
  u8 delete;
  u8 replace;
} EditActionType;

EditActionType edit_action_type = {
  .insert = 1,
  .delete = 2,
  .replace = 3
};

typedef struct {
  u8 type;
  i32 index;
  s8 text;
  s8 replaced_text;
} EditAction;

typedef struct {
  Array *actions; // Array of EditAction;
  i32 current_index; // Index from 1 to be able to use 0 as a null value
} EditHistory;

typedef struct {
  i32 start_index;
  i32 end_index;
} Selection;

typedef struct {
  s8 text;
  Selection selection;
  EditHistory *history;
  Arena *arena;
} InputData;

EditHistory *new_edit_history(Arena *arena) {
  EditHistory *history = arena_fill(arena, sizeof(EditHistory));
  *history = (EditHistory){
    .actions = array_create(arena, sizeof(EditAction)),
    .current_index = 0
  };
  return history;
}

InputData *new_input(Arena *arena) {
  InputData *input = arena_fill(arena, sizeof(InputData));
  *input = (InputData){
    .text = new_string(arena),
    .selection = (Selection){0, 0},
    .history = new_edit_history(arena),
    .arena = arena
  };
  return input;
}

// EditHistory is an Array of of EditActions
void add_edit_action(EditHistory *history, EditAction action) {
  // Pop all actions after current_index if history size is larger than current index
  while (array_length(history->actions) > history->current_index) {
    array_pop(history->actions);
  }
  // Add the new action at the current end position
  array_push(history->actions, &action);

  // Increment the current index
  history->current_index += 1;
}

bool has_continuation_byte(u8 byte) {
  return (byte & 0b11000000) == 0b10000000;
}

// Returns a reference to start of the selection
i32 *get_start_ref(Selection *selection) {
  return selection->start_index < selection->end_index ? &selection->start_index : &selection->end_index;
}
// Returns a reference to the end of the selection
i32 *get_end_ref(Selection *selection) {
  return selection->start_index < selection->end_index ? &selection->end_index : &selection->start_index;
}

// Cursor movement
void move_cursor_left(InputData *input) {
  i32 *start_index = get_start_ref(&input->selection);
  i32 *end_index = get_end_ref(&input->selection);
  // Move the cursor to the left
  if (*start_index > 0) {
    *start_index = *start_index - 1;
  }
  // Move the cursor to the start of the previous character
  while (*start_index > 0 && has_continuation_byte(input->text.data[*start_index])) {
    *start_index = *start_index - 1;
  }
  *end_index = *start_index;
}
void move_cursor_right(InputData *input) {
  i32 *start_index = get_start_ref(&input->selection);
  i32 *end_index = get_end_ref(&input->selection);
  // Move the cursor to the right
  if (*end_index < input->text.length) {
    *end_index = *end_index + 1;
  }
  // Move the cursor to the start of the next character
  while (*end_index < input->text.length && has_continuation_byte(input->text.data[*end_index])) {
    *end_index = *end_index + 1;
  }
  *start_index = *end_index;
}
// Moves end_index to the right
void select_right(InputData *input) {
  i32 *end_index = &input->selection.end_index;
  if (*end_index < input->text.length) {
    *end_index = *end_index + 1;
  }
  // Keep moving if next character has continuation byte, which means we're still in the same character
  while (*end_index < input->text.length && has_continuation_byte(input->text.data[*end_index])) {
    *end_index = *end_index + 1;
  }
}
// Moves end_index to the left
void select_left(InputData *input) {
  i32 *end_index = &input->selection.end_index;
  if (*end_index > 0) {
    *end_index = *end_index - 1;
  }
  // Keep moving if previous character has continuation byte which means we haven't yet reached the start of the character
  while (*end_index > 0 && has_continuation_byte(input->text.data[*end_index])) {
    *end_index = *end_index - 1;
  }
}
void select_start(InputData *input) {
  input->selection.end_index = 0;
}
void select_end(InputData *input) {
  input->selection.end_index = input->text.length;
}
void select_all(InputData *input) {
  input->selection.start_index = 0;
  input->selection.end_index = input->text.length;
}
void deselect(InputData *input) {
  input->selection.start_index = input->selection.end_index;
}

// Text editing
void replace_text(InputData *input, char *text) {
  i32 *start_index = get_start_ref(&input->selection);
  i32 *end_index = get_end_ref(&input->selection);

  // Find the length of the text to be replaced
  i32 replaced_text_length = *end_index - *start_index;
  s8 replaced_text = string_from_substring(input->arena, input->text.data, *start_index, replaced_text_length);

  // Find the length of the new text
  i32 new_text_length = 0;
  while (text[new_text_length] != '\0') {
    new_text_length++;
  }
  s8 new_text = string_from_substring(input->arena, (u8 *)text, 0, new_text_length);

  // Replace the text by removing the replaced text and then inserting the new text
  delete_from_string(&input->text, *start_index, replaced_text_length);
  if (insert_into_string(input->arena, &input->text, new_text, *start_index) == status.ERROR) {
    // Clean up and abort if the insert fails
    insert_into_string(input->arena, &input->text, replaced_text, *start_index);
    return;
  }

  // Create an edit action
  EditAction action = {
    .type = edit_action_type.replace,
    .index = *start_index,
    .text = new_text,
    .replaced_text = replaced_text
  };

  // Add the edit action to the history
  add_edit_action(input->history, action);
  // Move the selection to the end of the inserted text
  *start_index += new_text.length;
  *end_index = *start_index;
}

void insert_text(InputData *input, char *text) {
  i32 *start_index = get_start_ref(&input->selection);
  i32 *end_index = get_end_ref(&input->selection);
  // If there is a selection, replace the text
  if (*start_index != *end_index) {
    replace_text(input, text);
  } else {
    // Find the length of the new text
    i32 new_text_length = 0;
    while (text[new_text_length] != '\0') {
      new_text_length++;
    }
    s8 new_text = string_from_substring(input->arena, (u8 *)text, 0, new_text_length);

    // Insert the text
    if (insert_into_string(input->arena, &input->text, new_text, *start_index) == status.ERROR) return;

    // Create an edit action
    EditAction action = {
      .type = edit_action_type.insert,
      .index = *start_index,
      .text = new_text
    };
    // Add the edit action to the history
    add_edit_action(input->history, action);
    // Move the selection to the end of the inserted text
    *start_index += new_text.length;
    *end_index = *start_index;
  }
}

void delete_text(InputData *input) {
  i32 *start_index = get_start_ref(&input->selection);
  i32 *end_index = get_end_ref(&input->selection);
  // Check if there is anything to delete
  if (*end_index == 0) {
    return;
  }
  if (*start_index == *end_index) {
    // Move the start index to the left
    if (*start_index > 0) {
      *start_index = *start_index - 1;
    }
    // Move the start index to the start of the character if it is a multi-byte character
    while (*start_index > 0 && has_continuation_byte(input->text.data[*start_index])) {
      *start_index = *start_index - 1;
    }
  }

  // Store the deleted text
  i32 deleted_text_length = *end_index - *start_index;
  s8 deleted_text = string_from_substring(input->arena, input->text.data, *start_index, deleted_text_length);

  // Delete the text
  delete_from_string(&input->text, *start_index, deleted_text_length);

  // Create an edit action
  EditAction action = {
    .type = edit_action_type.delete,
    .index = *start_index,
    .text = {
      .data = NULL,
      .length = 0
    },
    .replaced_text = deleted_text
  };

  // Add the edit action to the history
  add_edit_action(input->history, action);
  // Move the selection to the start index
  *end_index = *start_index;
}

// Undo an action
void undo_action(InputData *input) {
  EditHistory *history = input->history;
  // Check if there are any actions to undo
  if (history->current_index == 0) return;
  EditAction *action = array_get(history->actions, history->current_index - 1);
  // Return if no action is found
  if (action == 0) return;
  i32 *start_index = get_start_ref(&input->selection);
  i32 *end_index = get_end_ref(&input->selection);
  if (action->type == edit_action_type.insert) {
    // Delete the inserted text
    delete_from_string(&input->text, action->index, action->text.length);
    // Set the selection to the start index
    *start_index = action->index;
    *end_index = action->index;
  } else if (action->type == edit_action_type.delete) {
    // Insert the deleted text
    if (insert_into_string(input->arena, &input->text, action->replaced_text, action->index) == status.ERROR) return;

    // Set the selection to the end of the inserted text
    *start_index = action->index + action->replaced_text.length;
    *end_index = action->index + action->replaced_text.length;
  } else if (action->type == edit_action_type.replace) {
    // Replace the replaced text with the text by first removing the replaced text and then inserting the text
    delete_from_string(&input->text, action->index, action->text.length);
    if (insert_into_string(input->arena, &input->text, action->replaced_text, action->index) == status.ERROR) {
      // Clean up and abort if the insert fails
      insert_into_string(input->arena, &input->text, action->replaced_text, action->index);
      return;
    }
    // Set the selection to the end of the inserted text
    *start_index = action->index + action->replaced_text.length;
    *end_index = *start_index;
  }
  // Move current_index backwards
  history->current_index -= 1;
}

// Redo an action
void redo_action(InputData *input) {
  EditHistory *history = input->history;
  // Check if there are any actions to redo
  if (history->current_index == array_length(history->actions)) return;
  EditAction *action = array_get(history->actions, history->current_index);
  // Return if no action is found
  if (action == 0) return;
  i32 *start_index = get_start_ref(&input->selection);
  i32 *end_index = get_end_ref(&input->selection);
  // Perform the redo action for next_index
  if (action->type == edit_action_type.insert) {
    // Add the inserted text
    if (insert_into_string(input->arena, &input->text, action->text, action->index) == status.ERROR) return;
    // Move the selection to the end of the inserted text
    *start_index = action->index + action->text.length;
    *end_index = action->index + action->text.length;
  } else if (action->type == edit_action_type.delete) {
    // Delete the deleted text
    delete_from_string(&input->text, action->index, action->replaced_text.length);
    // Set the selection to the start index
    *start_index = action->index;
    *end_index = action->index;
  } else if (action->type == edit_action_type.replace) {
    // Replace by first removing the replaced_text and then inserting the new text
    delete_from_string(&input->text, action->index, action->replaced_text.length);
    if (insert_into_string(input->arena, &input->text, action->text, action->index) == status.ERROR) {
      // Clean up and abort if the insert fails
      insert_into_string(input->arena, &input->text, action->replaced_text, action->index);
      return;
    }
    // Set the selection to the end of the inserted text
    *start_index = action->index + action->text.length;
    *end_index = *start_index;
  }
  history->current_index += 1;
}

// Copy the selected text to the clipboard
void copy_text(InputData *input) {
  // Get the selection
  i32 *start_index = get_start_ref(&input->selection);
  i32 *end_index = get_end_ref(&input->selection);
  i32 selection_length = *end_index - *start_index;
  i32 selection_size = sizeof(char) * (selection_length + 1); // +1 for null terminator
  // Open a temporary arena
  Arena *temp_arena = arena_open(selection_size);
  char *selection_data = arena_fill(temp_arena, selection_size);
  // Copy the selected text
  memcpy(selection_data, input->text.data + *start_index, selection_length);
  selection_data[selection_length] = '\0';
  // Add to the clipboard
  SDL_SetClipboardText(selection_data);
  // Close the temporary arena
  arena_close(temp_arena);
}

// Cut the selected text
void cut_text(InputData *input) {
  // Check if there is a selection at all
  if (input->selection.start_index != input->selection.end_index) {
    // Copy the selected text to the clipboard
    copy_text(input);
    // Delete the selected text
    delete_text(input);
  }
}

// Paste the copied text
void paste_text(InputData *input) {
  // Get the clipboard text
  char *clipboard_text = SDL_GetClipboardText();
  if (clipboard_text != NULL) {
    // Insert the clipboard text
    insert_text(input, clipboard_text);
    // Free the clipboard text
    SDL_free(clipboard_text);
  }
}

void handle_text_input(InputData *input, char *text) {
  if (strcmp(text, "BACKSPACE") == 0) {
    delete_text(input);
  } else if (strcmp(text, "SELECT_LEFT") == 0) {
    select_left(input);
  } else if (strcmp(text, "SELECT_RIGHT") == 0) {
    select_right(input);
  } else if (strcmp(text, "SELECT_START") == 0) {
    select_start(input);
  } else if (strcmp(text, "SELECT_END") == 0) {
    select_end(input);
  } else if (strcmp(text, "SELECT_ALL") == 0) {
    select_all(input);
  } else if (strcmp(text, "DESELECT") == 0) {
    deselect(input);
  } else if (strcmp(text, "MOVE_LEFT") == 0) {
    move_cursor_left(input);
  } else if (strcmp(text, "MOVE_RIGHT") == 0) {
    move_cursor_right(input);
  } else if (strcmp(text, "UNDO") == 0) {
    undo_action(input);
  } else if (strcmp(text, "REDO") == 0) {
    redo_action(input);
  } else if (strcmp(text, "COPY") == 0) {
    copy_text(input);
  } else if (strcmp(text, "CUT") == 0) {
    cut_text(input);
  } else if (strcmp(text, "PASTE") == 0) {
    paste_text(input);
  } else {
    insert_text(input, text);
  }
}

SDL_Rect measure_selection(TTF_Font *font, InputData *input) {
  i32 start_index = *get_start_ref(&input->selection);
  i32 end_index = *get_end_ref(&input->selection);
  Arena *temp_arena = arena_open(sizeof(char) * input->text.capacity);
  s8 text = input->text;

  // Measure from text start to end of selection
  char *selection_end = arena_fill(temp_arena, end_index + 1); // +1 for null terminator
  memcpy(selection_end, text.data, end_index);
  // Add null-terminator
  selection_end[end_index] = '\0';
  i32 selection_end_x;
  TTF_SizeUTF8(font, selection_end, &selection_end_x, 0);

  // Measure the selected text
  i32 selection_length = end_index - start_index;
  char *selected_text = arena_fill(temp_arena, selection_length + 1); // +1 for null terminator
  memcpy(selected_text, text.data + start_index, selection_length);
  // Add null-terminator
  selected_text[selection_length] = '\0';
  i32 selection_w;
  i32 selection_h;
  TTF_SizeUTF8(font, selected_text, &selection_w, &selection_h);

  arena_close(temp_arena);
  SDL_Rect result = {
    .x = selection_end_x - selection_w,
    .y = 0,
    .w = selection_w,
    .h = selection_h
  };
  return result;
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

// Set selection based on mouse position
void set_selection(InputData *input, i32 relative_mouse_x_position) {
  i32 *start_index = &input->selection.start_index;
  i32 *end_index = &input->selection.end_index;
  TTF_Font *font = get_font(font_variant.regular);
  if (relative_mouse_x_position <= 0) {
    *start_index = 0;
    *end_index = 0;
  } else {
    i32 character_count = 0;
    i32 character_width = 0;
    TTF_MeasureUTF8(font, (char *)input->text.data, relative_mouse_x_position, &character_width, &character_count);
    i32 next_character_width = get_next_character_width(font, (char *)input->text.data, character_count);
    if (relative_mouse_x_position - character_width > next_character_width / 2) {
      character_count++;
    }

    *start_index = 0;
    *end_index = 0;
    for (i32 i = 0; i < character_count; i++) {
      move_cursor_right(input);
    }
  }
}

// Set selection end based on mouse position
void set_selection_end(InputData *input, i32 relative_mouse_x_position) {
  i32 *end_index = &input->selection.end_index;
  TTF_Font *font = get_font(font_variant.regular);
  if (relative_mouse_x_position <= 0) {
    *end_index = 0;
  } else {
    i32 character_count = 0;
    i32 character_width = 0;
    TTF_MeasureUTF8(font, (char *)input->text.data, relative_mouse_x_position, &character_width, &character_count);
    i32 next_character_width = get_next_character_width(font, (char *)input->text.data, character_count);
    if (relative_mouse_x_position - character_width > next_character_width / 2) {
      character_count++;
    }
    *end_index = 0;
    for (i32 i = 0; i < character_count; i++) {
      select_right(input);
    }
  }
}

// Select a word based on mouse position
void select_word(InputData *input, i32 relative_mouse_x_position) {
  i32 *start_index = &input->selection.start_index;
  i32 *end_index = &input->selection.end_index;
  char *text_data = (char *)input->text.data;
  TTF_Font *font = get_font(font_variant.regular);
  i32 character_count = 0;
  i32 character_width = 0;
  TTF_MeasureUTF8(font, text_data, relative_mouse_x_position, &character_width, &character_count);
  // Set the cursor to the mouse selection position
  // This handles UTF8 characters that take several bytes per character
  *start_index = 0;
  *end_index = 0;
  for (i32 i = 0; i < character_count; i++) {
    move_cursor_right(input);
  }
  // Move cursor to the left until we reach a space character
  while (*start_index > 0 &&
         text_data[*start_index - 1] != (char)32) {
    move_cursor_left(input);
  }
  // Select to the right until we reach a space character
  while (*end_index < input->text.length &&
         text_data[*end_index] != (char)32) {
    select_right(input);
  }
}

#define C9_INPUT
#endif