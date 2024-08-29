#ifndef C9_FONT

#include <stdbool.h> // bool
#include "SDL_ttf.h" // TTF_Font, TTF_Init, TTF_OpenFont, TTF_CloseFont, TTF_GetError, TTF_Quit
#include "status.h" // status
#include "types.h" // i32, u8

bool font_initialized = false;
TTF_Font *inter_regular = 0;
TTF_Font *inter_bold = 0;
TTF_Font *inter_small = 0;
TTF_Font *inter_large = 0;

typedef struct {
  u8 regular;
  u8 bold;
  u8 small;
  u8 large;
} FontVariants;

FontVariants font_variant = {
  .regular = 0,
  .bold = 1,
  .small = 2,
  .large = 3,
};

i32 init_font(void) {
  if (TTF_Init()) {
    printf("TTF_Init: %s\n", TTF_GetError());
    return status.ERROR;
  }
  if (inter_regular == 0) {
    inter_regular = TTF_OpenFont("Inter-Regular.ttf", 14);
    if (inter_regular == 0) {
      printf("inter_regular failed: %s\n", TTF_GetError());
      return status.ERROR;
    }
    TTF_SetFontHinting(inter_regular, TTF_HINTING_LIGHT_SUBPIXEL);
  }
  if (inter_bold == 0) {
    inter_bold = TTF_OpenFont("Inter-Medium.ttf", 14);
    if (inter_bold == 0) {
      printf("inter_bold failed: %s\n", TTF_GetError());
      return status.ERROR;
    }
    TTF_SetFontHinting(inter_bold, TTF_HINTING_LIGHT_SUBPIXEL);
  }
  if (inter_small == 0) {
    inter_small = TTF_OpenFont("Inter-Medium.ttf", 12);
    if (inter_small == 0) {
      printf("inter_small failed: %s\n", TTF_GetError());
      return status.ERROR;
    }
    TTF_SetFontHinting(inter_small, TTF_HINTING_LIGHT_SUBPIXEL);
  }
  if (inter_large == 0) {
    inter_large = TTF_OpenFont("Inter-Regular.ttf", 18);
    if (inter_large == 0) {
      printf("inter_large failed: %s\n", TTF_GetError());
      return status.ERROR;
    }
    TTF_SetFontHinting(inter_large, TTF_HINTING_LIGHT_SUBPIXEL);
  }
  return status.OK;
}

TTF_Font *get_font(u8 variant) {
  if (font_initialized == false) {
    if (init_font() == status.OK) {
      font_initialized = true;
    } else {
      return 0; // null pointer
    }
  }
  if (variant == font_variant.regular) {
    return inter_regular;
  }
  if (variant == font_variant.bold) {
    return inter_bold;
  } else if (variant == font_variant.small) {
    return inter_small;
  } else if (variant == font_variant.large) {
    return inter_large;
  } else {
    return inter_regular;
  }
}

i32 get_font_height(u8 variant) {
  if (variant == font_variant.regular) {
    return 18;
  }
  if (variant == font_variant.bold) {
    return 18;
  } else if (variant == font_variant.small) {
    return 15;
  } else if (variant == font_variant.large) {
    return 23;
  } else {
    return 18;
  }
}
void close_font(void) {
  if (font_initialized == true) {
    TTF_CloseFont(inter_regular);
    TTF_CloseFont(inter_bold);
    TTF_CloseFont(inter_small);
    TTF_CloseFont(inter_large);
    TTF_Quit();
  }
}

#define C9_FONT
#endif