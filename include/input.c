#ifndef C9_INPUT

#include "arena.c" // Arena
#include "array.c" // Array, array_create, array_clear
#include "string.c" // s8, new_string
#include "types.c" // u8, i32
#include "types_common.c" // Line

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
  Array *lines;
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
    .lines = array_create(arena, sizeof(Line)),
    .arena = arena
  };
  return input;
}

void clear_input(InputData *input) {
  input->text.length = 0;
  input->text.data[0] = '\0';
  input->selection = (Selection){0, 0};
  input->history->current_index = 0;
  array_clear(input->history->actions);
  array_clear(input->lines);
}

#define C9_INPUT
#endif