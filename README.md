# C9 gui

C9 gui is a performant and flexible GUI library built on SDL2. C9 gui primarily targets the [C9 language](https://github.com/1jss/C9-lang), which is a C99 subset.

## Features
- Flexible element tree structure (with dynamic loading of children)
- Padding, border and gutter around elements and between children
- Superellipse rounded corners (with anti-aliasing)
- Gradient backgrounds (smoothed with blue noise dithering) 
- Image backgrounds (png)
- Event handling (click, blur, key press)
- Layout engine (flex or scroll children in any direction)
- Lazy loading of elements (if outside of window bounds)
- Text rendering (single line with alignment or multiline)
- Easy table layout (automatic column sizing)
- Text input (single and multi line with selection and undo history)
- Event driven rendering (only rerender on user input)
- Partial rendering (only rerender elements that have changed)
- Buffered rendering (all elements are cached as textures)
- Element tags for easy element selection

## Screenshots
![image_1](/screenshots/Screenshot_240924_1.png?raw=true)
![image_1](/screenshots/Screenshot_240924_2.png?raw=true)
![image_1](/screenshots/Screenshot_240924_3.png?raw=true)
![image_1](/screenshots/Screenshot_240924_4.png?raw=true)

## Architecture

### Element tree
C9 gui uses a tree structure to store the retained "document model". Every node in the tree is an `element` that can have their own children. The list of children is a flexible C9 array, so the number of children (and thus the tree itself), can be changed at runtime.

### Element

An element is a struct that contains all the properties of a child. The structure is initalized with default values, so only the changed or used properties need to be set. In other words, all items are optional.

| Type          | Name                  | Comment                             |
|---------------|-----------------------|-------------------------------------|
| `u8`          | `element_tag`         | id or group id                      |
| `u16`         | `width`               | fixed width of the element          |
| `u16`         | `height`              | fixed height of the element         |
| `Padding`     | `padding`             | padding inside element (4 values)   |
| `Border`      | `border`              | border around element (4 values)    |
| `u8`          | `corner_radius`       | radius of superellipse corners      |
| `u8`          | `gutter`              | space between children              |
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

Example of creating a standalone card element with text:

```c
  Element *card_element = new_element(arena);
  *card_element = (Element){
    .background_type = background_type.color,
    .background.color = 0xFFFFFFFF,
    .border = (Border){1, 1, 1, 1},
    .border_color = 0xF2F3F4FF,
    .corner_radius = 15,
    .padding = (Padding){10, 10, 10, 10},
    .layout_direction = layout_direction.vertical,
  };

  Element *title_element = add_new_element(arena, card_element);
  *title_element = (Element){
    .text = to_s8("Card title"),
    .text_color = 0x555555FF,
    .font_variant = font_variant.large,
  };

  Element *text_element = add_new_element(arena, card_element);
  *text_element = (Element){
    .text = to_s8("Some text in the card"),
    .text_color = 0x555555FF,
  };
  ```

### Event handling
Events are handled in the main loop and if an element has an event handler function set for the event type (`on_click`, `on_blur`, `on_key_press`), it will be called.

If the event is a mouse down event, the element that is both under the pointer and has an on_click function will be set as active element and the on_click function will be called. The formerly active element will have its on_blur function called. If the element has a text input object, the text cursor will be set at the position of the mouse pointer.

If the event is a key press event, the active element will have its on_key_press function called, if it has one. On element with input objects the key press is automatically handled.

If the event is a scroll event, the entire tree will be searched for scrollable elements under the pointer and the scroll event will be applied to them, starting with the outermost element, so that children get scrolled before parents. The active element is not changed on scroll.

### Rendering
The interface is only rendered when the `rerender` flag of the element tree is set to true. The render function will then traverse the tree and redraw all child elements. All element are cached as textures, so only the elements that have new dimensions or are marked as changed will be redrawn from scratch. This means that scrolling and moving elements around is very efficient.

### Components
Components are reusable standalone elements that can dynamically be added and removed from the tree. They are implemented as global Element references (pointers) that get initalized on their first use. This way no more memory is used than needed and the already initalized component can be removed and readded to the tree without loosing its state and rendering cache.

## Project navigation
The project starts from `main.c` and all C9 includes are header only. The base library implementation is located in the `include` folder. These do not need to be altered by the library user. The example components and helpers are located in the `components`, `helpers`, and `constants` folders. These are just for reference and should be replaced.

The foundational layer of drawing is handled in `draw_shapes.h`, where functions for directly drawing superellipses of different types. These should normaly not be used directly by the library user, but only by the rendereer (`renderer.h`) that draws the Element tree. The layout of the tree is calculated in `layout.h`.

## Prerequisites
SDL2 is required to compile and run this project.

### MacOS
Download SDL2 from official releases and move it to `/Library/Frameworks`

## Compiling and running

### Clang
Compiling with SDL2:
`clang -std=c99 -Wall -Wextra -F /Library/Frameworks -framework SDL2 main.c -o main`

Running:
`./main`

## Todo
- Mac .app packaging
- To-Do example app
- Font variant in input?
- Input rerenders when added to layout
  - Only rerender lines that have changed
    - Compare newline list length with child element list length. If different, rerender.
    - Compare text length with child element text length. If different, rerender.
    - Compare text content with child element text content. If different, rerender.
- Fixed height multiline input scroll grows in layout
  - Might be fixed by adding input text as actual children, not as text in the same element.
- Dark theme example
- Better boolean values? boolean.TRUE, boolean.FALSE
  - https://github.com/leowhitehead/c-bool-value
- Make SFT_MeasureUTF8 return u8 indexes instead of utf8 characters

## Notes
- If element has background image, it has no border, other background or corner radius
- If element has input it has has no label (text)
- prop text_align only applies to label
- prop font_variant only applies to label
- text_color only applies to label and input