// This file is based on stb_image (C) 2017 Sean Barrett and contributors
// This file is not conformant to the C9 standard
#ifndef STBI_INCLUDE_STB_IMAGE

/*
DOCUMENTATION

Basic usage:
   int x; // width of image in pixels
   int y; // height of image in pixels
   int cif; // actual number of components per pixel in image
   int dc; // desired components per pixel in output data (0-4) where 0 is auto
   uint8_t *data = stbi_load(filename, &x, &y, &cif, d);
   // process result data
   stbi_image_free(data);

The return value from an image loader is an 'uint8_t *' which points to the pixel data, or NULL on an allocation failure or if the image is corrupt or invalid. The pixel data consists of *y scanlines of *x pixels, with each pixel consisting of N interleaved 8-bit components; the first pixel pointed to is top-left-most in the image. There is no padding between image scanlines or between pixels, regardless of format. The number of components N is 'desired_channels' if desired_channels is non-zero. If desired_channels is non-zero, *channels_in_file has the number of components that _would_ have been output otherwise. E.g. if you set desired_channels to 4, you will always get RGBA output, but you can check *channels_in_file to see if it's trivially opaque because e.g. there were only 3 channels in the source image.

An output image with N components has the following components interleaved in this order in each pixel:

    N=#comp     components
      1           grey
      2           grey, alpha
      3           red, green, blue
      4           red, green, blue, alpha

If image loading fails for any reason, the return value will be NULL, and *x, *y, *channels_in_file will be unchanged. The function stbi_failure_reason() can be queried for an extremely brief, end-user unfriendly explanation of why the load failed.

Paletted PNG images are automatically depalettized.

To query the width, height and component count of an image without having to decode the full file, you can use the stbi_info family of functions:

  int x,y,n,ok;
  ok = stbi_info(filename, &x, &y, &n);
  // returns ok=1 and sets x, y, n if image is a supported format,
  // 0 otherwise.

stb_image will reject image files that have any of their dimensions set to a larger value than the configurable STBI_MAX_DIMENSIONS, which defaults to 2**24 = 16777216 pixels. If you need to load an image with individual axis dimensions larger than this, and it still fits in the overall size limit, you can #define STBI_MAX_DIMENSIONS to be larger.
*/

#include <assert.h> // assert
#include <limits.h> // INT_MAX
#include <stdint.h> // uint8_t, uint16_t
#include <stdbool.h> // true, false
#include <stdio.h> // FILE, fopen, fclose, fseek, fgetc, feof, ferror
#include <stdlib.h> // malloc, free, realloc
#include <string.h> // memcpy

enum {
  STBI_default = 0, // only used for desired_channels
  STBI_grey = 1,
  STBI_grey_alpha = 2,
  STBI_rgb = 3,
  STBI_rgb_alpha = 4
};

// PRIMARY API

// load image by filename, open file, or memory buffer

typedef struct {
  int (*read)(void *user, char *data, int size); // fill 'data' with 'size' bytes.  return number of bytes actually read
  void (*skip)(void *user, int n); // skip the next 'n' bytes, or 'unget' the last -n bytes if negative
  int (*eof)(void *user); // returns nonzero if we are at end of file/data
} stbi_io_callbacks;

// 8-bits-per-channel interface

extern uint8_t *stbi_load_from_memory(uint8_t const *buffer, int len, int *x, int *y, int *channels_in_file, int desired_channels);

extern uint8_t *stbi_load(char const *filename, int *x, int *y, int *channels_in_file, int desired_channels);
extern uint8_t *stbi_load_from_file(FILE *f, int *x, int *y, int *channels_in_file, int desired_channels);
// for stbi_load_from_file, file pointer is left pointing immediately after image

// 16-bits-per-channel interface

extern uint16_t *stbi_load_16_from_memory(uint8_t const *buffer, int len, int *x, int *y, int *channels_in_file, int desired_channels);

extern uint16_t *stbi_load_16(char const *filename, int *x, int *y, int *channels_in_file, int desired_channels);
extern uint16_t *stbi_load_from_file_16(FILE *f, int *x, int *y, int *channels_in_file, int desired_channels);

// get image dimensions & components without fully decoding
extern int stbi_info_from_memory(uint8_t const *buffer, int len, int *x, int *y, int *comp);
extern int stbi_is_16_bit_from_memory(uint8_t const *buffer, int len);
extern int stbi_info(char const *filename, int *x, int *y, int *comp);
extern int stbi_info_from_file(FILE *f, int *x, int *y, int *comp);
extern int stbi_is_16_bit(char const *filename);
extern int stbi_is_16_bit_from_file(FILE *f);

// ZLIB client - used by PNG, available for other purposes
extern char *stbi_zlib_decode_malloc_guesssize(const char *buffer, int len, int initial_size, int *outlen);
extern char *stbi_zlib_decode_malloc_guesssize_headerflag(const char *buffer, int len, int initial_size, int *outlen, int parse_header);
extern char *stbi_zlib_decode_malloc(const char *buffer, int len, int *outlen);
extern int stbi_zlib_decode_buffer(char *obuffer, int olen, const char *ibuffer, int ilen);
extern char *stbi_zlib_decode_noheader_malloc(const char *buffer, int len, int *outlen);
extern int stbi_zlib_decode_noheader_buffer(char *obuffer, int olen, const char *ibuffer, int ilen);

#define STBI_NOTUSED(v) (void)sizeof(v)

#ifndef STBI_REALLOC_SIZED
#define STBI_REALLOC_SIZED(p, oldsz, newsz) realloc(p, newsz)
#endif

#ifndef STBI_MAX_DIMENSIONS
#define STBI_MAX_DIMENSIONS (1 << 24)
#endif

//  stbi__context struct and start_xxx functions

// stbi__context structure is our basic context used by all images, so it contains all the IO context, plus some basic image information
typedef struct {
  uint32_t img_x;
  uint32_t img_y;
  int img_n;
  int img_out_n;

  stbi_io_callbacks io;
  void *io_user_data;

  int read_from_callbacks;
  int buflen;
  uint8_t buffer_start[128];
  int callback_already_read;

  uint8_t *img_buffer;
  uint8_t *img_buffer_end;
  uint8_t *img_buffer_original;
  uint8_t *img_buffer_original_end;
} stbi__context;

static void stbi__refill_buffer(stbi__context *s);

// initialize a memory-decode context
static void stbi__start_mem(stbi__context *s, uint8_t const *buffer, int len) {
  s->io.read = NULL;
  s->read_from_callbacks = 0;
  s->callback_already_read = 0;
  s->img_buffer = s->img_buffer_original = (uint8_t *)buffer;
  s->img_buffer_end = s->img_buffer_original_end = (uint8_t *)buffer + len;
}

// initialize a callback-based context
static void stbi__start_callbacks(stbi__context *s, stbi_io_callbacks *c, void *user) {
  s->io = *c;
  s->io_user_data = user;
  s->buflen = sizeof(s->buffer_start);
  s->read_from_callbacks = 1;
  s->callback_already_read = 0;
  s->img_buffer = s->img_buffer_original = s->buffer_start;
  stbi__refill_buffer(s);
  s->img_buffer_original_end = s->img_buffer_end;
}

static int stbi__stdio_read(void *user, char *data, int size) {
  return (int)fread(data, 1, size, (FILE *)user);
}

static void stbi__stdio_skip(void *user, int n) {
  int ch;
  fseek((FILE *)user, n, SEEK_CUR);
  ch = fgetc((FILE *)user); // have to read a byte to reset feof()'s flag
  if (ch != EOF) {
    ungetc(ch, (FILE *)user); // push byte back onto stream if valid.
  }
}

static int stbi__stdio_eof(void *user) {
  return feof((FILE *)user) || ferror((FILE *)user);
}

static stbi_io_callbacks stbi__stdio_callbacks = {
  stbi__stdio_read,
  stbi__stdio_skip,
  stbi__stdio_eof,
};

static void stbi__start_file(stbi__context *s, FILE *f) {
  stbi__start_callbacks(s, &stbi__stdio_callbacks, (void *)f);
}

static void stbi__rewind(stbi__context *s) {
  // conceptually rewind SHOULD rewind to the beginning of the stream, but we just rewind to the beginning of the initial buffer, because we only use it after doing 'test', which only ever looks at at most 92 bytes
  s->img_buffer = s->img_buffer_original;
  s->img_buffer_end = s->img_buffer_original_end;
}

typedef struct {
  int bits_per_channel;
  int num_channels;
} stbi__result_info;

static int stbi__png_test(stbi__context *s);
static void *stbi__png_load(stbi__context *s, int *x, int *y, int *comp, int req_comp, stbi__result_info *ri);
static int stbi__png_info(stbi__context *s, int *x, int *y, int *comp);
static int stbi__png_is16(stbi__context *s);

static const char *stbi__g_failure_reason;

// get a brief reason for failure
extern const char *stbi_failure_reason(void) {
  return stbi__g_failure_reason;
}

static int stbi__err(const char *str) {
  stbi__g_failure_reason = str;
  return 0;
}

