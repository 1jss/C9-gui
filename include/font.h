#ifndef C9_FONT

#include "SDL_ttf.h" // TTF_Font, TTF_Init, TTF_OpenFont, TTF_CloseFont, TTF_GetError, TTF_Quit
#include "types.h" // i32

TTF_Font *inter_font = 0;

i32 init_font(void) {
  if (inter_font == 0) {
    if (TTF_Init()) {
      printf("TTF_Init: %s\n", TTF_GetError());
      return -1;
    }
    inter_font = TTF_OpenFont("Inter-Regular.ttf", 16);
    if (inter_font == 0) {
      printf("Failed to load font: %s\n", TTF_GetError());
      return -1;
    }
    return 0;
  }
  return 1;
}

TTF_Font *get_font(void) {
  if (inter_font == 0) {
    init_font();
  }
  return inter_font;
}

void close_font(void) {
  if (inter_font != 0) {
    TTF_CloseFont(inter_font);
    TTF_Quit();
  }
}

#define C9_FONT
#endif