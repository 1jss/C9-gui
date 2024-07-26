#ifndef C9_EVENT

#include "element_tree.h" // ElementTree, Element

void click_handler(ElementTree *tree, void *data) {
  Element *element = tree->active_element;
  if (element != 0 && element->on_click != 0) {
    element->on_click(tree, data);
    element->render.changed = 1;
  }
}

void blur_handler(ElementTree *tree, void *data) {
  Element *element = tree->active_element;
  if (element != 0 && element->on_blur != 0) {
    element->on_blur(tree, data);
    element->render.changed = 1;
  }
}

void input_handler(ElementTree *tree, void *data) {
  Element *element = tree->active_element;
  if (element != 0 && element->on_key_press != 0) {
    element->on_key_press(tree, data);
    element->render.changed = 1;
  }
}

#define C9_EVENT
#endif