// stbi__errpuc - error returning pointer to unsigned char
static unsigned char *stbi__errpuc(const char *str) {
  stbi__g_failure_reason = str;
  return NULL;
}

// stb_image uses ints pervasively, including for offset calculations. therefore the largest decoded image size we can support with the current code, even on 64-bit targets, is INT_MAX. this is not a significant limitation for the intended use case.
// we do, however, need to make sure our size calculations don't overflow. hence a few helper functions for size calculations that multiply integers together, making sure that they're non-negative and no overflow occurs.

// return 1 if the sum is valid, 0 on overflow.
// negative terms are considered invalid.
static int stbi__addsizes_valid(int a, int b) {
  if (b < 0) return 0;
  // now 0 <= b <= INT_MAX, hence also
  // 0 <= INT_MAX - b <= INTMAX.
  // And "a + b <= INT_MAX" (which might overflow) is the
  // same as a <= INT_MAX - b (no overflow)
  return a <= INT_MAX - b;
}

// returns 1 if the product is valid, 0 on overflow.
// negative factors are considered invalid.
static int stbi__mul2sizes_valid(int a, int b) {
  if (a < 0 || b < 0) return 0;
  if (b == 0) return 1; // mul-by-0 is always safe
  // portable way to check for no overflows in a*b
  return a <= INT_MAX / b;
}

// returns 1 if "a*b + add" has no negative terms/factors and doesn't overflow
static int stbi__mad2sizes_valid(int a, int b, int add) {
  return stbi__mul2sizes_valid(a, b) && stbi__addsizes_valid(a * b, add);
}

// returns 1 if "a*b*c + add" has no negative terms/factors and doesn't overflow
static int stbi__mad3sizes_valid(int a, int b, int c, int add) {
  return stbi__mul2sizes_valid(a, b) && stbi__mul2sizes_valid(a * b, c) &&
         stbi__addsizes_valid(a * b * c, add);
}

// mallocs with size overflow checking
static void *stbi__malloc_mad2(int a, int b, int add) {
  if (!stbi__mad2sizes_valid(a, b, add)) return NULL;
  return malloc(a * b + add);
}

static void *stbi__malloc_mad3(int a, int b, int c, int add) {
  if (!stbi__mad3sizes_valid(a, b, c, add)) return NULL;
  return malloc(a * b * c + add);
}

extern void stbi_image_free(void *retval_from_stbi_load) {
  free(retval_from_stbi_load);
}

static void *stbi__load_main(stbi__context *s, int *x, int *y, int *comp, int req_comp, stbi__result_info *ri, int bpc) {
  memset(ri, 0, sizeof(*ri)); // make sure it's initialized if we add new fields
  ri->bits_per_channel = bpc; // default is 8 so most paths don't have to be changed
  ri->num_channels = 0;

  // test the formats with a very explicit header first (at least a FOURCC or distinctive magic number first)
  if (stbi__png_test(s)) return stbi__png_load(s, x, y, comp, req_comp, ri);

  return stbi__errpuc("unknown image type");
}

static uint8_t *stbi__convert_16_to_8(uint16_t *orig, int w, int h, int channels) {
  int i;
  int img_len = w * h * channels;
  uint8_t *reduced;

  reduced = (uint8_t *)malloc(img_len);
  if (reduced == NULL) return stbi__errpuc("outofmem");

  for (i = 0; i < img_len; ++i)
    reduced[i] = (uint8_t)((orig[i] >> 8) & 0xFF); // top half of each byte is sufficient approx of 16->8 bit scaling

  free(orig);
  return reduced;
}

static uint16_t *stbi__convert_8_to_16(uint8_t *orig, int w, int h, int channels) {
  int i;
  int img_len = w * h * channels;
  uint16_t *enlarged;

  enlarged = (uint16_t *)malloc(img_len * 2);
  if (enlarged == NULL) return (uint16_t *)stbi__errpuc("outofmem");

  for (i = 0; i < img_len; ++i)
    enlarged[i] = (uint16_t)((orig[i] << 8) + orig[i]); // replicate to high and low byte, maps 0->0, 255->0xffff

  free(orig);
  return enlarged;
}

static unsigned char *stbi__load_and_postprocess_8bit(stbi__context *s, int *x, int *y, int *comp, int req_comp) {
  stbi__result_info ri;
  void *result = stbi__load_main(s, x, y, comp, req_comp, &ri, 8);

  if (result == NULL)
    return NULL;

  // it is the responsibility of the loaders to make sure we get either 8 or 16 bit.
  assert(ri.bits_per_channel == 8 || ri.bits_per_channel == 16);

  if (ri.bits_per_channel != 8) {
    result = stbi__convert_16_to_8((uint16_t *)result, *x, *y, req_comp == 0 ? *comp : req_comp);
    ri.bits_per_channel = 8;
  }

  return (unsigned char *)result;
}

static uint16_t *stbi__load_and_postprocess_16bit(stbi__context *s, int *x, int *y, int *comp, int req_comp) {
  stbi__result_info ri;
  void *result = stbi__load_main(s, x, y, comp, req_comp, &ri, 16);

  if (result == NULL)
    return NULL;

  // it is the responsibility of the loaders to make sure we get either 8 or 16 bit.
  assert(ri.bits_per_channel == 8 || ri.bits_per_channel == 16);

  if (ri.bits_per_channel != 16) {
    result = stbi__convert_8_to_16((uint8_t *)result, *x, *y, req_comp == 0 ? *comp : req_comp);
    ri.bits_per_channel = 16;
  }

  return (uint16_t *)result;
}

static FILE *stbi__fopen(char const *filename, char const *mode) {
  FILE *f;
  f = fopen(filename, mode);
  return f;
}

extern uint8_t *stbi_load(char const *filename, int *x, int *y, int *comp, int req_comp) {
  FILE *f = stbi__fopen(filename, "rb");
  unsigned char *result;
  if (!f) return stbi__errpuc("can't fopen");
  result = stbi_load_from_file(f, x, y, comp, req_comp);
  fclose(f);
  return result;
}

extern uint8_t *stbi_load_from_file(FILE *f, int *x, int *y, int *comp, int req_comp) {
  unsigned char *result;
  stbi__context s;
  stbi__start_file(&s, f);
  result = stbi__load_and_postprocess_8bit(&s, x, y, comp, req_comp);
  if (result) {
    // need to 'unget' all the characters in the IO buffer
    fseek(f, -(int)(s.img_buffer_end - s.img_buffer), SEEK_CUR);
  }
  return result;
}

extern uint16_t *stbi_load_from_file_16(FILE *f, int *x, int *y, int *comp, int req_comp) {
  uint16_t *result;
  stbi__context s;
  stbi__start_file(&s, f);
  result = stbi__load_and_postprocess_16bit(&s, x, y, comp, req_comp);
  if (result) {
    // need to 'unget' all the characters in the IO buffer
    fseek(f, -(int)(s.img_buffer_end - s.img_buffer), SEEK_CUR);
  }
  return result;
}

extern uint16_t *stbi_load_16(char const *filename, int *x, int *y, int *comp, int req_comp) {
  FILE *f = stbi__fopen(filename, "rb");
  uint16_t *result;
  if (!f) return (uint16_t *)stbi__errpuc("can't fopen");
  result = stbi_load_from_file_16(f, x, y, comp, req_comp);
  fclose(f);
  return result;
}

extern uint16_t *stbi_load_16_from_memory(uint8_t const *buffer, int len, int *x, int *y, int *channels_in_file, int desired_channels) {
  stbi__context s;
  stbi__start_mem(&s, buffer, len);
  return stbi__load_and_postprocess_16bit(&s, x, y, channels_in_file, desired_channels);
}

extern uint8_t *stbi_load_from_memory(uint8_t const *buffer, int len, int *x, int *y, int *comp, int req_comp) {
  stbi__context s;
  stbi__start_mem(&s, buffer, len);
  return stbi__load_and_postprocess_8bit(&s, x, y, comp, req_comp);
}

// Common code used by all image loaders

enum {
  STBI__SCAN_load = 0,
  STBI__SCAN_type,
  STBI__SCAN_header
};

static void stbi__refill_buffer(stbi__context *s) {
  int n = (s->io.read)(s->io_user_data, (char *)s->buffer_start, s->buflen);
  s->callback_already_read += (int)(s->img_buffer - s->img_buffer_original);
  if (n == 0) {
    // at end of file, treat same as if from memory, but need to handle case where s->img_buffer isn't pointing to safe memory, e.g. 0-byte file
    s->read_from_callbacks = 0;
    s->img_buffer = s->buffer_start;
    s->img_buffer_end = s->buffer_start + 1;
    *s->img_buffer = 0;
  } else {
    s->img_buffer = s->buffer_start;
    s->img_buffer_end = s->buffer_start + n;
  }
}

static uint8_t stbi__get8(stbi__context *s) {
  if (s->img_buffer < s->img_buffer_end)
    return *s->img_buffer++;
  if (s->read_from_callbacks) {
    stbi__refill_buffer(s);
    return *s->img_buffer++;
  }
  return 0;
}

