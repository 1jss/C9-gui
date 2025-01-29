#ifndef C9_ARRAY

#include <string.h> // memcpy

#include "types.c" // i32
#include "arena.c" // Arena, arena_fill

/*

Dynamic array implementation that has the following functions:
 - array_create: initializes the array with an item size and returns a pointer to it
 - array_create_width: initializes the array with an item size and a given index width and returns a pointer to it
 - array_push: adds an element to the array
 - array_pop: removes the last element from the array
 - array_get: returns the element at the given index
 - array_set: sets the element at the given index
 - array_length: returns the used size of the array
 - array_last: returns the last index of the array

The array index is implemented around a tree structure to allow for fast access and insertion. Each layer of the tree has a width of index_width and the depth is determined by the number of items in the array.

Consider an example where 'index_width' is 10. The tree would look something like this:

Layer 1: 0 1 2 3 4 5 6 7 8 9
Each of these nodes can have up to 10 children:

Layer 2: 0 1 2 3 4 5 6 7 8 9 (for each of the nodes in layer 1)
And each of those nodes can have up to 10 children, and so on:

Layer 3: 0 1 2 3 4 5 6 7 8 9 (for each of the nodes in layer 2)

The indexes are stored in reverse order, meaning that the largest indexes are deepest into the tree.

For example:
- The index 198 would be stored at positions 8 -> 9- > 1 in layers 1, 2, and 3
- The index 42 would be stored at positions 2 -> 4 in layers 1 and 2
- The index 1024 would be stored at positions 4 -> 2 -> 0 -> 1 in layers 1, 2, 3, and 4

The tree structure grows as more items are added to the array, adding more layers as needed.

The array index is stored in an arena allocator to allow for fast allocation and growth without needing to free memory on every pop or set operation. All items added to the array are copied to the array's arena, so the original data can be safely disposed of after adding it to the array.

*/

const i32 DEFAULT_INDEX_WIDTH = 8;
const i32 INVALID_ARRAY_INDEX = -1;

typedef struct IndexNode IndexNode;
struct IndexNode {
  IndexNode **children; // Points to an array of children
  void *item;
};

// item_size and index_widht do not need to be i32, but the alignment is 8 bytes, so the struct will be 32 bytes even with i16
typedef struct {
  Arena *arena;
  IndexNode *index; // Index tree of all items
  i32 length; // Number of items currently in the array
  i32 item_size; // Size of each item in the array
  i32 index_width; // Number of children each node can have
  i32 allocated; // Number of items we have allocated item memory for
} Array;

// Create a new index node and return a pointer to it
static IndexNode *index_create(Arena *arena) {
  IndexNode *index = (IndexNode *)arena_fill(arena, sizeof(IndexNode));
  index->children = 0;
  index->item = 0;
  return index;
}

typedef struct {
  Arena *arena;
  IndexNode *indexNode;
  i32 index;
  i32 index_width;
  void *item;
} IndexSetParams;

// Set item at the given index
static void index_set(IndexSetParams params) {
  // If the indexNode is 0 there is nothing to add to
  if (params.indexNode == 0) {
    return;
  }
  // If the index is 0 we are at the leaf and should set the data
  else if (params.index == 0) {
    params.indexNode->item = params.item;
    return;
  }
  // Otherwise we need to go deeper into the tree
  else {
    i32 digit = params.index % params.index_width;
    i32 next_index = params.index / params.index_width;
    // If the children node does not exist, create it
    if (params.indexNode->children == 0) {
      params.indexNode->children = (IndexNode **)arena_fill(params.arena, params.index_width * sizeof(IndexNode *));
      for (i32 i = 0; i < params.index_width; i++) {
        params.indexNode->children[i] = 0;
      }
    }
    // If the child indexNode at position digit does not exist, create it
    if (params.indexNode->children[digit] == 0) {
      params.indexNode->children[digit] = index_create(params.arena);
    }
    IndexSetParams next_params = {
      .arena = params.arena,
      .indexNode = params.indexNode->children[digit],
      .index = next_index,
      .index_width = params.index_width,
      .item = params.item
    };
    // Continue to the next node
    index_set(next_params);
  }
}

