#ifndef C9_COLOR_THEME

#include "../include/color.c" // RGBA

// Colors
const RGBA white = 0xFFFFFFFF;
const RGBA white_2 = 0xF8F8F8FF;
const RGBA gray_1 = 0xF8F9FAFF;
const RGBA gray_2 = 0xF2F3F4FF;
const RGBA border_color = 0xDEE2E6FF;
const RGBA border_color_active = 0xADAFB2FF;
const RGBA text_color = 0x555555FF;
const RGBA text_color_active = 0x222222FF;
const RGBA text_color_muted = 0x9999AAFF;
const RGBA menu_active_color = 0xDEE2E6FF;
const RGBA text_cursor_color = 0x88BBF1FF;
const RGBA selection_color = 0xC7E1FCFF;
const RGBA scrollbar_color = 0xDEE2E6FF;

const C9_Gradient white_shade = {
  .start_color = white,
  .end_color = white_2,
  .start_at = 0.95,
  .end_at = 1
};

const C9_Gradient gray_1_shade = {
  .start_color = gray_1,
  .end_color = gray_2,
  .start_at = 0.95,
  .end_at = 1
};

const C9_Gradient button_gradient = {
  .start_color = 0x7281EDFF,
  .end_color = 0x8998EFFF,
};

#define C9_COLOR_THEME
#endif