static void stbi__skip(stbi__context *s, int n) {
  if (n == 0) return; // already there!
  if (n < 0) {
    s->img_buffer = s->img_buffer_end;
    return;
  }
  if (s->io.read) {
    int blen = (int)(s->img_buffer_end - s->img_buffer);
    if (blen < n) {
      s->img_buffer = s->img_buffer_end;
      (s->io.skip)(s->io_user_data, n - blen);
      return;
    }
  }
  s->img_buffer += n;
}

static int stbi__getn(stbi__context *s, uint8_t *buffer, int n) {
  if (s->io.read) {
    int blen = (int)(s->img_buffer_end - s->img_buffer);
    if (blen < n) {
      int res, count;

      memcpy(buffer, s->img_buffer, blen);

      count = (s->io.read)(s->io_user_data, (char *)buffer + blen, n - blen);
      res = (count == (n - blen));
      s->img_buffer = s->img_buffer_end;
      return res;
    }
  }

  if (s->img_buffer + n <= s->img_buffer_end) {
    memcpy(buffer, s->img_buffer, n);
    s->img_buffer += n;
    return 1;
  } else
    return 0;
}

static int stbi__get16be(stbi__context *s) {
  int z = stbi__get8(s);
  return (z << 8) + stbi__get8(s);
}

static uint32_t stbi__get32be(stbi__context *s) {
  uint32_t z = stbi__get16be(s);
  return (z << 16) + stbi__get16be(s);
}

#define STBI__BYTECAST(x) ((uint8_t)((x) & 255)) // truncate int to byte without warnings

//  generic converter from built-in img_n to req_comp
//  assume data buffer is malloced, so malloc a new one and free that one
//  only failure mode is malloc failing

static uint8_t stbi__compute_y(int r, int g, int b) {
  return (uint8_t)(((r * 77) + (g * 150) + (29 * b)) >> 8);
}

static unsigned char *stbi__convert_format(unsigned char *data, int img_n, int req_comp, unsigned int x, unsigned int y) {
  int i, j;
  unsigned char *good;

  if (req_comp == img_n) return data;
  assert(req_comp >= 1 && req_comp <= 4);

  good = (unsigned char *)stbi__malloc_mad3(req_comp, x, y, 0);
  if (good == NULL) {
    free(data);
    return stbi__errpuc("outofmem");
  }

  for (j = 0; j < (int)y; ++j) {
    unsigned char *src = data + j * x * img_n;
    unsigned char *dest = good + j * x * req_comp;

#define STBI__COMBO(a, b) ((a) * 8 + (b))
#define STBI__CASE(a, b) \
  case STBI__COMBO(a, b): \
    for (i = x - 1; i >= 0; --i, src += a, dest += b)
    // convert source image with img_n components to one with req_comp components; avoid switch per pixel, so use switch per scanline and massive macros
    switch (STBI__COMBO(img_n, req_comp)) {
      STBI__CASE(1, 2) {
        dest[0] = src[0];
        dest[1] = 255;
      }
      break;
      STBI__CASE(1, 3) { dest[0] = dest[1] = dest[2] = src[0]; }
      break;
      STBI__CASE(1, 4) {
        dest[0] = dest[1] = dest[2] = src[0];
        dest[3] = 255;
      }
      break;
      STBI__CASE(2, 1) { dest[0] = src[0]; }
      break;
      STBI__CASE(2, 3) { dest[0] = dest[1] = dest[2] = src[0]; }
      break;
      STBI__CASE(2, 4) {
        dest[0] = dest[1] = dest[2] = src[0];
        dest[3] = src[1];
      }
      break;
      STBI__CASE(3, 4) {
        dest[0] = src[0];
        dest[1] = src[1];
        dest[2] = src[2];
        dest[3] = 255;
      }
      break;
      STBI__CASE(3, 1) { dest[0] = stbi__compute_y(src[0], src[1], src[2]); }
      break;
      STBI__CASE(3, 2) {
        dest[0] = stbi__compute_y(src[0], src[1], src[2]);
        dest[1] = 255;
      }
      break;
      STBI__CASE(4, 1) { dest[0] = stbi__compute_y(src[0], src[1], src[2]); }
      break;
      STBI__CASE(4, 2) {
        dest[0] = stbi__compute_y(src[0], src[1], src[2]);
        dest[1] = src[3];
      }
      break;
      STBI__CASE(4, 3) {
        dest[0] = src[0];
        dest[1] = src[1];
        dest[2] = src[2];
      }
      break;
      default:
        assert(0);
        free(data);
        free(good);
        return stbi__errpuc("unsupported");
    }
#undef STBI__CASE
  }

  free(data);
  return good;
}

static uint16_t stbi__compute_y_16(int r, int g, int b) {
  return (uint16_t)(((r * 77) + (g * 150) + (29 * b)) >> 8);
}

static uint16_t *stbi__convert_format16(uint16_t *data, int img_n, int req_comp, unsigned int x, unsigned int y) {
  int i, j;
  uint16_t *good;

  if (req_comp == img_n) return data;
  assert(req_comp >= 1 && req_comp <= 4);

  good = (uint16_t *)malloc(req_comp * x * y * 2);
  if (good == NULL) {
    free(data);
    return (uint16_t *)stbi__errpuc("outofmem");
  }

  for (j = 0; j < (int)y; ++j) {
    uint16_t *src = data + j * x * img_n;
    uint16_t *dest = good + j * x * req_comp;

#define STBI__COMBO(a, b) ((a) * 8 + (b))
#define STBI__CASE(a, b) \
  case STBI__COMBO(a, b): \
    for (i = x - 1; i >= 0; --i, src += a, dest += b)
    // convert source image with img_n components to one with req_comp components; avoid switch per pixel, so use switch per scanline and massive macros
    switch (STBI__COMBO(img_n, req_comp)) {
      STBI__CASE(1, 2) {
        dest[0] = src[0];
        dest[1] = 0xffff;
      }
      break;
      STBI__CASE(1, 3) { dest[0] = dest[1] = dest[2] = src[0]; }
      break;
      STBI__CASE(1, 4) {
        dest[0] = dest[1] = dest[2] = src[0];
        dest[3] = 0xffff;
      }
      break;
      STBI__CASE(2, 1) { dest[0] = src[0]; }
      break;
      STBI__CASE(2, 3) { dest[0] = dest[1] = dest[2] = src[0]; }
      break;
      STBI__CASE(2, 4) {
        dest[0] = dest[1] = dest[2] = src[0];
        dest[3] = src[1];
      }
      break;
      STBI__CASE(3, 4) {
        dest[0] = src[0];
        dest[1] = src[1];
        dest[2] = src[2];
        dest[3] = 0xffff;
      }
      break;
      STBI__CASE(3, 1) { dest[0] = stbi__compute_y_16(src[0], src[1], src[2]); }
      break;
      STBI__CASE(3, 2) {
        dest[0] = stbi__compute_y_16(src[0], src[1], src[2]);
        dest[1] = 0xffff;
      }
      break;
      STBI__CASE(4, 1) { dest[0] = stbi__compute_y_16(src[0], src[1], src[2]); }
      break;
      STBI__CASE(4, 2) {
        dest[0] = stbi__compute_y_16(src[0], src[1], src[2]);
        dest[1] = src[3];
      }
      break;
      STBI__CASE(4, 3) {
        dest[0] = src[0];
        dest[1] = src[1];
        dest[2] = src[2];
      }
      break;
      default:
        assert(0);
        free(data);
        free(good);
        return (uint16_t *)stbi__errpuc("unsupported");
    }
#undef STBI__CASE
  }

  free(data);
  return good;
}

// public domain zlib decode    v0.2  Sean Barrett 2006-11-18
//    simple implementation
//      - all input must be provided in an upfront buffer
//      - all output is written to a single output buffer (can malloc/realloc)
//    performance
//      - fast huffman

// fast-way is faster to check than jpeg huffman, but slow way is slower
#define STBI__ZFAST_BITS 9 // accelerate all cases in default tables
#define STBI__ZFAST_MASK ((1 << STBI__ZFAST_BITS) - 1)
#define STBI__ZNSYMS 288 // number of symbols in literal/length alphabet

// zlib-style huffman encoding
typedef struct {
  uint16_t fast[1 << STBI__ZFAST_BITS];
  uint16_t firstcode[16];
  int maxcode[17];
  uint16_t firstsymbol[16];
  uint8_t size[STBI__ZNSYMS];
  uint16_t value[STBI__ZNSYMS];
} stbi__zhuffman;

static int stbi__bitreverse16(int n) {
  n = ((n & 0xAAAA) >> 1) | ((n & 0x5555) << 1);
  n = ((n & 0xCCCC) >> 2) | ((n & 0x3333) << 2);
  n = ((n & 0xF0F0) >> 4) | ((n & 0x0F0F) << 4);
  n = ((n & 0xFF00) >> 8) | ((n & 0x00FF) << 8);
  return n;
}

