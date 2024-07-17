#ifndef C9_INPUT

#include <stdbool.h> // bool
#include <string.h> // memcpy
#include "SDL_ttf.h" // TTF_Font, TTF_SizeUTF8
#include "arena.h" // Arena
#include "array.h" // Array
#include "fixed_string.h" // s8
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
  u32 index;
  fs8 text;
  fs8 replaced_text;
} EditAction;

typedef struct {
  EditAction *actions;
  size_t capacity;
  size_t start;
  size_t end;
  size_t current_index;
} EditHistory;

typedef struct {
  u32 start_index; // Start index
  u32 end_index; // End index
} Selection;

typedef struct {
  fs8 text;
  u32 max_length;
  Selection selection;
  EditHistory *history;
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
    .text = new_fs8(arena, max_length),
    .max_length = max_length,
    .selection = (Selection){0, 0},
    .history = new_edit_history(arena, 100)
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

// Undo an action
void undoAction(InputData *input) {
  EditHistory *history = input->history;
  if (history->current_index != history->start) {
    // Perform the undo action for current index
    EditAction action = history->actions[history->current_index];
    if (action.type == edit_action_type.insert) {
      // Delete the inserted text
      delete_fs8(&input->text, action.index, action.text.length);
    } else if (action.type == edit_action_type.delete) {
      // Insert the deleted text
      insert_fs8(&input->text, action.text, action.index);
    } else if (action.type == edit_action_type.replace) {
      // Replace the replaced text with the text by first removing the replaced text and then inserting the text
      delete_fs8(&input->text, action.index, action.replaced_text.length);
      insert_fs8(&input->text, action.text, action.index);
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
void redoAction(InputData *input) {
  EditHistory *history = input->history;
  size_t next_index = (history->current_index + 1) % history->capacity;
  if (next_index != history->end) {
    // Perform the redo action for next_index
    EditAction action = history->actions[next_index];
    if (action.type == edit_action_type.insert) {
      // Insert the inserted text
      insert_fs8(&input->text, action.text, action.index);
    } else if (action.type == edit_action_type.delete) {
      // Delete the deleted text
      delete_fs8(&input->text, action.index, action.text.length);
    } else if (action.type == edit_action_type.replace) {
      // Replace the text with the replaced text by first removing the text and then inserting the replaced text
      delete_fs8(&input->text, action.index, action.text.length);
      insert_fs8(&input->text, action.replaced_text, action.index);
    }
    history->current_index = next_index;
  }
}

bool has_continuation_byte(u8 byte) {
  return (byte & 0b11000000) == 0b10000000;
}

// Cursor movement
void move_cursor_left(InputData *input) {
  // Move the cursor to the left
  if (input->selection.start_index > 0) {
    input->selection.start_index--;
  }
  // Move the cursor to the start of the previous character
  while (input->selection.start_index > 0 && has_continuation_byte(input->text.data[input->selection.start_index])) {
    input->selection.start_index--;
  }
  input->selection.end_index = input->selection.start_index;
}
void move_cursor_right(InputData *input) {
  // Move the cursor to the right
  if (input->selection.end_index < input->text.length) {
    input->selection.end_index++;
  }
  // Move the cursor to the start of the next character
  while (input->selection.end_index < input->text.length && has_continuation_byte(input->text.data[input->selection.end_index])) {
    input->selection.end_index++;
  }
  input->selection.start_index = input->selection.end_index;
}
// Text selection
void select_right(InputData *input) {
  if (input->selection.end_index < input->text.length) {
    input->selection.end_index++;
  }
  // Keep moving if next character has continuation byte, which means we're still in the same character
  while (input->selection.end_index < input->text.length && has_continuation_byte(input->text.data[input->selection.end_index])) {
    input->selection.end_index++;
  }
}
void select_left(InputData *input) {
  if (input->selection.start_index > 0) {
    input->selection.start_index--;
  }
  // Keep moving if previous character has continuation byte which means we haven't yet reached the start of the character
  while (input->selection.start_index > 0 && has_continuation_byte(input->text.data[input->selection.start_index])) {
    input->selection.start_index--;
  }
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
  fs8 new_text = to_fs8(text);
  fs8 replaced_text = {
    .data = input->text.data + input->selection.start_index,
    .length = input->selection.end_index - input->selection.start_index
  };
  // Replace the text by removing the replaced text and then inserting the new text
  delete_fs8(&input->text, input->selection.start_index, replaced_text.length);
  insert_fs8(&input->text, new_text, input->selection.start_index);

  // Create an edit action
  EditAction action = {
    .type = edit_action_type.replace,
    .index = input->selection.start_index,
    .text = new_text,
    .replaced_text = replaced_text
  };

  // Add the edit action to the history
  add_edit_action(input->history, action);
  // Move the selection to the end of the inserted text
  input->selection.start_index += new_text.length;
  input->selection.end_index = input->selection.start_index;
}

void insert_text(InputData *input, char *text) {
  // If there is a selection, replace the text
  if (input->selection.start_index != input->selection.end_index) {
    replace_text(input, text);
  } else {
    fs8 new_text = to_fs8(text);
    // Insert the text
    insert_fs8(&input->text, new_text, input->selection.start_index);

    // Create an edit action
    EditAction action = {
      .type = edit_action_type.insert,
      .index = input->selection.start_index,
      .text = new_text
    };

    // Add the edit action to the history
    add_edit_action(input->history, action);
    // Move the selection to the end of the inserted text
    input->selection.start_index += new_text.length;
    input->selection.end_index = input->selection.start_index;
  }
}

void delete_text(InputData *input) {
  // Check if there is anything to delete
  if (input->selection.end_index == 0) {
    return;
  }
  if (input->selection.start_index == input->selection.end_index) {
    // Move the start index to the left
    if (input->selection.start_index > 0) {
      input->selection.start_index--;
    }
    // Move the start index to the start of the character if it is a multi-byte character
    while (input->selection.start_index > 0 && has_continuation_byte(input->text.data[input->selection.start_index])) {
      input->selection.start_index--;
    }
  }
  // Delete the text
  delete_fs8(&input->text, input->selection.start_index, input->selection.end_index - input->selection.start_index);

  // Create an edit action
  EditAction action = {
    .type = edit_action_type.delete,
    .index = input->selection.start_index,
    .text = to_fs8("")
  };

  // Add the edit action to the history
  add_edit_action(input->history, action);
  // Move the selection to the start index
  input->selection.end_index = input->selection.start_index;
}

void handle_text_input(InputData *input, char *text) {
  if (strcmp(text, "BACKSPACE") == 0) {
    delete_text(input);
  } else if (strcmp(text, "SELECT_LEFT") == 0) {
    select_left(input);
  } else if (strcmp(text, "SELECT_RIGHT") == 0) {
    select_right(input);
  } else if (strcmp(text, "SELECT_ALL") == 0) {
    select_all(input);
  } else if (strcmp(text, "DESELECT") == 0) {
    deselect(input);
  } else if (strcmp(text, "MOVE_LEFT") == 0) {
    move_cursor_left(input);
  } else if (strcmp(text, "MOVE_RIGHT") == 0) {
    move_cursor_right(input);
  } else if (strcmp(text, "UNDO") == 0) {
    undoAction(input);
  } else if (strcmp(text, "REDO") == 0) {
    redoAction(input);
  } else {
    insert_text(input, text);
  }
}

SDL_Rect measure_selection(TTF_Font *font, InputData *input) {
  Arena *temp_arena = arena_open(sizeof(char) * input->text.capacity);
  u32 start_index = input->selection.start_index;
  u32 end_index = input->selection.end_index;
  fs8 text = input->text;

  // Measure the text before the selection
  char *before_selection = arena_fill(temp_arena, start_index + 1); // +1 for null terminator
  memcpy(before_selection, text.data, start_index);
  // Add null-terminator
  before_selection[start_index] = '\0';
  i32 selection_x;
  TTF_SizeUTF8(font, before_selection, &selection_x, 0);

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
    .x = selection_x,
    .y = 0,
    .w = selection_w,
    .h = selection_h
  };
  return result;
}

i32 get_next_character_width(TTF_Font *font, char *text, i32 start_index) {
  Arena *temp_arena = arena_open(sizeof(char) * 5); // 4 bytes for character + 1 byte for null terminator
  char *character = arena_fill(temp_arena, 5);
  i32 source_position = 0;
  // Step over previous characters
  for (i32 i = 0; i < start_index; i++) {
    source_position++;
    while (has_continuation_byte(text[source_position])) {
      source_position++;
    }
  }
  // Set the first character
  i32 target_position = 0;
  character[target_position] = text[source_position];
  source_position++;
  target_position++;
  while (has_continuation_byte(text[source_position])) {
    character[1] = text[source_position];
    source_position++;
    target_position++;
  }
  character[target_position] = '\0';
  i32 character_width = 0;
  TTF_SizeUTF8(font, character, &character_width, 0);
  arena_close(temp_arena);
  return character_width;
}

// Set selection based on mouse position
void set_selection(TTF_Font *font, InputData *input, i32 realative_mouse_x_position) {
  if (realative_mouse_x_position <= 0) {
    input->selection.start_index = 0;
    input->selection.end_index = 0;
  } else {
    i32 character_count = 0;
    i32 character_width = 0;
    TTF_MeasureUTF8(font, (char *)input->text.data, realative_mouse_x_position, &character_width, &character_count);
    i32 next_character_width = get_next_character_width(font, (char *)input->text.data, character_count);
    printf("Character count: %d\n", character_count);
    printf("Character width: %d\n", character_width);
    printf("Next character width: %d\n", next_character_width);
    if (realative_mouse_x_position - character_width > next_character_width / 2) {
      character_count++;
    }

    input->selection.start_index = 0;
    input->selection.end_index = 0;
    for (i32 i = 0; i < character_count; i++) {
      move_cursor_right(input);
    }
  }
}

#define C9_INPUT
#endif
