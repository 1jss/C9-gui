# C9 gui

C9 gui is a performant and flexible GUI library built on SDL2. C9 gui primarily targets the [C9 language](https://github.com/1jss/C9-lang), which is a C99 subset.

## Features
- Flexible element tree structure (with dynamic loading of children)
- Padding, border and gutter around elements and between children
- Superellipse rounded corners (with anti-aliasing)
- Gradient backgrounds (smoothed with blue noise dithering) 
- Image backgrounds (png, jpg, bmp)
- Event handling (click, blur, key press)
- Layout engine (flex or scroll children in any direction)
- Lazy loading of elements outside of window
- Text rendering (TTF)
- Easy table layout (with automatic column sizing)
- Text input (single line with selection and undo history)
- Partial rendering (only rerender elements that have changed)
- Buffered rendering (all elements are cached as textures)
- Element tags for easy element selection

## Screenshots
![image_1](/screenshots/Screenshot_240812_1.png?raw=true)
![image_2](/screenshots/Screenshot_240812_2.png?raw=true)

## Architecture

### Element tree
C9 gui uses a tree structure to store the "document model". The model contains references (pointers) to the root element, the currently selected element, the next element to rerender (if any), the rerender type and the arena where all allocations for the tree are made.

Every node in the tree is an element that can have their own children. The list of children is a flexible array, so the number of children (and thus the tree itself), can be changed at runtime.

### Element

An element is a struct that contains all the properties of a child. The structure is initalized with default values, so only the changed or used properties need to be set. In other words, all items are optional.

| Type          | Name                  | Comment                             |
|---------------|-----------------------|-------------------------------------|
| `u8`          | `element_tag`         | id or group id                      |
| `i32`         | `width`               | fixed width of the element          |
| `i32`         | `height`              | fixed height of the element         |
| `i32`         | `min_width`           | minimum width of the element        |
| `i32`         | `min_height`          | minimum height of the element       |
| `Padding`     | `padding`             | padding inside element (4 values)   |
| `Border`      | `border`              | border around element (4 values)    |
| `i32`         | `corner_radius`       | radius of superellipse corners      |
| `i32`         | `gutter`              | space between children              |
| `u8`          | `overflow`            | contain or scroll children          |
| `u8`          | `layout_direction`    | direction of flex layout            |
| `u8`          | `background_type`     | none, color, gradient, image        |
| `RGBA`        | `background.color`    | used if background_type is color    |
| `C9_Gradient` | `background.gradient` | used if background_type is gradient |
| `s8`          | `background.image`    | used if background_type is image    |
| `RGBA`        | `text_color`          | color of rendered text              |
| `RGBA`        | `border_color`        | color of border                     |
| `s8`          | `text`                | text label                          |
| `u8`          | `text_align`          | horizontal text alignment           |
| `InputData*`  | `input`               | text input object (new_input)       |
| `Array*`      | `children`            | flexible array of child elements    |
| `OnEvent`     | `on_click`            | function pointer called on click    |
| `OnEvent`     | `on_blur`             | function pointer called on blur     |
| `OnEvent`     | `on_key_press`        | function pointer called on input    |
| `LayoutProps` | `layout`              | props set by the layout engine      |
| `RenderProps` | `render`              | texture cache for the renderer      |

New elements can be created in two ways. Either as children of an existing element(`add_new_element`) or as a standalone element(`new_element`). The standalone element can then dynamically be added to an element in the tree by calling `add_element`.

### Event handling
Events are handled in the main loop and if an element has an event handler set for the event type, it will be called.

If the event is a mouse down event, the element that is both under the pointer and has a on_click function will be set as active element and the on_click function will be called. The formerly active element will have it's on_blur function called. If the element has an input object, the cursor will be set at the position of the mouse pointer.

If the event is a key press event, the active element will have its on_key_press function called, if it has one.

If the event is a scroll event, the entire tree will be searched for scrollable elements under the pointer and the scroll event will be applied to them, starting with the outermost element, so that children get scrolled before parents. The active element is not changed on scroll.

### Rendering
The interface is only rendered when the `rerender` member of the element tree is set to either all or selected. The render function will then traverse the tree and redraw all child elements of either the root element or the selected element. All element are cached as textures, so only the elements that have new dimensions or are marked as changed will be rerendered from scratch. This means that scrolling and moving elements around is very efficient.

### Components
Components are reusable standalone elements that can dynamically be added and removed from the tree. They are implemented as global Element references (pointers) that get initalized on their first use. This way no more memory is used than needed and the already initalized component can be removed and readded to the tree without loosing its state and rendering cache.

## Project navigation
The project starts from `main.c` and all C9 includes are header only. The base library implementation is located in the `include` folder. These do not need to be altered by the library user. The example components and helpers are located in the `components`, `helpers`, and `constants` folders. These are just for reference and should be replaced.

The foundational layer of drawing is handled in `draw_shapes.h`, where functions for directly drawing superellipses of different types. These should normaly not be used directly by the library user, but only by the rendereer (`renderer.h`) that draws the Element tree. The layout of the tree is calculated in `layout.h`.

## Prerequisites
SDL2, SDL2_ttf and SDL2_image libraries are required to compile and run this project.

### MacOS
Download sdl2, sdl2_ttf and sdl2_image from official releases and move them to `/Library/Frameworks`
https://stackoverflow.com/questions/60202947/vscode-intellisense-not-recognising-sdl-image-extension-library-for-sdl-framewor

## Compiling and running

### Clang
Compiling with SDL2, SDL2_ttf and SDL2_image:
`clang -std=c99 -Wall -Wextra  -F /Library/Frameworks -framework SDL2 -framework SDL2_ttf -framework SDL2_image main.c -o main`

`clang $(sdl2-config --cflags --libs) -lSDL2_ttf -lSDL2_image -o main main.c`

Running:
`./main`

## Todo
- Mac .app packaging
- To-Done example app
- Multiline text input
  - Input is one continous string
  - Reuse undo from single line
  - Linebreak function return array of strings
  - Find index from rendered position
  - Selection returns start row, start col, end row, end col