static int stbi__bit_reverse(int v, int bits) {
  assert(bits <= 16);
  // to bit reverse n bits, reverse 16 and shift
  // e.g. 11 bits, bit reverse and shift away 5
  return stbi__bitreverse16(v) >> (16 - bits);
}

static int stbi__zbuild_huffman(stbi__zhuffman *z, const uint8_t *sizelist, int num) {
  int i, k = 0;
  int code, next_code[16], sizes[17];

  // DEFLATE spec for generating codes
  memset(sizes, 0, sizeof(sizes));
  memset(z->fast, 0, sizeof(z->fast));
  for (i = 0; i < num; ++i)
    ++sizes[sizelist[i]];
  sizes[0] = 0;
  for (i = 1; i < 16; ++i)
    if (sizes[i] > (1 << i))
      return stbi__err("bad sizes");
  code = 0;
  for (i = 1; i < 16; ++i) {
    next_code[i] = code;
    z->firstcode[i] = (uint16_t)code;
    z->firstsymbol[i] = (uint16_t)k;
    code = (code + sizes[i]);
    if (sizes[i])
      if (code - 1 >= (1 << i)) return stbi__err("bad codelengths");
    z->maxcode[i] = code << (16 - i); // preshift for inner loop
    code <<= 1;
    k += sizes[i];
  }
  z->maxcode[16] = 0x10000; // sentinel
  for (i = 0; i < num; ++i) {
    int s = sizelist[i];
    if (s) {
      int c = next_code[s] - z->firstcode[s] + z->firstsymbol[s];
      uint16_t fastv = (uint16_t)((s << 9) | i);
      z->size[c] = (uint8_t)s;
      z->value[c] = (uint16_t)i;
      if (s <= STBI__ZFAST_BITS) {
        int j = stbi__bit_reverse(next_code[s], s);
        while (j < (1 << STBI__ZFAST_BITS)) {
          z->fast[j] = fastv;
          j += (1 << s);
        }
      }
      ++next_code[s];
    }
  }
  return 1;
}

// zlib-from-memory implementation for PNG reading
// because PNG allows splitting the zlib stream arbitrarily, and it's annoying structurally to have PNG call ZLIB call PNG, we require PNG read all the IDATs and combine them into a single memory buffer

typedef struct {
  uint8_t *zbuffer, *zbuffer_end;
  int num_bits;
  int hit_zeof_once;
  uint32_t code_buffer;

  char *zout;
  char *zout_start;
  char *zout_end;
  int z_expandable;

  stbi__zhuffman z_length, z_distance;
} stbi__zbuf;

static int stbi__zeof(stbi__zbuf *z) {
  return (z->zbuffer >= z->zbuffer_end);
}

static uint8_t stbi__zget8(stbi__zbuf *z) {
  return stbi__zeof(z) ? 0 : *z->zbuffer++;
}

static void stbi__fill_bits(stbi__zbuf *z) {
  do {
    if (z->code_buffer >= (1U << z->num_bits)) {
      z->zbuffer = z->zbuffer_end; // treat this as EOF so we fail
      return;
    }
    z->code_buffer |= (unsigned int)stbi__zget8(z) << z->num_bits;
    z->num_bits += 8;
  } while (z->num_bits <= 24);
}

static unsigned int stbi__zreceive(stbi__zbuf *z, int n) {
  unsigned int k;
  if (z->num_bits < n) stbi__fill_bits(z);
  k = z->code_buffer & ((1 << n) - 1);
  z->code_buffer >>= n;
  z->num_bits -= n;
  return k;
}

static int stbi__zhuffman_decode_slowpath(stbi__zbuf *a, stbi__zhuffman *z) {
  int b, s, k;
  // not resolved by fast table, so compute it the slow way
  // use jpeg approach, which requires MSbits at top
  k = stbi__bit_reverse(a->code_buffer, 16);
  for (s = STBI__ZFAST_BITS + 1;; ++s)
    if (k < z->maxcode[s])
      break;
  if (s >= 16) return -1; // invalid code!
  // code size is s, so:
  b = (k >> (16 - s)) - z->firstcode[s] + z->firstsymbol[s];
  if (b >= STBI__ZNSYMS) return -1; // some data was corrupt somewhere!
  if (z->size[b] != s) return -1; // was originally an assert, but report failure instead.
  a->code_buffer >>= s;
  a->num_bits -= s;
  return z->value[b];
}

static int stbi__zhuffman_decode(stbi__zbuf *a, stbi__zhuffman *z) {
  int b, s;
  if (a->num_bits < 16) {
    if (stbi__zeof(a)) {
      if (!a->hit_zeof_once) {
        // This is the first time we hit eof, insert 16 extra padding bits to allow us to keep going; if we actually consume any of them though, that is invalid data. This is caught later.
        a->hit_zeof_once = 1;
        a->num_bits += 16; // add 16 implicit zero bits
      } else {
        // We already inserted our extra 16 padding bits and are again out, this stream is actually prematurely terminated.
        return -1;
      }
    } else {
      stbi__fill_bits(a);
    }
  }
  b = z->fast[a->code_buffer & STBI__ZFAST_MASK];
  if (b) {
    s = b >> 9;
    a->code_buffer >>= s;
    a->num_bits -= s;
    return b & 511;
  }
  return stbi__zhuffman_decode_slowpath(a, z);
}

static int stbi__zexpand(stbi__zbuf *z, char *zout, int n) // need to make room for n bytes
{
  char *q;
  unsigned int cur, limit, old_limit;
  z->zout = zout;
  if (!z->z_expandable) return stbi__err("output buffer limit");
  cur = (unsigned int)(z->zout - z->zout_start);
  limit = old_limit = (unsigned)(z->zout_end - z->zout_start);
  if (UINT_MAX - cur < (unsigned)n) return stbi__err("outofmem");
  while (cur + n > limit) {
    if (limit > UINT_MAX / 2) return stbi__err("outofmem");
    limit *= 2;
  }
  q = (char *)STBI_REALLOC_SIZED(z->zout_start, old_limit, limit);
  STBI_NOTUSED(old_limit);
  if (q == NULL) return stbi__err("outofmem");
  z->zout_start = q;
  z->zout = q + cur;
  z->zout_end = q + limit;
  return 1;
}

static const int stbi__zlength_base[31] = {
  3, 4, 5, 6, 7, 8, 9, 10, 11, 13,
  15, 17, 19, 23, 27, 31, 35, 43, 51, 59,
  67, 83, 99, 115, 131, 163, 195, 227, 258, 0, 0
};

static const int stbi__zlength_extra[31] = {0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 0, 0, 0};

static const int stbi__zdist_base[32] = {1, 2, 3, 4, 5, 7, 9, 13, 17, 25, 33, 49, 65, 97, 129, 193, 257, 385, 513, 769, 1025, 1537, 2049, 3073, 4097, 6145, 8193, 12289, 16385, 24577, 0, 0};

static const int stbi__zdist_extra[32] = {0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12, 12, 13, 13};

static int stbi__parse_huffman_block(stbi__zbuf *a) {
  char *zout = a->zout;
  while (true) {
    int z = stbi__zhuffman_decode(a, &a->z_length);
    if (z < 256) {
      if (z < 0) return stbi__err("bad huffman code"); // error in huffman codes
      if (zout >= a->zout_end) {
        if (!stbi__zexpand(a, zout, 1)) return 0;
        zout = a->zout;
      }
      *zout++ = (char)z;
    } else {
      uint8_t *p;
      int len, dist;
      if (z == 256) {
        a->zout = zout;
        if (a->hit_zeof_once && a->num_bits < 16) {
          // The first time we hit zeof, we inserted 16 extra zero bits into our bit buffer so the decoder can just do its speculative decoding. But if we actually consumed any of those bits (which is the case when num_bits < 16), the stream actually read past the end so it is malformed.
          return stbi__err("unexpected end");
        }
        return 1;
      }
      if (z >= 286) return stbi__err("bad huffman code"); // per DEFLATE, length codes 286 and 287 must not appear in compressed data
      z -= 257;
      len = stbi__zlength_base[z];
      if (stbi__zlength_extra[z]) len += stbi__zreceive(a, stbi__zlength_extra[z]);
      z = stbi__zhuffman_decode(a, &a->z_distance);
      if (z < 0 || z >= 30) return stbi__err("bad huffman code"); // per DEFLATE, distance codes 30 and 31 must not appear in compressed data
      dist = stbi__zdist_base[z];
      if (stbi__zdist_extra[z]) dist += stbi__zreceive(a, stbi__zdist_extra[z]);
      if (zout - a->zout_start < dist) return stbi__err("bad dist");
      if (len > a->zout_end - zout) {
        if (!stbi__zexpand(a, zout, len)) return 0;
        zout = a->zout;
      }
      p = (uint8_t *)(zout - dist);
      if (dist == 1) { // run of one byte; common in images.
        uint8_t v = *p;
        if (len) {
          do
            *zout++ = v;
          while (--len);
        }
      } else {
        if (len) {
          do
            *zout++ = *p++;
          while (--len);
        }
      }
    }
  }
}

