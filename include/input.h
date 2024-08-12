#ifndef C9_INPUT

#include <SDL2/SDL.h> // SDL_SetClipboardText, SDL_GetClipboardText
#include <stdbool.h> // bool
#include <string.h> // memcpy, strcmp
#include "SDL_ttf.h" // TTF_Font, TTF_SizeUTF8
#include "arena.h" // Arena
#include "array.h" // Array
#include "fixed_string.h" // FixedString, new_fixed_string, to_fixed_string, insert_fixed_string, delete_fixed_string
#include "font.h" // get_font
#include "types.h" // u8, u32

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
  u32 index;
  FixedString text;
  FixedString replaced_text;
} EditAction;

typedef struct {
  EditAction *actions;
  size_t capacity;
  size_t start;
  size_t end;
  size_t current_index;
} EditHistory;

typedef struct {
  u32 start_index;
  u32 end_index;
} Selection;

typedef struct {
  FixedString text;
  u32 max_length;
  Selection selection;
  EditHistory *history;
  Arena *text_arena;
} InputData;

EditHistory *new_edit_history(Arena *arena, u32 capacity) {
  EditHistory *history = arena_fill(arena, sizeof(EditHistory));
  *history = (EditHistory){
    .actions = arena_fill(arena, sizeof(EditAction) * capacity),
    .capacity = capacity,
    .start = 0,
    .end = 0,
    .current_index = 0
  };
  return history;
}

InputData *new_input(Arena *arena, size_t max_length) {
  InputData *input = arena_fill(arena, sizeof(InputData));
  *input = (InputData){
    .text = new_fixed_string(arena, max_length),
    .max_length = max_length,
    .selection = (Selection){0, 0},
    .history = new_edit_history(arena, 100),
    .text_arena = arena
  };
  return input;
}

// EditHistory is a circular buffer of EditActions
void add_edit_action(EditHistory *history, EditAction action) {
  size_t next_index = (history->current_index + 1) % history->capacity;
  // Overwrite future actions if next is not end
  if (next_index != history->end) {
    history->end = next_index;
  }

  // Add the new action at the current end position
  history->actions[history->end] = action;

  // Increment the end index
  history->end = (history->end + 1) % history->capacity;

  // Increment start if reached by end
  if (history->end == history->start) {
    history->start = (history->start + 1) % history->capacity;
  }

  // Increment the current index
  history->current_index = next_index;
}

bool has_continuation_byte(u8 byte) {
  return (byte & 0b11000000) == 0b10000000;
}

// Returns a reference to start of the selection
u32 *get_start_ref(Selection *selection) {
  return selection->start_index < selection->end_index ? &selection->start_index : &selection->end_index;
}
// Returns a reference to the end of the selection
u32 *get_end_ref(Selection *selection) {
  return selection->start_index < selection->end_index ? &selection->end_index : &selection->start_index;
}

