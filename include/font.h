#ifndef C9_FONT

#include <stdbool.h> // bool
#include "schrift.h" // SFT, sft_loadfile, sft_freefont
#include "status.h" // status
#include "types.h" // i32, u8

bool font_initialized = false;

SFT inter_regular = {
  .xScale = 15,
  .yScale = 15,
};

SFT inter_bold = {
  .xScale = 15,
  .yScale = 15,
};

SFT inter_small = {
  .xScale = 13,
  .yScale = 13,
};

SFT inter_large = {
  .xScale = 19,
  .yScale = 19,
};

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

i32 init_fonts(void) {
  if (inter_regular.font == 0) {
    inter_regular.font = sft_loadfile("InterDisplay-Regular-Tiny.ttf");
    if (inter_regular.font == NULL) {
      printf("inter_regular STF failed\n");
      return status.ERROR;
    }
  }
  if (inter_bold.font == 0) {
    inter_bold.font = sft_loadfile("InterDisplay-Medium-Tiny.ttf");
    if (inter_bold.font == NULL) {
      printf("inter_bold STF failed\n");
      return status.ERROR;
    }
  }
  if (inter_small.font == 0) {
    inter_small.font = sft_loadfile("InterDisplay-Medium-Tiny.ttf");
    if (inter_small.font == NULL) {
      printf("inter_small STF failed\n");
      return status.ERROR;
    }
  }
  if (inter_large.font == 0) {
    inter_large.font = sft_loadfile("InterDisplay-Regular-Tiny.ttf");
    if (inter_large.font == NULL) {
      printf("inter_large STF failed\n");
      return status.ERROR;
    }
  }
  return status.OK;
}

SFT *get_sft(u8 variant) {
  if (font_initialized == false) {
    if (init_fonts() == status.OK) {
      font_initialized = true;
    } else {
      return 0; // null pointer
    }
  }
  if (variant == font_variant.regular) {
    return &inter_regular;
  }
  if (variant == font_variant.bold) {
    return &inter_bold;
  } else if (variant == font_variant.small) {
    return &inter_small;
  } else if (variant == font_variant.large) {
    return &inter_large;
  } else {
    return &inter_regular;
  }
}

i32 get_font_height(u8 variant) {
  if (variant == font_variant.regular) {
    return 19;
  }
  if (variant == font_variant.bold) {
    return 19;
  } else if (variant == font_variant.small) {
    return 16;
  } else if (variant == font_variant.large) {
    return 23;
  } else {
    return 19;
  }
}

void close_fonts(void) {
  if (font_initialized == true) {
    sft_freefont(inter_regular.font);
    sft_freefont(inter_bold.font);
    sft_freefont(inter_small.font);
    sft_freefont(inter_large.font);
  }
}

#define C9_FONT
#endif