static int stbi__compute_huffman_codes(stbi__zbuf *a) {
  static const uint8_t length_dezigzag[19] = {16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};
  stbi__zhuffman z_codelength;
  uint8_t lencodes[286 + 32 + 137]; //padding for maximum single op
  uint8_t codelength_sizes[19];
  int i, n;

  int hlit = stbi__zreceive(a, 5) + 257;
  int hdist = stbi__zreceive(a, 5) + 1;
  int hclen = stbi__zreceive(a, 4) + 4;
  int ntot = hlit + hdist;

  memset(codelength_sizes, 0, sizeof(codelength_sizes));
  for (i = 0; i < hclen; ++i) {
    int s = stbi__zreceive(a, 3);
    codelength_sizes[length_dezigzag[i]] = (uint8_t)s;
  }
  if (!stbi__zbuild_huffman(&z_codelength, codelength_sizes, 19)) return 0;

  n = 0;
  while (n < ntot) {
    int c = stbi__zhuffman_decode(a, &z_codelength);
    if (c < 0 || c >= 19) return stbi__err("bad codelengths");
    if (c < 16)
      lencodes[n++] = (uint8_t)c;
    else {
      uint8_t fill = 0;
      if (c == 16) {
        c = stbi__zreceive(a, 2) + 3;
        if (n == 0) return stbi__err("bad codelengths");
        fill = lencodes[n - 1];
      } else if (c == 17) {
        c = stbi__zreceive(a, 3) + 3;
      } else if (c == 18) {
        c = stbi__zreceive(a, 7) + 11;
      } else {
        return stbi__err("bad codelengths");
      }
      if (ntot - n < c) return stbi__err("bad codelengths");
      memset(lencodes + n, fill, c);
      n += c;
    }
  }
  if (n != ntot) return stbi__err("bad codelengths");
  if (!stbi__zbuild_huffman(&a->z_length, lencodes, hlit)) return 0;
  if (!stbi__zbuild_huffman(&a->z_distance, lencodes + hlit, hdist)) return 0;
  return 1;
}

static int stbi__parse_uncompressed_block(stbi__zbuf *a) {
  uint8_t header[4];
  int len, nlen, k;
  if (a->num_bits & 7)
    stbi__zreceive(a, a->num_bits & 7); // discard
  // drain the bit-packed data into header
  k = 0;
  while (a->num_bits > 0) {
    header[k++] = (uint8_t)(a->code_buffer & 255); // suppress MSVC run-time check
    a->code_buffer >>= 8;
    a->num_bits -= 8;
  }
  if (a->num_bits < 0) return stbi__err("zlib corrupt");
  // now fill header the normal way
  while (k < 4)
    header[k++] = stbi__zget8(a);
  len = header[1] * 256 + header[0];
  nlen = header[3] * 256 + header[2];
  if (nlen != (len ^ 0xffff)) return stbi__err("zlib corrupt");
  if (a->zbuffer + len > a->zbuffer_end) return stbi__err("read past buffer");
  if (a->zout + len > a->zout_end)
    if (!stbi__zexpand(a, a->zout, len)) return 0;
  memcpy(a->zout, a->zbuffer, len);
  a->zbuffer += len;
  a->zout += len;
  return 1;
}

static int stbi__parse_zlib_header(stbi__zbuf *a) {
  int cmf = stbi__zget8(a);
  int cm = cmf & 15;
  int flg = stbi__zget8(a);
  if (stbi__zeof(a)) return stbi__err("bad zlib header"); // zlib spec
  if ((cmf * 256 + flg) % 31 != 0) return stbi__err("bad zlib header"); // zlib spec
  if (flg & 32) return stbi__err("no preset dict"); // preset dictionary not allowed in png
  if (cm != 8) return stbi__err("bad compression"); // DEFLATE required for png
  return 1;
}

static const uint8_t stbi__zdefault_length[STBI__ZNSYMS] = {
  8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
  8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
  8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
  8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
  8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9,
  9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9,
  9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9,
  9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9,
  7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 8, 8, 8, 8, 8, 8, 8, 8
};
static const uint8_t stbi__zdefault_distance[32] = {
  5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5
};

static int stbi__parse_zlib(stbi__zbuf *a, int parse_header) {
  int final, type;
  if (parse_header)
    if (!stbi__parse_zlib_header(a)) return 0;
  a->num_bits = 0;
  a->code_buffer = 0;
  a->hit_zeof_once = 0;
  do {
    final = stbi__zreceive(a, 1);
    type = stbi__zreceive(a, 2);
    if (type == 0) {
      if (!stbi__parse_uncompressed_block(a)) return 0;
    } else if (type == 3) {
      return 0;
    } else {
      if (type == 1) {
        // use fixed code lengths
        if (!stbi__zbuild_huffman(&a->z_length, stbi__zdefault_length, STBI__ZNSYMS)) return 0;
        if (!stbi__zbuild_huffman(&a->z_distance, stbi__zdefault_distance, 32)) return 0;
      } else {
        if (!stbi__compute_huffman_codes(a)) return 0;
      }
      if (!stbi__parse_huffman_block(a)) return 0;
    }
  } while (!final);
  return 1;
}

static int stbi__do_zlib(stbi__zbuf *a, char *obuf, int olen, int exp, int parse_header) {
  a->zout_start = obuf;
  a->zout = obuf;
  a->zout_end = obuf + olen;
  a->z_expandable = exp;

  return stbi__parse_zlib(a, parse_header);
}

extern char *stbi_zlib_decode_malloc_guesssize(const char *buffer, int len, int initial_size, int *outlen) {
  stbi__zbuf a;
  char *p = (char *)malloc(initial_size);
  if (p == NULL) return NULL;
  a.zbuffer = (uint8_t *)buffer;
  a.zbuffer_end = (uint8_t *)buffer + len;
  if (stbi__do_zlib(&a, p, initial_size, 1, 1)) {
    if (outlen) *outlen = (int)(a.zout - a.zout_start);
    return a.zout_start;
  } else {
    free(a.zout_start);
    return NULL;
  }
}

extern char *stbi_zlib_decode_malloc(char const *buffer, int len, int *outlen) {
  return stbi_zlib_decode_malloc_guesssize(buffer, len, 16384, outlen);
}

extern char *stbi_zlib_decode_malloc_guesssize_headerflag(const char *buffer, int len, int initial_size, int *outlen, int parse_header) {
  stbi__zbuf a;
  char *p = (char *)malloc(initial_size);
  if (p == NULL) return NULL;
  a.zbuffer = (uint8_t *)buffer;
  a.zbuffer_end = (uint8_t *)buffer + len;
  if (stbi__do_zlib(&a, p, initial_size, 1, parse_header)) {
    if (outlen) *outlen = (int)(a.zout - a.zout_start);
    return a.zout_start;
  } else {
    free(a.zout_start);
    return NULL;
  }
}

extern int stbi_zlib_decode_buffer(char *obuffer, int olen, char const *ibuffer, int ilen) {
  stbi__zbuf a;
  a.zbuffer = (uint8_t *)ibuffer;
  a.zbuffer_end = (uint8_t *)ibuffer + ilen;
  if (stbi__do_zlib(&a, obuffer, olen, 0, 1))
    return (int)(a.zout - a.zout_start);
  else
    return -1;
}

extern char *stbi_zlib_decode_noheader_malloc(char const *buffer, int len, int *outlen) {
  stbi__zbuf a;
  char *p = (char *)malloc(16384);
  if (p == NULL) return NULL;
  a.zbuffer = (uint8_t *)buffer;
  a.zbuffer_end = (uint8_t *)buffer + len;
  if (stbi__do_zlib(&a, p, 16384, 1, 0)) {
    if (outlen) *outlen = (int)(a.zout - a.zout_start);
    return a.zout_start;
  } else {
    free(a.zout_start);
    return NULL;
  }
}

extern int stbi_zlib_decode_noheader_buffer(char *obuffer, int olen, const char *ibuffer, int ilen) {
  stbi__zbuf a;
  a.zbuffer = (uint8_t *)ibuffer;
  a.zbuffer_end = (uint8_t *)ibuffer + ilen;
  if (stbi__do_zlib(&a, obuffer, olen, 0, 0))
    return (int)(a.zout - a.zout_start);
  else
    return -1;
}

// public domain "baseline" PNG decoder   v0.10  Sean Barrett 2006-11-18
//    simple implementation
//      - only 8-bit samples
//      - no CRC checking
//      - allocates lots of intermediate memory
//        - avoids problem of streaming data between subsystems
//        - avoids explicit window management
//    performance
//      - uses stb_zlib, a PD zlib implementation with fast huffman decoding

typedef struct {
  uint32_t length;
  uint32_t type;
} stbi__pngchunk;

static stbi__pngchunk stbi__get_chunk_header(stbi__context *s) {
  stbi__pngchunk c;
  c.length = stbi__get32be(s);
  c.type = stbi__get32be(s);
  return c;
}