// Cursor movement
void move_cursor_left(InputData *input) {
  u32 *start_index = get_start_ref(&input->selection);
  u32 *end_index = get_end_ref(&input->selection);
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
  u32 *start_index = get_start_ref(&input->selection);
  u32 *end_index = get_end_ref(&input->selection);
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
  u32 *end_index = &input->selection.end_index;
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
  u32 *end_index = &input->selection.end_index;
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
  u32 *start_index = get_start_ref(&input->selection);
  u32 *end_index = get_end_ref(&input->selection);
  // Find the length of the text to be replaced
  u32 replaced_text_length = *end_index - *start_index;
  // Allocate memory for the text
  char *replaced_text_data = arena_fill(input->text_arena, sizeof(char) * replaced_text_length + 1);
  // Copy the replaced text
  memcpy(replaced_text_data, input->text.data + *start_index, replaced_text_length);
  // Add null terminator
  replaced_text_data[replaced_text_length] = '\0';
  FixedString replaced_text = {
    .data = (u8 *)replaced_text_data,
    .length = replaced_text_length
  };

  // Find the length of the new text
  u32 new_text_length = 0;
  while (text[new_text_length] != '\0') {
    new_text_length++;
  }
  // Allocate memory for the text
  char *new_text_data = arena_fill(input->text_arena, sizeof(char) * new_text_length + 1);
  // Copy the new text
  memcpy(new_text_data, text, new_text_length);
  // Add null terminator
  new_text_data[new_text_length] = '\0';
  // Create a fixed string for the new text
  FixedString new_text = {
    .data = (u8 *)new_text_data,
    .length = new_text_length
  };

  // Replace the text by removing the replaced text and then inserting the new text
  delete_fixed_string(&input->text, *start_index, replaced_text_length);
  insert_fixed_string(&input->text, new_text, *start_index);

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
  u32 *start_index = get_start_ref(&input->selection);
  u32 *end_index = get_end_ref(&input->selection);
  // If there is a selection, replace the text
  if (*start_index != *end_index) {
    replace_text(input, text);
  } else {
    // Find the length of the new text
    u32 new_text_length = 0;
    while (text[new_text_length] != '\0') {
      new_text_length++;
    }
    // Allocate memory for the text
    char *new_text_data = arena_fill(input->text_arena, sizeof(char) * new_text_length + 1);
    // Copy the new text
    memcpy(new_text_data, text, new_text_length);
    // Add null terminator
    new_text_data[new_text_length] = '\0';
    // Create a fixed string for the new text
    FixedString new_text = {
      .data = (u8 *)new_text_data,
      .length = new_text_length
    };

    // Insert the text
    insert_fixed_string(&input->text, new_text, *start_index);

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
  u32 *start_index = get_start_ref(&input->selection);
  u32 *end_index = get_end_ref(&input->selection);
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
  u32 deleted_text_length = *end_index - *start_index;
  char *deleted_text_data = arena_fill(input->text_arena, sizeof(char) * deleted_text_length + 1);
  memcpy(deleted_text_data, input->text.data + *start_index, deleted_text_length);
  deleted_text_data[deleted_text_length] = '\0';
  FixedString deleted_text = {
    .data = (u8 *)deleted_text_data,
    .length = deleted_text_length
  };

  // Delete the text
  delete_fixed_string(&input->text, *start_index, deleted_text_length);

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
  if (history->current_index != history->start) {
    u32 *start_index = get_start_ref(&input->selection);
    u32 *end_index = get_end_ref(&input->selection);
    // Perform the undo action for current index
    EditAction action = history->actions[history->current_index];
    if (action.type == edit_action_type.insert) {
      // Delete the inserted text
      delete_fixed_string(&input->text, action.index, action.text.length);
      // Set the selection to the start index
      *start_index = action.index;
      *end_index = action.index;
    } else if (action.type == edit_action_type.delete) {
      // Insert the deleted text
      insert_fixed_string(&input->text, action.replaced_text, action.index);
      // Set the selection to the end of the inserted text
      *start_index = action.index + action.replaced_text.length;
      *end_index = action.index + action.replaced_text.length;
    } else if (action.type == edit_action_type.replace) {
      // Replace the replaced text with the text by first removing the replaced text and then inserting the text
      delete_fixed_string(&input->text, action.index, action.text.length);
      insert_fixed_string(&input->text, action.replaced_text, action.index);
      // Set the selection to the end of the inserted text
      *start_index = action.index + action.replaced_text.length;
      *end_index = *start_index;
    }
    // Move current_index backwards
    if (history->current_index > 0) {
      history->current_index--;
    } else {
      history->current_index = history->capacity - 1;
    }
  }
}

// Redo an action
void redo_action(InputData *input) {
  EditHistory *history = input->history;
  size_t next_index = (history->current_index + 1) % history->capacity;
  if (next_index != history->end) {
    u32 *start_index = get_start_ref(&input->selection);
    u32 *end_index = get_end_ref(&input->selection);
    // Perform the redo action for next_index
    EditAction action = history->actions[next_index];
    if (action.type == edit_action_type.insert) {
      // Insert the inserted text
      insert_fixed_string(&input->text, action.text, action.index);
      // Move the selection to the end of the inserted text
      *start_index = action.index + action.text.length;
      *end_index = action.index + action.text.length;
    } else if (action.type == edit_action_type.delete) {
      // Delete the deleted text
      delete_fixed_string(&input->text, action.index, action.replaced_text.length);
      // Set the selection to the start index
      *start_index = action.index;
      *end_index = action.index;
    } else if (action.type == edit_action_type.replace) {
      // Replace by first removing the replaced_text and then inserting the new text
      delete_fixed_string(&input->text, action.index, action.replaced_text.length);
      insert_fixed_string(&input->text, action.text, action.index);
      // Set the selection to the end of the inserted text
      *start_index = action.index + action.text.length;
      *end_index = *start_index;
    }
    history->current_index = next_index;
  }
}

// Copy the selected text to the clipboard
void copy_text(InputData *input) {
  // Get the selection
  u32 *start_index = get_start_ref(&input->selection);
  u32 *end_index = get_end_ref(&input->selection);
  u32 selection_length = *end_index - *start_index;
  size_t selection_size = sizeof(char) * (selection_length + 1); // +1 for null terminator
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
  // Copy the selected text to the clipboard
  copy_text(input);
  // Delete the selected text
  delete_text(input);
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
  FixedString text = input->text;

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
void set_selection(InputData *input, i32 realative_mouse_x_position) {
  u32 *start_index = &input->selection.start_index;
  u32 *end_index = &input->selection.end_index;
  TTF_Font *font = get_font();
  if (realative_mouse_x_position <= 0) {
    *start_index = 0;
    *end_index = 0;
  } else {
    i32 character_count = 0;
    i32 character_width = 0;
    TTF_MeasureUTF8(font, (char *)input->text.data, realative_mouse_x_position, &character_width, &character_count);
    i32 next_character_width = get_next_character_width(font, (char *)input->text.data, character_count);
    if (realative_mouse_x_position - character_width > next_character_width / 2) {
      character_count++;
    }

    *start_index = 0;
    *end_index = 0;
    for (i32 i = 0; i < character_count; i++) {
      move_cursor_right(input);
    }
  }
}

#define C9_INPUT
#endif