typedef struct {
  IndexNode *indexNode;
  i32 index;
  i32 index_width;
} IndexGetParams;

// Get the item at the given index
static void *index_get(IndexGetParams params) {
  // If the indexNode is 0 there is nothing to get from
  if (params.indexNode == 0) {
    return 0;
  }
  // If the index is 0 we are at the leaf and should return the data
  else if (params.index == 0) {
    return params.indexNode->item;
  }
  // Otherwise we need to go deeper
  else {
    i32 digit = params.index % params.index_width;
    i32 next_index = params.index / params.index_width;
    IndexGetParams next_params = {
      .indexNode = params.indexNode->children[digit],
      .index = next_index,
      .index_width = params.index_width
    };
    return index_get(next_params);
  }
}

// Create a new array width a given item size and index width and return a pointer to it
// Item size is the size of each item in the array
// Index width is the number of children each node can have.
// The optimal value is determined by the number of items in the array.
Array *array_create_width(Arena *arena, i32 item_size, i32 index_width) {
  Array *new_array = (Array *)arena_fill(arena, sizeof(Array));
  new_array->arena = arena;
  new_array->index = index_create(arena);
  new_array->length = 0;
  new_array->allocated = 0;
  new_array->item_size = item_size;
  new_array->index_width = index_width;
  return new_array;
}

// Create a new default array and return a pointer to it
Array *array_create(Arena *arena, i32 item_size) {
  return array_create_width(arena, item_size, DEFAULT_INDEX_WIDTH);
}

// Copy data onto the array and add it to the last position
// The data has to be pushed by reference as that's the only type agnostic way to pass values
void array_push(Array *array, void *data) {
  // If there is already space in the array, we can just copy the new data directly to the last index
  if (array->length < array->allocated) {
    IndexGetParams get_params = {
      .indexNode = array->index,
      .index = array->length,
      .index_width = array->index_width
    };
    void *item = index_get(get_params);
    if (item == 0) return; // Item is null
    // Overwrite data in item
    memcpy(item, data, array->item_size);
    // Increase the length of the array
    array->length += 1;
  }
  // Otherwise we will need to allocate more memory
  else {
    void *item = arena_fill(array->arena, array->item_size);
    if (item == 0) return; // Allocation failed
    memcpy(item, data, array->item_size);
    // Add the item to the index
    IndexSetParams set_params = {
      .arena = array->arena,
      .indexNode = array->index,
      .index = array->length,
      .index_width = array->index_width,
      .item = item
    };
    index_set(set_params);
    array->allocated += 1;
    array->length += 1;
  }
}

// Get the data from the last item and decrease the length counter
// The data has to be returned by reference as that's the only type agnostic way to return a value
void *array_pop(Array *array) {
  // No items to pop
  if (array->length == 0) return 0;
  // Decrease the length of the array
  array->length -= 1;
  // Return the last item
  IndexGetParams get_params = {
    .indexNode = array->index,
    .index = array->length,
    .index_width = array->index_width
  };
  void *data = index_get(get_params);
  return data;
}

// Get the data at the given index starting from 0
void *array_get(Array *array, i32 index) {
  if (index < 0 || index >= array->length) return 0;
  IndexGetParams get_params = {
    .indexNode = array->index,
    .index = index,
    .index_width = array->index_width
  };
  return index_get(get_params);
}

// Set the data at the given index starting from 0
void array_set(Array *array, i32 index, void *data) {
  if (index >= array->length) return;
  IndexGetParams get_params = {
    .indexNode = array->index,
    .index = index,
    .index_width = array->index_width
  };
  void *item = index_get(get_params);
  if (item == 0) return; // Failed to get item
  memcpy(item, data, array->item_size);
}

// Return the used size of the array
i32 array_length(Array *array) {
  return array->length;
}

// Get last index of the array
// This returns -1 if the array is empty
i32 array_last(Array *array) {
  // If the array is empty
  if (array->length == 0) return INVALID_ARRAY_INDEX;
  return array->length - 1;
}

// Resets the length counter, which overwrites the old data when pushing new data to the array, reusing the allocated memory
void array_clear(Array *array) {
  array->length = 0;
}

#define C9_ARRAY
#endif