static int stbi__check_png_header(stbi__context *s) {
  static const uint8_t png_sig[8] = {137, 80, 78, 71, 13, 10, 26, 10};
  int i;
  for (i = 0; i < 8; ++i)
    if (stbi__get8(s) != png_sig[i]) return stbi__err("bad png sig");
  return 1;
}

typedef struct {
  stbi__context *s;
  uint8_t *idata, *expanded, *out;
  int depth;
} stbi__png;

enum {
  STBI__F_none = 0,
  STBI__F_sub = 1,
  STBI__F_up = 2,
  STBI__F_avg = 3,
  STBI__F_paeth = 4,
  // synthetic filter used for first scanline to avoid needing a dummy row of 0s
  STBI__F_avg_first
};

static uint8_t first_row_filter[5] = {
  STBI__F_none,
  STBI__F_sub,
  STBI__F_none,
  STBI__F_avg_first,
  STBI__F_sub // Paeth with b=c=0 turns out to be equivalent to sub
};

static int stbi__paeth(int a, int b, int c) {
  // This formulation looks very different from the reference in the PNG spec, but is actually equivalent and has favorable data dependencies and admits straightforward generation of branch-free code, which helps performance significantly.
  int thresh = c * 3 - (a + b);
  int lo = a < b ? a : b;
  int hi = a < b ? b : a;
  int t0 = (hi <= thresh) ? lo : c;
  int t1 = (thresh <= lo) ? hi : t0;
  return t1;
}

static const uint8_t stbi__depth_scale_table[9] = {0, 0xff, 0x55, 0, 0x11, 0, 0, 0, 0x01};

// adds an extra all-255 alpha channel
// dest == src is legal
// img_n must be 1 or 3
static void stbi__create_png_alpha_expand8(uint8_t *dest, uint8_t *src, uint32_t x, int img_n) {
  int i;
  // must process data backwards since we allow dest==src
  if (img_n == 1) {
    for (i = x - 1; i >= 0; --i) {
      dest[i * 2 + 1] = 255;
      dest[i * 2 + 0] = src[i];
    }
  } else {
    assert(img_n == 3);
    for (i = x - 1; i >= 0; --i) {
      dest[i * 4 + 3] = 255;
      dest[i * 4 + 2] = src[i * 3 + 2];
      dest[i * 4 + 1] = src[i * 3 + 1];
      dest[i * 4 + 0] = src[i * 3 + 0];
    }
  }
}

// create the png data from post-deflated data
static int stbi__create_png_image_raw(stbi__png *a, uint8_t *raw, uint32_t raw_len, int out_n, uint32_t x, uint32_t y, int depth, int color) {
  int bytes = (depth == 16 ? 2 : 1);
  stbi__context *s = a->s;
  uint32_t i, j, stride = x * out_n * bytes;
  uint32_t img_len, img_width_bytes;
  uint8_t *filter_buf;
  int all_ok = 1;
  int k;
  int img_n = s->img_n; // copy it into a local for later

  int output_bytes = out_n * bytes;
  int filter_bytes = img_n * bytes;
  int width = x;

  assert(out_n == s->img_n || out_n == s->img_n + 1);
  a->out = (uint8_t *)stbi__malloc_mad3(x, y, output_bytes, 0); // extra bytes to write off the end into
  if (!a->out) return stbi__err("outofmem");

  // note: error exits here don't need to clean up a->out individually,
  // stbi__do_png always does on error.
  if (!stbi__mad3sizes_valid(img_n, x, depth, 7)) return stbi__err("too large");
  img_width_bytes = (((img_n * x * depth) + 7) >> 3);
  if (!stbi__mad2sizes_valid(img_width_bytes, y, img_width_bytes)) return stbi__err("too large");
  img_len = (img_width_bytes + 1) * y;

  // we used to check for exact match between raw_len and img_len on non-interlaced PNGs,
  // but issue #276 reported a PNG in the wild that had extra data at the end (all zeros),
  // so just check for raw_len < img_len always.
  if (raw_len < img_len) return stbi__err("not enough pixels");

  // Allocate two scan lines worth of filter workspace buffer.
  filter_buf = (uint8_t *)stbi__malloc_mad2(img_width_bytes, 2, 0);
  if (!filter_buf) return stbi__err("outofmem");

  // Filtering for low-bit-depth images
  if (depth < 8) {
    filter_bytes = 1;
    width = img_width_bytes;
  }

  for (j = 0; j < y; ++j) {
    // cur/prior filter buffers alternate
    uint8_t *cur = filter_buf + (j & 1) * img_width_bytes;
    uint8_t *prior = filter_buf + (~j & 1) * img_width_bytes;
    uint8_t *dest = a->out + stride * j;
    int nk = width * filter_bytes;
    int filter = *raw++;

    // check filter type
    if (filter > 4) {
      all_ok = stbi__err("invalid filter");
      break;
    }

    // if first row, use special filter that doesn't sample previous row
    if (j == 0) filter = first_row_filter[filter];

    // perform actual filtering
    switch (filter) {
      case STBI__F_none:
        memcpy(cur, raw, nk);
        break;
      case STBI__F_sub:
        memcpy(cur, raw, filter_bytes);
        for (k = filter_bytes; k < nk; ++k)
          cur[k] = STBI__BYTECAST(raw[k] + cur[k - filter_bytes]);
        break;
      case STBI__F_up:
        for (k = 0; k < nk; ++k)
          cur[k] = STBI__BYTECAST(raw[k] + prior[k]);
        break;
      case STBI__F_avg:
        for (k = 0; k < filter_bytes; ++k)
          cur[k] = STBI__BYTECAST(raw[k] + (prior[k] >> 1));
        for (k = filter_bytes; k < nk; ++k)
          cur[k] = STBI__BYTECAST(raw[k] + ((prior[k] + cur[k - filter_bytes]) >> 1));
        break;
      case STBI__F_paeth:
        for (k = 0; k < filter_bytes; ++k)
          cur[k] = STBI__BYTECAST(raw[k] + prior[k]); // prior[k] == stbi__paeth(0,prior[k],0)
        for (k = filter_bytes; k < nk; ++k)
          cur[k] = STBI__BYTECAST(raw[k] + stbi__paeth(cur[k - filter_bytes], prior[k], prior[k - filter_bytes]));
        break;
      case STBI__F_avg_first:
        memcpy(cur, raw, filter_bytes);
        for (k = filter_bytes; k < nk; ++k)
          cur[k] = STBI__BYTECAST(raw[k] + (cur[k - filter_bytes] >> 1));
        break;
    }

    raw += nk;

    // expand decoded bits in cur to dest, also adding an extra alpha channel if desired
    if (depth < 8) {
      uint8_t scale = (color == 0) ? stbi__depth_scale_table[depth] : 1; // scale grayscale values to 0..255 range
      uint8_t *in = cur;
      uint8_t *out = dest;
      uint8_t inb = 0;
      uint32_t nsmp = x * img_n;

      // expand bits to bytes first
      if (depth == 4) {
        for (i = 0; i < nsmp; ++i) {
          if ((i & 1) == 0) inb = *in++;
          *out++ = scale * (inb >> 4);
          inb <<= 4;
        }
      } else if (depth == 2) {
        for (i = 0; i < nsmp; ++i) {
          if ((i & 3) == 0) inb = *in++;
          *out++ = scale * (inb >> 6);
          inb <<= 2;
        }
      } else {
        assert(depth == 1);
        for (i = 0; i < nsmp; ++i) {
          if ((i & 7) == 0) inb = *in++;
          *out++ = scale * (inb >> 7);
          inb <<= 1;
        }
      }

      // insert alpha=255 values if desired
      if (img_n != out_n)
        stbi__create_png_alpha_expand8(dest, dest, x, img_n);
    } else if (depth == 8) {
      if (img_n == out_n)
        memcpy(dest, cur, x * img_n);
      else
        stbi__create_png_alpha_expand8(dest, cur, x, img_n);
    } else if (depth == 16) {
      // convert the image data from big-endian to platform-native
      uint16_t *dest16 = (uint16_t *)dest;
      uint32_t nsmp = x * img_n;

      if (img_n == out_n) {
        for (i = 0; i < nsmp; ++i, ++dest16, cur += 2)
          *dest16 = (cur[0] << 8) | cur[1];
      } else {
        assert(img_n + 1 == out_n);
        if (img_n == 1) {
          for (i = 0; i < x; ++i, dest16 += 2, cur += 2) {
            dest16[0] = (cur[0] << 8) | cur[1];
            dest16[1] = 0xffff;
          }
        } else {
          assert(img_n == 3);
          for (i = 0; i < x; ++i, dest16 += 4, cur += 6) {
            dest16[0] = (cur[0] << 8) | cur[1];
            dest16[1] = (cur[2] << 8) | cur[3];
            dest16[2] = (cur[4] << 8) | cur[5];
            dest16[3] = 0xffff;
          }
        }
      }
    }
  }

  free(filter_buf);
  if (!all_ok) return 0;

  return 1;
}

static int stbi__create_png_image(stbi__png *a, uint8_t *image_data, uint32_t image_data_len, int out_n, int depth, int color, int interlaced) {
  int bytes = (depth == 16 ? 2 : 1);
  int out_bytes = out_n * bytes;
  uint8_t *final;
  int p;
  if (!interlaced)
    return stbi__create_png_image_raw(a, image_data, image_data_len, out_n, a->s->img_x, a->s->img_y, depth, color);

  // de-interlacing
  final = (uint8_t *)stbi__malloc_mad3(a->s->img_x, a->s->img_y, out_bytes, 0);
  if (!final) return stbi__err("outofmem");
  for (p = 0; p < 7; ++p) {
    int xorig[] = {0, 4, 0, 2, 0, 1, 0};
    int yorig[] = {0, 0, 4, 0, 2, 0, 1};
    int xspc[] = {8, 8, 4, 4, 2, 2, 1};
    int yspc[] = {8, 8, 8, 4, 4, 2, 2};
    int i, j, x, y;
    // pass1_x[4] = 0, pass1_x[5] = 1, pass1_x[12] = 1
    x = (a->s->img_x - xorig[p] + xspc[p] - 1) / xspc[p];
    y = (a->s->img_y - yorig[p] + yspc[p] - 1) / yspc[p];
    if (x && y) {
      uint32_t img_len = ((((a->s->img_n * x * depth) + 7) >> 3) + 1) * y;
      if (!stbi__create_png_image_raw(a, image_data, image_data_len, out_n, x, y, depth, color)) {
        free(final);
        return 0;
      }
      for (j = 0; j < y; ++j) {
        for (i = 0; i < x; ++i) {
          int out_y = j * yspc[p] + yorig[p];
          int out_x = i * xspc[p] + xorig[p];
          memcpy(final + out_y * a->s->img_x * out_bytes + out_x * out_bytes, a->out + (j * x + i) * out_bytes, out_bytes);
        }
      }
      free(a->out);
      image_data += img_len;
      image_data_len -= img_len;
    }
  }
  a->out = final;

  return 1;
}

static int stbi__compute_transparency(stbi__png *z, uint8_t tc[3], int out_n) {
  stbi__context *s = z->s;
  uint32_t i, pixel_count = s->img_x * s->img_y;
  uint8_t *p = z->out;

  // compute color-based transparency, assuming we've
  // already got 255 as the alpha value in the output
  assert(out_n == 2 || out_n == 4);

  if (out_n == 2) {
    for (i = 0; i < pixel_count; ++i) {
      p[1] = (p[0] == tc[0] ? 0 : 255);
      p += 2;
    }
  } else {
    for (i = 0; i < pixel_count; ++i) {
      if (p[0] == tc[0] && p[1] == tc[1] && p[2] == tc[2])
        p[3] = 0;
      p += 4;
    }
  }
  return 1;
}

static int stbi__compute_transparency16(stbi__png *z, uint16_t tc[3], int out_n) {
  stbi__context *s = z->s;
  uint32_t i, pixel_count = s->img_x * s->img_y;
  uint16_t *p = (uint16_t *)z->out;

  // compute color-based transparency, assuming we've
  // already got 65535 as the alpha value in the output
  assert(out_n == 2 || out_n == 4);

  if (out_n == 2) {
    for (i = 0; i < pixel_count; ++i) {
      p[1] = (p[0] == tc[0] ? 0 : 65535);
      p += 2;
    }
  } else {
    for (i = 0; i < pixel_count; ++i) {
      if (p[0] == tc[0] && p[1] == tc[1] && p[2] == tc[2])
        p[3] = 0;
      p += 4;
    }
  }
  return 1;
}

static int stbi__expand_png_palette(stbi__png *a, uint8_t *palette, int len, int pal_img_n) {
  uint32_t i, pixel_count = a->s->img_x * a->s->img_y;
  uint8_t *p, *temp_out, *orig = a->out;

  p = (uint8_t *)stbi__malloc_mad2(pixel_count, pal_img_n, 0);
  if (p == NULL) return stbi__err("outofmem");

  // between here and free(out) below, exiting would leak
  temp_out = p;

  if (pal_img_n == 3) {
    for (i = 0; i < pixel_count; ++i) {
      int n = orig[i] * 4;
      p[0] = palette[n];
      p[1] = palette[n + 1];
      p[2] = palette[n + 2];
      p += 3;
    }
  } else {
    for (i = 0; i < pixel_count; ++i) {
      int n = orig[i] * 4;
      p[0] = palette[n];
      p[1] = palette[n + 1];
      p[2] = palette[n + 2];
      p[3] = palette[n + 3];
      p += 4;
    }
  }
  free(a->out);
  a->out = temp_out;

  STBI_NOTUSED(len);

  return 1;
}

#define STBI__PNG_TYPE(a, b, c, d) (((unsigned)(a) << 24) + ((unsigned)(b) << 16) + ((unsigned)(c) << 8) + (unsigned)(d))

static int stbi__parse_png_file(stbi__png *z, int scan, int req_comp) {
  uint8_t palette[1024], pal_img_n = 0;
  uint8_t has_trans = 0, tc[3] = {0};
  uint16_t tc16[3];
  uint32_t ioff = 0, idata_limit = 0, i, pal_len = 0;
  int first = 1, k, interlace = 0, color = 0;
  stbi__context *s = z->s;

  z->expanded = NULL;
  z->idata = NULL;
  z->out = NULL;

  if (!stbi__check_png_header(s)) return 0;

  if (scan == STBI__SCAN_type) return 1;

  while (true) {
    stbi__pngchunk c = stbi__get_chunk_header(s);
    switch (c.type) {
      case STBI__PNG_TYPE('I', 'H', 'D', 'R'): {
        int comp, filter;
        if (!first) return stbi__err("multiple IHDR");
        first = 0;
        if (c.length != 13) return stbi__err("bad IHDR len");
        s->img_x = stbi__get32be(s);
        s->img_y = stbi__get32be(s);
        if (s->img_y > STBI_MAX_DIMENSIONS) return stbi__err("too large y");
        if (s->img_x > STBI_MAX_DIMENSIONS) return stbi__err("too large");
        z->depth = stbi__get8(s);
        if (z->depth != 1 && z->depth != 2 && z->depth != 4 && z->depth != 8 && z->depth != 16) return stbi__err("1/2/4/8/16-bit only");
        color = stbi__get8(s);
        if (color > 6) return stbi__err("bad ctype");
        if (color == 3 && z->depth == 16) return stbi__err("bad ctype");
        if (color == 3)
          pal_img_n = 3;
        else if (color & 1)
          return stbi__err("bad ctype");
        comp = stbi__get8(s);
        if (comp) return stbi__err("bad comp method");
        filter = stbi__get8(s);
        if (filter) return stbi__err("bad filter method");
        interlace = stbi__get8(s);
        if (interlace > 1) return stbi__err("bad interlace method");
        if (!s->img_x || !s->img_y) return stbi__err("0-pixel image");
        if (!pal_img_n) {
          s->img_n = (color & 2 ? 3 : 1) + (color & 4 ? 1 : 0);
          if ((1 << 30) / s->img_x / s->img_n < s->img_y) return stbi__err("too large");
        } else {
          // if paletted, then pal_n is our final components, and
          // img_n is # components to decompress/filter.
          s->img_n = 1;
          if ((1 << 30) / s->img_x / 4 < s->img_y) return stbi__err("too large");
        }
        // even with SCAN_header, have to scan to see if we have a tRNS
        break;
      }

      case STBI__PNG_TYPE('P', 'L', 'T', 'E'): {
        if (first) return stbi__err("first not IHDR");
        if (c.length > 256 * 3) return stbi__err("invalid PLTE");
        pal_len = c.length / 3;
        if (pal_len * 3 != c.length) return stbi__err("invalid PLTE");
        for (i = 0; i < pal_len; ++i) {
          palette[i * 4 + 0] = stbi__get8(s);
          palette[i * 4 + 1] = stbi__get8(s);
          palette[i * 4 + 2] = stbi__get8(s);
          palette[i * 4 + 3] = 255;
        }
        break;
      }

      case STBI__PNG_TYPE('t', 'R', 'N', 'S'): {
        if (first) return stbi__err("first not IHDR");
        if (z->idata) return stbi__err("tRNS after IDAT");
        if (pal_img_n) {
          if (scan == STBI__SCAN_header) {
            s->img_n = 4;
            return 1;
          }
          if (pal_len == 0) return stbi__err("tRNS before PLTE");
          if (c.length > pal_len) return stbi__err("bad tRNS len");
          pal_img_n = 4;
          for (i = 0; i < c.length; ++i)
            palette[i * 4 + 3] = stbi__get8(s);
        } else {
          if (!(s->img_n & 1)) return stbi__err("tRNS with alpha");
          if (c.length != (uint32_t)s->img_n * 2) return stbi__err("bad tRNS len");
          has_trans = 1;
          // non-paletted with tRNS = constant alpha. if header-scanning, we can stop now.
          if (scan == STBI__SCAN_header) {
            ++s->img_n;
            return 1;
          }
          if (z->depth == 16) {
            for (k = 0; k < s->img_n && k < 3; ++k) // extra loop test to suppress false GCC warning
              tc16[k] = (uint16_t)stbi__get16be(s); // copy the values as-is
          } else {
            for (k = 0; k < s->img_n && k < 3; ++k)
              tc[k] = (uint8_t)(stbi__get16be(s) & 255) * stbi__depth_scale_table[z->depth]; // non 8-bit images will be larger
          }
        }
        break;
      }

      case STBI__PNG_TYPE('I', 'D', 'A', 'T'): {
        if (first) return stbi__err("first not IHDR");
        if (pal_img_n && !pal_len) return stbi__err("no PLTE");
        if (scan == STBI__SCAN_header) {
          // header scan definitely stops at first IDAT
          if (pal_img_n)
            s->img_n = pal_img_n;
          return 1;
        }
        if (c.length > (1u << 30)) return stbi__err("IDAT size limit");
        if ((int)(ioff + c.length) < (int)ioff) return 0;
        if (ioff + c.length > idata_limit) {
          uint32_t idata_limit_old = idata_limit;
          uint8_t *p;
          if (idata_limit == 0) idata_limit = c.length > 4096 ? c.length : 4096;
          while (ioff + c.length > idata_limit)
            idata_limit *= 2;
          STBI_NOTUSED(idata_limit_old);
          p = (uint8_t *)STBI_REALLOC_SIZED(z->idata, idata_limit_old, idata_limit);
          if (p == NULL) return stbi__err("outofmem");
          z->idata = p;
        }
        if (!stbi__getn(s, z->idata + ioff, c.length)) return stbi__err("outofdata");
        ioff += c.length;
        break;
      }

      case STBI__PNG_TYPE('I', 'E', 'N', 'D'): {
        uint32_t raw_len, bpl;
        if (first) return stbi__err("first not IHDR");
        if (scan != STBI__SCAN_load) return 1;
        if (z->idata == NULL) return stbi__err("no IDAT");
        // initial guess for decoded data size to avoid unnecessary reallocs
        bpl = (s->img_x * z->depth + 7) / 8; // bytes per line, per component
        raw_len = bpl * s->img_y * s->img_n + s->img_y;
        z->expanded = (uint8_t *)stbi_zlib_decode_malloc_guesssize_headerflag((char *)z->idata, ioff, raw_len, (int *)&raw_len, true);
        if (z->expanded == NULL) return 0; // zlib should set error
        free(z->idata);
        z->idata = NULL;
        if ((req_comp == s->img_n + 1 && req_comp != 3 && !pal_img_n) || has_trans)
          s->img_out_n = s->img_n + 1;
        else
          s->img_out_n = s->img_n;
        if (!stbi__create_png_image(z, z->expanded, raw_len, s->img_out_n, z->depth, color, interlace)) return 0;
        if (has_trans) {
          if (z->depth == 16) {
            if (!stbi__compute_transparency16(z, tc16, s->img_out_n)) return 0;
          } else {
            if (!stbi__compute_transparency(z, tc, s->img_out_n)) return 0;
          }
        }
        if (pal_img_n) {
          // pal_img_n == 3 or 4
          s->img_n = pal_img_n; // record the actual colors we had
          s->img_out_n = pal_img_n;
          if (req_comp >= 3) s->img_out_n = req_comp;
          if (!stbi__expand_png_palette(z, palette, pal_len, s->img_out_n))
            return 0;
        } else if (has_trans) {
          // non-paletted image with tRNS -> source image has (constant) alpha
          ++s->img_n;
        }
        free(z->expanded);
        z->expanded = NULL;
        // end of PNG chunk, read and skip CRC
        stbi__get32be(s);
        return 1;
      }

      default:
        // if critical, fail
        if (first) return stbi__err("first not IHDR");
        if ((c.type & (1 << 29)) == 0) {
          // not threadsafe
          static char invalid_chunk[] = "XXXX PNG chunk not known";
          invalid_chunk[0] = STBI__BYTECAST(c.type >> 24);
          invalid_chunk[1] = STBI__BYTECAST(c.type >> 16);
          invalid_chunk[2] = STBI__BYTECAST(c.type >> 8);
          invalid_chunk[3] = STBI__BYTECAST(c.type >> 0);
          return stbi__err(invalid_chunk);
        }
        stbi__skip(s, c.length);
        break;
    }
    // end of PNG chunk, read and skip CRC
    stbi__get32be(s);
  }
}

static void *stbi__do_png(stbi__png *p, int *x, int *y, int *n, int req_comp, stbi__result_info *ri) {
  void *result = NULL;
  if (req_comp < 0 || req_comp > 4) return stbi__errpuc("bad req_comp");
  if (stbi__parse_png_file(p, STBI__SCAN_load, req_comp)) {
    if (p->depth <= 8)
      ri->bits_per_channel = 8;
    else if (p->depth == 16)
      ri->bits_per_channel = 16;
    else
      return stbi__errpuc("bad bits_per_channel");
    result = p->out;
    p->out = NULL;
    if (req_comp && req_comp != p->s->img_out_n) {
      if (ri->bits_per_channel == 8)
        result = stbi__convert_format((unsigned char *)result, p->s->img_out_n, req_comp, p->s->img_x, p->s->img_y);
      else
        result = stbi__convert_format16((uint16_t *)result, p->s->img_out_n, req_comp, p->s->img_x, p->s->img_y);
      p->s->img_out_n = req_comp;
      if (result == NULL) return result;
    }
    *x = p->s->img_x;
    *y = p->s->img_y;
    if (n) *n = p->s->img_n;
  }
  free(p->out);
  p->out = NULL;
  free(p->expanded);
  p->expanded = NULL;
  free(p->idata);
  p->idata = NULL;

  return result;
}

static void *stbi__png_load(stbi__context *s, int *x, int *y, int *comp, int req_comp, stbi__result_info *ri) {
  stbi__png p = {.s = s};
  return stbi__do_png(&p, x, y, comp, req_comp, ri);
}

static int stbi__png_test(stbi__context *s) {
  int r = stbi__check_png_header(s);
  stbi__rewind(s);
  return r;
}

static int stbi__png_info_raw(stbi__png *p, int *x, int *y, int *comp) {
  if (!stbi__parse_png_file(p, STBI__SCAN_header, 0)) {
    stbi__rewind(p->s);
    return 0;
  }
  if (x) *x = p->s->img_x;
  if (y) *y = p->s->img_y;
  if (comp) *comp = p->s->img_n;
  return 1;
}

static int stbi__png_info(stbi__context *s, int *x, int *y, int *comp) {
  stbi__png p = {.s = s};
  return stbi__png_info_raw(&p, x, y, comp);
}

static int stbi__png_is16(stbi__context *s) {
  stbi__png p = {.s = s};
  if (!stbi__png_info_raw(&p, NULL, NULL, NULL))
    return 0;
  if (p.depth != 16) {
    stbi__rewind(p.s);
    return 0;
  }
  return 1;
}

static int stbi__info_main(stbi__context *s, int *x, int *y, int *comp) {
  if (stbi__png_info(s, x, y, comp)) return 1;
  return stbi__err("unknown image type");
}

static int stbi__is_16_main(stbi__context *s) {
  if (stbi__png_is16(s)) return 1;
  return 0;
}

extern int stbi_info(char const *filename, int *x, int *y, int *comp) {
  FILE *f = stbi__fopen(filename, "rb");
  if (!f) return stbi__err("can't fopen");
  int result = stbi_info_from_file(f, x, y, comp);
  fclose(f);
  return result;
}

extern int stbi_info_from_file(FILE *f, int *x, int *y, int *comp) {
  stbi__context s;
  long pos = ftell(f);
  stbi__start_file(&s, f);
  int r = stbi__info_main(&s, x, y, comp);
  fseek(f, pos, SEEK_SET);
  return r;
}

extern int stbi_is_16_bit(char const *filename) {
  FILE *f = stbi__fopen(filename, "rb");
  if (!f) return stbi__err("can't fopen");
  int result = stbi_is_16_bit_from_file(f);
  fclose(f);
  return result;
}

extern int stbi_is_16_bit_from_file(FILE *f) {
  stbi__context s;
  long pos = ftell(f);
  stbi__start_file(&s, f);
  int r = stbi__is_16_main(&s);
  fseek(f, pos, SEEK_SET);
  return r;
}

extern int stbi_info_from_memory(uint8_t const *buffer, int len, int *x, int *y, int *comp) {
  stbi__context s;
  stbi__start_mem(&s, buffer, len);
  return stbi__info_main(&s, x, y, comp);
}

extern int stbi_is_16_bit_from_memory(uint8_t const *buffer, int len) {
  stbi__context s;
  stbi__start_mem(&s, buffer, len);
  return stbi__is_16_main(&s);
}

#define STBI_INCLUDE_STB_IMAGE
#endif // STBI_INCLUDE_STB_IMAGE
