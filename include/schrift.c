// This file is based on libschrift (C) 2019-2022 Thomas Oltmann and contributors
// This file is not conformant to the C9 standard
#ifndef SCHRIFT_TTF

#include <assert.h> // assert
#include <errno.h> // errno, ENOMEM
#include <fcntl.h> // open
#include <math.h> // fabs, floor
#include <stddef.h> // size_t
#include <stdint.h> // uint_fast32_t, uint_least32_t
#include <stdio.h>
#include <stdlib.h> // calloc, free
#include <string.h>
#include <sys/mman.h> // mmap, munmap
#include <sys/stat.h> // fstat
#include <unistd.h> // close

typedef struct SFT SFT;
typedef struct SFT_Font SFT_Font;
typedef uint_least32_t SFT_UChar; // Guaranteed to be compatible with char32_t.
typedef uint_fast32_t SFT_Glyph;
typedef struct SFT_LMetrics SFT_LMetrics;
typedef struct SFT_GMetrics SFT_GMetrics;
typedef struct SFT_Kerning SFT_Kerning;
typedef struct SFT_Image SFT_Image;

struct SFT {
  SFT_Font *font;
  double xScale;
  double yScale;
  double xOffset;
  double yOffset;
  int flags;
};

struct SFT_LMetrics {
  double ascender;
  double descender;
  double lineGap;
};

struct SFT_GMetrics {
  double advanceWidth;
  double leftSideBearing;
  int yOffset;
  int minWidth;
  int minHeight;
};

struct SFT_Kerning {
  double xShift;
  double yShift;
};

struct SFT_Image {
  void *pixels;
  int width;
  int height;
};

void sft_freefont(SFT_Font *font);

#define FILE_MAGIC_ONE 0x00010000
#define FILE_MAGIC_TWO 0x74727565

#define HORIZONTAL_KERNING 0x01
#define MINIMUM_KERNING 0x02
#define CROSS_STREAM_KERNING 0x04
#define OVERRIDE_KERNING 0x08

#define POINT_IS_ON_CURVE 0x01
#define X_CHANGE_IS_SMALL 0x02
#define Y_CHANGE_IS_SMALL 0x04
#define REPEAT_FLAG 0x08
#define X_CHANGE_IS_ZERO 0x10
#define X_CHANGE_IS_POSITIVE 0x10
#define Y_CHANGE_IS_ZERO 0x20
#define Y_CHANGE_IS_POSITIVE 0x20

#define OFFSETS_ARE_LARGE 0x001
#define ACTUAL_XY_OFFSETS 0x002
#define GOT_A_SINGLE_SCALE 0x008
#define THERE_ARE_MORE_COMPONENTS 0x020
#define GOT_AN_X_AND_Y_SCALE 0x040
#define GOT_A_SCALE_MATRIX 0x080

// macros
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define SIGN(x) (((x) > 0) - ((x) < 0))
// Allocate values on the stack if they are small enough, else spill to heap.
#define STACK_ALLOC(var, type, thresh, count) \
  type var##_stack_[thresh]; \
  var = (count) <= (thresh) ? var##_stack_ : calloc(sizeof(type), count);
#define STACK_FREE(var) \
  if (var != var##_stack_) free(var);

enum { SrcMapping,
       SrcUser };

// structs
typedef struct SFT_Point SFT_Point;
typedef struct SFT_Line SFT_Line;
typedef struct SFT_Curve SFT_Curve;
typedef struct SFT_Cell SFT_Cell;
typedef struct SFT_Outline SFT_Outline;
typedef struct SFT_Raster SFT_Raster;

struct SFT_Point {
  double x, y;
};
struct SFT_Line {
  uint_least16_t beg, end;
};
struct SFT_Curve {
  uint_least16_t beg, end, ctrl;
};
struct SFT_Cell {
  double area, cover;
};

struct SFT_Outline {
  SFT_Point *points;
  SFT_Curve *curves;
  SFT_Line *lines;
  uint_least16_t numPoints;
  uint_least16_t capPoints;
  uint_least16_t numCurves;
  uint_least16_t capCurves;
  uint_least16_t numLines;
  uint_least16_t capLines;
};

struct SFT_Raster {
  SFT_Cell *cells;
  int width;
  int height;
};

struct SFT_Font {
  const uint8_t *memory;
  uint_fast32_t size;
  int source;
  uint_least16_t unitsPerEm;
  int_least16_t locaFormat;
  uint_least16_t numLongHmtx;
};

// function declarations
// generic utility functions
static void *reallocarray(void *optr, size_t nmemb, size_t size);
static inline int fast_floor(double x);
static inline int fast_ceil(double x);
// file loading
static int map_file(SFT_Font *font, const char *filename);
static void unmap_file(SFT_Font *font);
static int init_font(SFT_Font *font);
// simple mathematical operations
static SFT_Point midpoint(SFT_Point a, SFT_Point b);
static void transform_points(unsigned int numPts, SFT_Point *points, double trf[6]);
static void clip_points(unsigned int numPts, SFT_Point *points, int width, int height);
// 'outline' data structure management
static int init_outline(SFT_Outline *outl);
static void free_outline(SFT_Outline *outl);
static int grow_points(SFT_Outline *outl);
static int grow_curves(SFT_Outline *outl);
static int grow_lines(SFT_Outline *outl);
// TTF parsing utilities
static inline int is_safe_offset(SFT_Font *font, uint_fast32_t offset, uint_fast32_t margin);
static void *csearch(const void *key, const void *base, size_t nmemb, size_t size, int (*compar)(const void *, const void *));
static int cmpu16(const void *a, const void *b);
static int cmpu32(const void *a, const void *b);
static inline uint_least8_t getu8(SFT_Font *font, uint_fast32_t offset);
static inline int_least8_t geti8(SFT_Font *font, uint_fast32_t offset);
static inline uint_least16_t getu16(SFT_Font *font, uint_fast32_t offset);
static inline int_least16_t geti16(SFT_Font *font, uint_fast32_t offset);
static inline uint_least32_t getu32(SFT_Font *font, uint_fast32_t offset);
static int gettable(SFT_Font *font, char tag[4], uint_fast32_t *offset);
// codepoint to glyph id translation
static int cmap_fmt4(SFT_Font *font, uint_fast32_t table, SFT_UChar charCode, uint_fast32_t *glyph);
static int cmap_fmt6(SFT_Font *font, uint_fast32_t table, SFT_UChar charCode, uint_fast32_t *glyph);
static int glyph_id(SFT_Font *font, SFT_UChar charCode, uint_fast32_t *glyph);
// glyph metrics lookup
static int hor_metrics(SFT_Font *font, uint_fast32_t glyph, int *advanceWidth, int *leftSideBearing);
static int glyph_bbox(const SFT *sft, uint_fast32_t outline, int box[4]);
// decoding outlines
static int outline_offset(SFT_Font *font, uint_fast32_t glyph, uint_fast32_t *offset);
static int simple_flags(SFT_Font *font, uint_fast32_t *offset, uint_fast16_t numPts, uint8_t *flags);
static int simple_points(SFT_Font *font, uint_fast32_t offset, uint_fast16_t numPts, uint8_t *flags, SFT_Point *points);
static int decode_contour(uint8_t *flags, uint_fast16_t basePoint, uint_fast16_t count, SFT_Outline *outl);
static int simple_outline(SFT_Font *font, uint_fast32_t offset, unsigned int numContours, SFT_Outline *outl);
static int compound_outline(SFT_Font *font, uint_fast32_t offset, int recDepth, SFT_Outline *outl);
static int decode_outline(SFT_Font *font, uint_fast32_t offset, int recDepth, SFT_Outline *outl);
// tesselation
static int is_flat(SFT_Outline *outl, SFT_Curve curve);
static int tesselate_curve(SFT_Curve curve, SFT_Outline *outl);
static int tesselate_curves(SFT_Outline *outl);
// silhouette rasterization
static void draw_line(SFT_Raster buf, SFT_Point origin, SFT_Point goal);
static void draw_lines(SFT_Outline *outl, SFT_Raster buf);
// post-processing
static void post_process(SFT_Raster buf, uint8_t *image);
// glyph rendering
static int render_outline(SFT_Outline *outl, double transform[6], SFT_Image image);

// function implementations

// Loads a font from a user-supplied memory range.
SFT_Font *sft_loadmem(const void *mem, size_t size) {
  SFT_Font *font;
  if (size > UINT32_MAX) {
    return NULL;
  }
  if (!(font = calloc(1, sizeof *font))) {
    return NULL;
  }
  font->memory = mem;
  font->size = (uint_fast32_t)size;
  font->source = SrcUser;
  if (init_font(font) < 0) {
    sft_freefont(font);
    return NULL;
  }
  return font;
}

// Loads a font from the file system. To do so, it has to map the entire font into memory.
SFT_Font *sft_loadfile(char const *filename) {
  SFT_Font *font;
  if (!(font = calloc(1, sizeof *font))) {
    return NULL;
  }
  if (map_file(font, filename) < 0) {
    printf("Failed to map file\n");
    free(font);
    return NULL;
  }
  if (init_font(font) < 0) {
    printf("Failed to init font\n");
    sft_freefont(font);
    return NULL;
  }
  return font;
}

void sft_freefont(SFT_Font *font) {
  if (!font) return;
  // Only unmap if we mapped it ourselves.
  if (font->source == SrcMapping)
    unmap_file(font);
  free(font);
}

int sft_lmetrics(const SFT *sft, SFT_LMetrics *metrics) {
  double factor;
  uint_fast32_t hhea;
  memset(metrics, 0, sizeof *metrics);
  if (gettable(sft->font, "hhea", &hhea) < 0) return -1;
  if (!is_safe_offset(sft->font, hhea, 36)) return -1;
  factor = sft->yScale / sft->font->unitsPerEm;
  metrics->ascender = geti16(sft->font, hhea + 4) * factor;
  metrics->descender = geti16(sft->font, hhea + 6) * factor;
  metrics->lineGap = geti16(sft->font, hhea + 8) * factor;
  return 0;
}

int sft_lookup(const SFT *sft, SFT_UChar codepoint, SFT_Glyph *glyph) {
  return glyph_id(sft->font, codepoint, glyph);
}

int sft_gmetrics(const SFT *sft, SFT_Glyph glyph, SFT_GMetrics *metrics) {
  int adv, lsb;
  double xScale = sft->xScale / sft->font->unitsPerEm;
  uint_fast32_t outline;
  int bbox[4];

  memset(metrics, 0, sizeof *metrics);

  if (hor_metrics(sft->font, glyph, &adv, &lsb) < 0) return -1;
  metrics->advanceWidth = adv * xScale;
  metrics->leftSideBearing = lsb * xScale;

  if (outline_offset(sft->font, glyph, &outline) < 0) return -1;
  if (!outline) return 0;
  if (glyph_bbox(sft, outline, bbox) < 0) return -1;
  metrics->minWidth = bbox[2] - bbox[0] + 1;
  metrics->minHeight = bbox[3] - bbox[1] + 1;
  metrics->yOffset = -bbox[3];

  return 0;
}

int sft_kerning(const SFT *sft, SFT_Glyph leftGlyph, SFT_Glyph rightGlyph, SFT_Kerning *kerning) {
  void *match;
  uint_fast32_t offset;
  unsigned int numTables, numPairs, length, format, flags;
  int value;
  uint8_t key[4];

  memset(kerning, 0, sizeof *kerning);

  if (gettable(sft->font, "kern", &offset) < 0) {
    printf("Kern table not found\n");
    return 0;
  };

  // Read kern table header.
  if (!is_safe_offset(sft->font, offset, 4)) return -1;
  if (getu16(sft->font, offset) != 0) {
    printf("Kern table format not supported\n");
    return 0;
  };
  numTables = getu16(sft->font, offset + 2);
  offset += 4;

  while (numTables > 0) {
    // Read subtable header.
    if (!is_safe_offset(sft->font, offset, 6))
      return -1;
    length = getu16(sft->font, offset + 2);
    format = getu8(sft->font, offset + 4);
    flags = getu8(sft->font, offset + 5);
    offset += 6;

    if (format == 0 && (flags & HORIZONTAL_KERNING) && !(flags & MINIMUM_KERNING)) {
      // Read format 0 header.
      if (!is_safe_offset(sft->font, offset, 8))
        return -1;
      numPairs = getu16(sft->font, offset);
      offset += 8;
      // Look up character code pair via binary search.
      key[0] = (leftGlyph >> 8) & 0xFF;
      key[1] = leftGlyph & 0xFF;
      key[2] = (rightGlyph >> 8) & 0xFF;
      key[3] = rightGlyph & 0xFF;
      if ((match = bsearch(key, sft->font->memory + offset, numPairs, 6, cmpu32)) != NULL) {
        value = geti16(sft->font, (uint_fast32_t)((uint8_t *)match - sft->font->memory + 4));
        if (flags & CROSS_STREAM_KERNING) {
          kerning->yShift += value;
        } else {
          kerning->xShift += value;
        }
      }
    }

    offset += length;
    --numTables;
  }

  kerning->xShift = kerning->xShift / sft->font->unitsPerEm * sft->xScale;
  kerning->yShift = kerning->yShift / sft->font->unitsPerEm * sft->yScale;

  return 0;
}

int sft_render(const SFT *sft, SFT_Glyph glyph, SFT_Image image) {
  uint_fast32_t outline;
  double transform[6];
  int bbox[4];
  SFT_Outline outl;

  if (outline_offset(sft->font, glyph, &outline) < 0) return -1;
  if (!outline) return 0;
  if (glyph_bbox(sft, outline, bbox) < 0) return -1;
  // Set up the transformation matrix such that
  // the transformed bounding boxes min corner lines
  // up with the (0, 0) point.
  transform[0] = sft->xScale / sft->font->unitsPerEm;
  transform[1] = 0.0;
  transform[2] = 0.0;
  transform[4] = sft->xOffset - bbox[0];
  transform[3] = -sft->yScale / sft->font->unitsPerEm;
  transform[5] = bbox[3] - sft->yOffset;

  memset(&outl, 0, sizeof outl);
  if (init_outline(&outl) < 0) {
    free_outline(&outl);
    return -1;
  }
  if (decode_outline(sft->font, outline, 0, &outl) < 0) {
    free_outline(&outl);
    return -1;
  }
  if (render_outline(&outl, transform, image) < 0) {
    free_outline(&outl);
    return -1;
  }
  free_outline(&outl);
  return 0;
}

// This is sqrt(SIZE_MAX+1), as s1*s2 <= SIZE_MAX
// if both s1 < MUL_NO_OVERFLOW and s2 < MUL_NO_OVERFLOW
#define MUL_NO_OVERFLOW ((size_t)1 << (sizeof(size_t) * 4))

// OpenBSD's reallocarray() standard libary function.
// A wrapper for realloc() that takes two size args like calloc().
// Useful because it eliminates common integer overflow bugs.
static void *reallocarray(void *optr, size_t nmemb, size_t size) {
  if ((nmemb >= MUL_NO_OVERFLOW || size >= MUL_NO_OVERFLOW) &&
      nmemb > 0 && SIZE_MAX / nmemb < size) {
    errno = ENOMEM;
    return NULL;
  }
  return realloc(optr, size * nmemb);
}

// TODO maybe we should use long here instead of int.
static inline int fast_floor(double x) {
  int i = (int)x;
  return i - (i > x);
}

static inline int fast_ceil(double x) {
  int i = (int)x;
  return i + (i < x);
}

static int map_file(SFT_Font *font, const char *filename) {
  struct stat info;
  int fd;
  font->memory = MAP_FAILED;
  font->size = 0;
  font->source = SrcMapping;
  if ((fd = open(filename, O_RDONLY)) < 0) {
    return -1;
  }
  if (fstat(fd, &info) < 0) {
    close(fd);
    return -1;
  }
  // FIXME do some basic validation on info.st_size maybe - it is signed for example, so it *could* be negative ..
  font->memory = mmap(NULL, (size_t)info.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
  font->size = (uint_fast32_t)info.st_size;
  close(fd);
  return font->memory == MAP_FAILED ? -1 : 0;
}

static void unmap_file(SFT_Font *font) {
  assert(font->memory != MAP_FAILED);
  munmap((void *)font->memory, font->size);
}

static int init_font(SFT_Font *font) {
  uint_fast32_t scalerType, head, hhea;

  if (!is_safe_offset(font, 0, 12)) return -1;
  // Check for a compatible scalerType (magic number).
  scalerType = getu32(font, 0);
  if (scalerType != FILE_MAGIC_ONE && scalerType != FILE_MAGIC_TWO) return -1;

  if (gettable(font, "head", &head) < 0) return -1;
  if (!is_safe_offset(font, head, 54)) return -1;
  font->unitsPerEm = getu16(font, head + 18);
  font->locaFormat = geti16(font, head + 50);

  if (gettable(font, "hhea", &hhea) < 0) return -1;
  if (!is_safe_offset(font, hhea, 36)) return -1;
  font->numLongHmtx = getu16(font, hhea + 34);

  return 0;
}

static SFT_Point midpoint(SFT_Point a, SFT_Point b) {
  return (SFT_Point){
    0.5 * (a.x + b.x),
    0.5 * (a.y + b.y)
  };
}

// Applies an affine linear transformation matrix to a set of points.
static void transform_points(unsigned int numPts, SFT_Point *points, double trf[6]) {
  SFT_Point pt;
  unsigned int i;
  for (i = 0; i < numPts; ++i) {
    pt = points[i];
    points[i] = (SFT_Point){
      pt.x * trf[0] + pt.y * trf[2] + trf[4],
      pt.x * trf[1] + pt.y * trf[3] + trf[5]
    };
  }
}

static void clip_points(unsigned int numPts, SFT_Point *points, int width, int height) {
  SFT_Point pt;
  unsigned int i;

  for (i = 0; i < numPts; ++i) {
    pt = points[i];

    if (pt.x < 0.0) {
      points[i].x = 0.0;
    }
    if (pt.x >= width) {
      points[i].x = nextafter(width, 0.0);
    }
    if (pt.y < 0.0) {
      points[i].y = 0.0;
    }
    if (pt.y >= height) {
      points[i].y = nextafter(height, 0.0);
    }
  }
}

static int init_outline(SFT_Outline *outl) {
  // TODO Smaller initial allocations
  outl->numPoints = 0;
  outl->capPoints = 64;
  if (!(outl->points = malloc(outl->capPoints * sizeof *outl->points))) return -1;
  outl->numCurves = 0;
  outl->capCurves = 64;
  if (!(outl->curves = malloc(outl->capCurves * sizeof *outl->curves))) return -1;
  outl->numLines = 0;
  outl->capLines = 64;
  if (!(outl->lines = malloc(outl->capLines * sizeof *outl->lines))) return -1;
  return 0;
}

static void free_outline(SFT_Outline *outl) {
  free(outl->points);
  free(outl->curves);
  free(outl->lines);
}

static int grow_points(SFT_Outline *outl) {
  void *mem;
  uint_fast16_t cap;
  assert(outl->capPoints);
  // Since we use uint_fast16_t for capacities, we have to be extra careful not to trigger integer overflow.
  if (outl->capPoints > UINT16_MAX / 2) return -1;
  cap = (uint_fast16_t)(2U * outl->capPoints);
  if (!(mem = reallocarray(outl->points, cap, sizeof *outl->points))) return -1;
  outl->capPoints = (uint_least16_t)cap;
  outl->points = mem;
  return 0;
}

static int grow_curves(SFT_Outline *outl) {
  void *mem;
  uint_fast16_t cap;
  assert(outl->capCurves);
  if (outl->capCurves > UINT16_MAX / 2) return -1;
  cap = (uint_fast16_t)(2U * outl->capCurves);
  if (!(mem = reallocarray(outl->curves, cap, sizeof *outl->curves))) return -1;
  outl->capCurves = (uint_least16_t)cap;
  outl->curves = mem;
  return 0;
}

static int grow_lines(SFT_Outline *outl) {
  void *mem;
  uint_fast16_t cap;
  assert(outl->capLines);
  if (outl->capLines > UINT16_MAX / 2) return -1;
  cap = (uint_fast16_t)(2U * outl->capLines);
  if (!(mem = reallocarray(outl->lines, cap, sizeof *outl->lines))) return -1;
  outl->capLines = (uint_least16_t)cap;
  outl->lines = mem;
  return 0;
}

static inline int is_safe_offset(SFT_Font *font, uint_fast32_t offset, uint_fast32_t margin) {
  if (offset > font->size) return 0;
  if (font->size - offset < margin) return 0;
  return 1;
}

// Like bsearch(), but returns the next highest element if key could not be found.
static void *csearch(const void *key, const void *base, size_t nmemb, size_t size, int (*compar)(const void *, const void *)) {
  const uint8_t *bytes = base, *sample;
  size_t low = 0, high = nmemb - 1, mid;
  if (!nmemb) return NULL;
  while (low != high) {
    mid = low + (high - low) / 2;
    sample = bytes + mid * size;
    if (compar(key, sample) > 0) {
      low = mid + 1;
    } else {
      high = mid;
    }
  }
  return (uint8_t *)bytes + low * size;
}

// Used as a comparison function for [bc]search().
static int cmpu16(const void *a, const void *b) {
  return memcmp(a, b, 2);
}

// Used as a comparison function for [bc]search().
static int cmpu32(const void *a, const void *b) {
  return memcmp(a, b, 4);
}

static inline uint_least8_t getu8(SFT_Font *font, uint_fast32_t offset) {
  assert(offset + 1 <= font->size);
  return *(font->memory + offset);
}

static inline int_least8_t geti8(SFT_Font *font, uint_fast32_t offset) {
  return (int_least8_t)getu8(font, offset);
}

static inline uint_least16_t getu16(SFT_Font *font, uint_fast32_t offset) {
  assert(offset + 2 <= font->size);
  const uint8_t *base = font->memory + offset;
  uint_least16_t b1 = base[0], b0 = base[1];
  return (uint_least16_t)(b1 << 8 | b0);
}

static inline int16_t geti16(SFT_Font *font, uint_fast32_t offset) {
  return (int_least16_t)getu16(font, offset);
}

static inline uint32_t getu32(SFT_Font *font, uint_fast32_t offset) {
  assert(offset + 4 <= font->size);
  const uint8_t *base = font->memory + offset;
  uint_least32_t b3 = base[0], b2 = base[1], b1 = base[2], b0 = base[3];
  return (uint_least32_t)(b3 << 24 | b2 << 16 | b1 << 8 | b0);
}

static int gettable(SFT_Font *font, char tag[4], uint_fast32_t *offset) {
  void *match;
  unsigned int numTables;
  // No need to bounds-check access to the first 12 bytes - this gets already checked by init_font().
  numTables = getu16(font, 4);
  if (!is_safe_offset(font, 12, (uint_fast32_t)numTables * 16)) return -1;
  if (!(match = bsearch(tag, font->memory + 12, numTables, 16, cmpu32))) return -1;
  *offset = getu32(font, (uint_fast32_t)((uint8_t *)match - font->memory + 8));
  return 0;
}

static int cmap_fmt4(SFT_Font *font, uint_fast32_t table, SFT_UChar charCode, SFT_Glyph *glyph) {
  const uint8_t *segPtr;
  uint_fast32_t segIdxX2;
  uint_fast32_t endCodes, startCodes, idDeltas, idRangeOffsets, idOffset;
  uint_fast16_t segCountX2, idRangeOffset, startCode, shortCode, idDelta, id;
  uint8_t key[2] = {(uint8_t)(charCode >> 8), (uint8_t)charCode};
  // cmap format 4 only supports the Unicode BMP.
  if (charCode > 0xFFFF) {
    *glyph = 0;
    return 0;
  }
  shortCode = (uint_fast16_t)charCode;
  if (!is_safe_offset(font, table, 8)) return -1;
  segCountX2 = getu16(font, table);
  if ((segCountX2 & 1) || !segCountX2) return -1;
  // Find starting positions of the relevant arrays.
  endCodes = table + 8;
  startCodes = endCodes + segCountX2 + 2;
  idDeltas = startCodes + segCountX2;
  idRangeOffsets = idDeltas + segCountX2;
  if (!is_safe_offset(font, idRangeOffsets, segCountX2)) return -1;
  // Find the segment that contains shortCode by binary searching over
  // the highest codes in the segments.
  segPtr = csearch(key, font->memory + endCodes, segCountX2 / 2, 2, cmpu16);
  segIdxX2 = (uint_fast32_t)(segPtr - (font->memory + endCodes));
  // Look up segment info from the arrays & short circuit if the spec requires.
  if ((startCode = getu16(font, startCodes + segIdxX2)) > shortCode) return 0;
  idDelta = getu16(font, idDeltas + segIdxX2);
  if (!(idRangeOffset = getu16(font, idRangeOffsets + segIdxX2))) {
    // Intentional integer under- and overflow.
    *glyph = (shortCode + idDelta) & 0xFFFF;
    return 0;
  }
  // Calculate offset into glyph array and determine ultimate value.
  idOffset = idRangeOffsets + segIdxX2 + idRangeOffset + 2U * (unsigned int)(shortCode - startCode);
  if (!is_safe_offset(font, idOffset, 2)) return -1;
  id = getu16(font, idOffset);
  // Intentional integer under- and overflow.
  *glyph = id ? (id + idDelta) & 0xFFFF : 0;
  return 0;
}

static int cmap_fmt6(SFT_Font *font, uint_fast32_t table, SFT_UChar charCode, SFT_Glyph *glyph) {
  unsigned int firstCode, entryCount;
  // cmap format 6 only supports the Unicode BMP.
  if (charCode > 0xFFFF) {
    *glyph = 0;
    return 0;
  }
  if (!is_safe_offset(font, table, 4)) return -1;
  firstCode = getu16(font, table);
  entryCount = getu16(font, table + 2);
  if (!is_safe_offset(font, table, 4 + 2 * entryCount)) return -1;
  if (charCode < firstCode) return -1;
  charCode -= firstCode;
  if (!(charCode < entryCount)) return -1;
  *glyph = getu16(font, table + 4 + 2 * charCode);
  return 0;
}

static int cmap_fmt12_13(SFT_Font *font, uint_fast32_t table, SFT_UChar charCode, SFT_Glyph *glyph, int which) {
  uint32_t len, numEntries;
  uint_fast32_t i;

  *glyph = 0;

  // check that the entire header is present
  if (!is_safe_offset(font, table, 16)) return -1;

  len = getu32(font, table + 4);

  // A minimal header is 16 bytes
  if (len < 16) return -1;

  if (!is_safe_offset(font, table, len)) return -1;

  numEntries = getu32(font, table + 12);

  for (i = 0; i < numEntries; ++i) {
    uint32_t firstCode, lastCode, glyphOffset;
    firstCode = getu32(font, table + (i * 12) + 16);
    lastCode = getu32(font, table + (i * 12) + 16 + 4);
    if (charCode < firstCode || charCode > lastCode) continue;
    glyphOffset = getu32(font, table + (i * 12) + 16 + 8);
    if (which == 12) {
      *glyph = (charCode - firstCode) + glyphOffset;
    } else {
      *glyph = glyphOffset;
    }
    return 0;
  }

  return 0;
}

// Maps Unicode code points to glyph indices.
static int glyph_id(SFT_Font *font, SFT_UChar charCode, SFT_Glyph *glyph) {
  uint_fast32_t cmap, entry, table;
  unsigned int idx, numEntries;
  int type, format;

  *glyph = 0;

  if (gettable(font, "cmap", &cmap) < 0) return -1;
  if (!is_safe_offset(font, cmap, 4)) return -1;
  numEntries = getu16(font, cmap + 2);

  if (!is_safe_offset(font, cmap, 4 + numEntries * 8)) return -1;

  // First look for a 'full repertoire'/non-BMP map.
  for (idx = 0; idx < numEntries; ++idx) {
    entry = cmap + 4 + idx * 8;
    type = getu16(font, entry) * 0100 + getu16(font, entry + 2);
    // Complete unicode map
    if (type == 0004 || type == 0312) {
      table = cmap + getu32(font, entry + 4);
      if (!is_safe_offset(font, table, 8)) return -1;
      // Dispatch based on cmap format.
      format = getu16(font, table);
      switch (format) {
        case 12:
          return cmap_fmt12_13(font, table, charCode, glyph, 12);
        default:
          return -1;
      }
    }
  }

  // If no 'full repertoire' cmap was found, try looking for a BMP map.
  for (idx = 0; idx < numEntries; ++idx) {
    entry = cmap + 4 + idx * 8;
    type = getu16(font, entry) * 0100 + getu16(font, entry + 2);
    // Unicode BMP
    if (type == 0003 || type == 0301) {
      table = cmap + getu32(font, entry + 4);
      if (!is_safe_offset(font, table, 6)) return -1;
      // Dispatch based on cmap format.
      switch (getu16(font, table)) {
        case 4:
          return cmap_fmt4(font, table + 6, charCode, glyph);
        case 6:
          return cmap_fmt6(font, table + 6, charCode, glyph);
        default:
          return -1;
      }
    }
  }
  return -1;
}

static int hor_metrics(SFT_Font *font, SFT_Glyph glyph, int *advanceWidth, int *leftSideBearing) {
  uint_fast32_t hmtx, offset, boundary;
  if (gettable(font, "hmtx", &hmtx) < 0) return -1;
  if (glyph < font->numLongHmtx) {
    // glyph is inside long metrics segment.
    offset = hmtx + 4 * glyph;
    if (!is_safe_offset(font, offset, 4))
      return -1;
    *advanceWidth = getu16(font, offset);
    *leftSideBearing = geti16(font, offset + 2);
    return 0;
  } else {
    // glyph is inside short metrics segment.
    boundary = hmtx + 4U * (uint_fast32_t)font->numLongHmtx;
    if (boundary < 4) return -1;

    offset = boundary - 4;
    if (!is_safe_offset(font, offset, 4)) return -1;
    *advanceWidth = getu16(font, offset);

    offset = boundary + 2 * (glyph - font->numLongHmtx);
    if (!is_safe_offset(font, offset, 2)) return -1;
    *leftSideBearing = geti16(font, offset);
    return 0;
  }
}

static int glyph_bbox(const SFT *sft, uint_fast32_t outline, int box[4]) {
  double xScale, yScale;
  // Read the bounding box from the font file verbatim.
  if (!is_safe_offset(sft->font, outline, 10)) return -1;
  box[0] = geti16(sft->font, outline + 2);
  box[1] = geti16(sft->font, outline + 4);
  box[2] = geti16(sft->font, outline + 6);
  box[3] = geti16(sft->font, outline + 8);
  if (box[2] <= box[0] || box[3] <= box[1]) return -1;
  // Transform the bounding box into SFT coordinate space.
  xScale = sft->xScale / sft->font->unitsPerEm;
  yScale = sft->yScale / sft->font->unitsPerEm;
  box[0] = (int)floor(box[0] * xScale);
  box[1] = (int)floor(box[1] * yScale);
  box[2] = (int)ceil(box[2] * xScale);
  box[3] = (int)ceil(box[3] * yScale);
  return 0;
}

// Returns the offset into the font that the glyph's outline is stored at.
static int outline_offset(SFT_Font *font, SFT_Glyph glyph, uint_fast32_t *offset) {
  uint_fast32_t loca, glyf;
  uint_fast32_t base, this, next;

  if (gettable(font, "loca", &loca) < 0) return -1;
  if (gettable(font, "glyf", &glyf) < 0) return -1;

  if (font->locaFormat == 0) {
    base = loca + 2 * glyph;

    if (!is_safe_offset(font, base, 4))
      return -1;

    this = 2U * (uint_fast32_t)getu16(font, base);
    next = 2U * (uint_fast32_t)getu16(font, base + 2);
  } else {
    base = loca + 4 * glyph;

    if (!is_safe_offset(font, base, 8))
      return -1;

    this = getu32(font, base);
    next = getu32(font, base + 4);
  }

  *offset = this == next ? 0 : glyf + this;
  return 0;
}

// For a 'simple' outline, determines each point of the outline with a set of flags.
static int simple_flags(SFT_Font *font, uint_fast32_t *offset, uint_fast16_t numPts, uint8_t *flags) {
  uint_fast32_t off = *offset;
  uint_fast16_t i;
  uint8_t value = 0, repeat = 0;
  for (i = 0; i < numPts; ++i) {
    if (repeat) {
      --repeat;
    } else {
      if (!is_safe_offset(font, off, 1))
        return -1;
      value = getu8(font, off++);
      if (value & REPEAT_FLAG) {
        if (!is_safe_offset(font, off, 1))
          return -1;
        repeat = getu8(font, off++);
      }
    }
    flags[i] = value;
  }
  *offset = off;
  return 0;
}

// For a 'simple' outline, decodes both X and Y coordinates for each point of the outline.
static int simple_points(SFT_Font *font, uint_fast32_t offset, uint_fast16_t numPts, uint8_t *flags, SFT_Point *points) {
  long accum, value, bit;
  uint_fast16_t i;

  accum = 0L;
  for (i = 0; i < numPts; ++i) {
    if (flags[i] & X_CHANGE_IS_SMALL) {
      if (!is_safe_offset(font, offset, 1))
        return -1;
      value = (long)getu8(font, offset++);
      bit = !!(flags[i] & X_CHANGE_IS_POSITIVE);
      accum -= (value ^ -bit) + bit;
    } else if (!(flags[i] & X_CHANGE_IS_ZERO)) {
      if (!is_safe_offset(font, offset, 2))
        return -1;
      accum += geti16(font, offset);
      offset += 2;
    }
    points[i].x = (double)accum;
  }

  accum = 0L;
  for (i = 0; i < numPts; ++i) {
    if (flags[i] & Y_CHANGE_IS_SMALL) {
      if (!is_safe_offset(font, offset, 1))
        return -1;
      value = (long)getu8(font, offset++);
      bit = !!(flags[i] & Y_CHANGE_IS_POSITIVE);
      accum -= (value ^ -bit) + bit;
    } else if (!(flags[i] & Y_CHANGE_IS_ZERO)) {
      if (!is_safe_offset(font, offset, 2))
        return -1;
      accum += geti16(font, offset);
      offset += 2;
    }
    points[i].y = (double)accum;
  }

  return 0;
}

static int decode_contour(uint8_t *flags, uint_fast16_t basePoint, uint_fast16_t count, SFT_Outline *outl) {
  uint_fast16_t i;
  uint_least16_t looseEnd, beg, ctrl, center, cur;
  unsigned int gotCtrl;

  // Skip contours with less than two points, since the following algorithm can't handle them and they should appear invisible either way (because they don't have any area).
  if (count < 2) return 0;

  assert(basePoint <= UINT16_MAX - count);

  if (flags[0] & POINT_IS_ON_CURVE) {
    looseEnd = (uint_least16_t)basePoint++;
    ++flags;
    --count;
  } else if (flags[count - 1] & POINT_IS_ON_CURVE) {
    looseEnd = (uint_least16_t)(basePoint + --count);
  } else {
    if (outl->numPoints >= outl->capPoints && grow_points(outl) < 0)
      return -1;

    looseEnd = outl->numPoints;
    outl->points[outl->numPoints++] = midpoint(
      outl->points[basePoint],
      outl->points[basePoint + count - 1]
    );
  }
  beg = looseEnd;
  gotCtrl = 0;
  for (i = 0; i < count; ++i) {
    // cur can't overflow because we ensure that basePoint + count < 0xFFFF before calling decode_contour().
    cur = (uint_least16_t)(basePoint + i);
    // NOTE clang-analyzer will often flag this and another piece of code because it thinks that flags and outl->points + basePoint don't always get properly initialized -- even when you explicitly loop over both and set every element to zero (but not when you use memset). This is a known clang-analyzer bug: http://clang-developers.42468.n3.nabble.com/StaticAnalyzer-False-positive-with-loop-handling-td4053875.html
    if (flags[i] & POINT_IS_ON_CURVE) {
      if (gotCtrl) {
        if (outl->numCurves >= outl->capCurves && grow_curves(outl) < 0)
          return -1;
        outl->curves[outl->numCurves++] = (SFT_Curve){beg, cur, ctrl};
      } else {
        if (outl->numLines >= outl->capLines && grow_lines(outl) < 0)
          return -1;
        outl->lines[outl->numLines++] = (SFT_Line){beg, cur};
      }
      beg = cur;
      gotCtrl = 0;
    } else {
      if (gotCtrl) {
        center = outl->numPoints;
        if (outl->numPoints >= outl->capPoints && grow_points(outl) < 0)
          return -1;
        outl->points[center] = midpoint(outl->points[ctrl], outl->points[cur]);
        ++outl->numPoints;

        if (outl->numCurves >= outl->capCurves && grow_curves(outl) < 0)
          return -1;
        outl->curves[outl->numCurves++] = (SFT_Curve){beg, center, ctrl};

        beg = center;
      }
      ctrl = cur;
      gotCtrl = 1;
    }
  }
  if (gotCtrl) {
    if (outl->numCurves >= outl->capCurves && grow_curves(outl) < 0)
      return -1;
    outl->curves[outl->numCurves++] = (SFT_Curve){beg, looseEnd, ctrl};
  } else {
    if (outl->numLines >= outl->capLines && grow_lines(outl) < 0)
      return -1;
    outl->lines[outl->numLines++] = (SFT_Line){beg, looseEnd};
  }
  return 0;
}

static int simple_outline(SFT_Font *font, uint_fast32_t offset, unsigned int numContours, SFT_Outline *outl) {
  uint_fast16_t *endPts = NULL;
  uint8_t *flags = NULL;
  uint_fast16_t numPts;
  unsigned int i;

  assert(numContours > 0);

  uint_fast16_t basePoint = outl->numPoints;

  if (!is_safe_offset(font, offset, numContours * 2 + 2))
    goto failure;
  numPts = getu16(font, offset + (numContours - 1) * 2);
  if (numPts >= UINT16_MAX)
    goto failure;
  numPts++;
  if (outl->numPoints > UINT16_MAX - numPts)
    goto failure;

  while (outl->capPoints < basePoint + numPts) {
    if (grow_points(outl) < 0)
      goto failure;
  }

  STACK_ALLOC(endPts, uint_fast16_t, 16, numContours);
  if (endPts == NULL)
    goto failure;
  STACK_ALLOC(flags, uint8_t, 128, numPts);
  if (flags == NULL)
    goto failure;

  for (i = 0; i < numContours; ++i) {
    endPts[i] = getu16(font, offset);
    offset += 2;
  }
  // Ensure that endPts are never falling. Falling endPts have no sensible interpretation and most likely only occur in malicious input. Therefore, we bail, should we ever encounter such input.
  for (i = 0; i < numContours - 1; ++i) {
    if (endPts[i + 1] < endPts[i] + 1)
      goto failure;
  }
  offset += 2U + getu16(font, offset);

  if (simple_flags(font, &offset, numPts, flags) < 0)
    goto failure;
  if (simple_points(font, offset, numPts, flags, outl->points + basePoint) < 0)
    goto failure;
  outl->numPoints = (uint_least16_t)(outl->numPoints + numPts);

  uint_fast16_t beg = 0;
  for (i = 0; i < numContours; ++i) {
    uint_fast16_t count = endPts[i] - beg + 1;
    if (decode_contour(flags + beg, basePoint + beg, count, outl) < 0)
      goto failure;
    beg = endPts[i] + 1;
  }

  STACK_FREE(endPts);
  STACK_FREE(flags);
  return 0;
failure:
  STACK_FREE(endPts);
  STACK_FREE(flags);
  return -1;
}

static int compound_outline(SFT_Font *font, uint_fast32_t offset, int recDepth, SFT_Outline *outl) {
  double local[6];
  uint_fast32_t outline;
  unsigned int flags, glyph, basePoint;
  // Guard against infinite recursion (compound glyphs that have themselves as component).
  if (recDepth >= 4) return -1;
  do {
    memset(local, 0, sizeof local);
    if (!is_safe_offset(font, offset, 4)) return -1;
    flags = getu16(font, offset);
    glyph = getu16(font, offset + 2);
    offset += 4;
    // We don't implement point matching, and neither does stb_truetype for that matter.
    if (!(flags & ACTUAL_XY_OFFSETS)) return -1;
    // Read additional X and Y offsets (in FUnits) of this component.
    if (flags & OFFSETS_ARE_LARGE) {
      if (!is_safe_offset(font, offset, 4)) return -1;
      local[4] = geti16(font, offset);
      local[5] = geti16(font, offset + 2);
      offset += 4;
    } else {
      if (!is_safe_offset(font, offset, 2)) return -1;
      local[4] = geti8(font, offset);
      local[5] = geti8(font, offset + 1);
      offset += 2;
    }
    if (flags & GOT_A_SINGLE_SCALE) {
      if (!is_safe_offset(font, offset, 2)) return -1;
      local[0] = geti16(font, offset) / 16384.0;
      local[3] = local[0];
      offset += 2;
    } else if (flags & GOT_AN_X_AND_Y_SCALE) {
      if (!is_safe_offset(font, offset, 4)) return -1;
      local[0] = geti16(font, offset + 0) / 16384.0;
      local[3] = geti16(font, offset + 2) / 16384.0;
      offset += 4;
    } else if (flags & GOT_A_SCALE_MATRIX) {
      if (!is_safe_offset(font, offset, 8)) return -1;
      local[0] = geti16(font, offset + 0) / 16384.0;
      local[1] = geti16(font, offset + 2) / 16384.0;
      local[2] = geti16(font, offset + 4) / 16384.0;
      local[3] = geti16(font, offset + 6) / 16384.0;
      offset += 8;
    } else {
      local[0] = 1.0;
      local[3] = 1.0;
    }
    // At this point, Apple's spec more or less tells you to scale the matrix by its own L1 norm. But stb_truetype scales by the L2 norm. And FreeType2 doesn't scale at all. Furthermore, Microsoft's spec doesn't even mention anything like this. It's almost as if nobody ever uses this feature anyway.
    if (outline_offset(font, glyph, &outline) < 0)
      return -1;
    if (outline) {
      basePoint = outl->numPoints;
      if (decode_outline(font, outline, recDepth + 1, outl) < 0)
        return -1;
      transform_points(outl->numPoints - basePoint, outl->points + basePoint, local);
    }
  } while (flags & THERE_ARE_MORE_COMPONENTS);

  return 0;
}

static int decode_outline(SFT_Font *font, uint_fast32_t offset, int recDepth, SFT_Outline *outl) {
  int numContours;
  if (!is_safe_offset(font, offset, 10)) return -1;
  numContours = geti16(font, offset);
  if (numContours > 0) {
    // Glyph has a 'simple' outline consisting of a number of contours.
    return simple_outline(font, offset + 10, (unsigned int)numContours, outl);
  } else if (numContours < 0) {
    // Glyph has a compound outline combined from mutiple other outlines.
    return compound_outline(font, offset + 10, recDepth, outl);
  } else {
    return 0;
  }
}

// A heuristic to tell whether a given curve can be approximated closely enough by a line.
static int is_flat(SFT_Outline *outl, SFT_Curve curve) {
  const double maxArea2 = 2.0;
  SFT_Point a = outl->points[curve.beg];
  SFT_Point b = outl->points[curve.ctrl];
  SFT_Point c = outl->points[curve.end];
  SFT_Point g = {b.x - a.x, b.y - a.y};
  SFT_Point h = {c.x - a.x, c.y - a.y};
  double area2 = fabs(g.x * h.y - h.x * g.y);
  return area2 <= maxArea2;
}

static int tesselate_curve(SFT_Curve curve, SFT_Outline *outl) {
  // From my tests I can conclude that this stack barely reaches a top height of 4 elements even for the largest font sizes I'm willing to support. And as space requirements should only grow logarithmically, I think 10 is more than enough.
#define STACK_SIZE 10
  SFT_Curve stack[STACK_SIZE];
  unsigned int top = 0;
  for (;;) {
    if (is_flat(outl, curve) || top >= STACK_SIZE) {
      if (outl->numLines >= outl->capLines && grow_lines(outl) < 0)
        return -1;
      outl->lines[outl->numLines++] = (SFT_Line){curve.beg, curve.end};
      if (top == 0) break;
      curve = stack[--top];
    } else {
      uint_least16_t ctrl0 = outl->numPoints;
      if (outl->numPoints >= outl->capPoints && grow_points(outl) < 0)
        return -1;
      outl->points[ctrl0] = midpoint(outl->points[curve.beg], outl->points[curve.ctrl]);
      ++outl->numPoints;

      uint_least16_t ctrl1 = outl->numPoints;
      if (outl->numPoints >= outl->capPoints && grow_points(outl) < 0)
        return -1;
      outl->points[ctrl1] = midpoint(outl->points[curve.ctrl], outl->points[curve.end]);
      ++outl->numPoints;

      uint_least16_t pivot = outl->numPoints;
      if (outl->numPoints >= outl->capPoints && grow_points(outl) < 0)
        return -1;
      outl->points[pivot] = midpoint(outl->points[ctrl0], outl->points[ctrl1]);
      ++outl->numPoints;

      stack[top++] = (SFT_Curve){curve.beg, pivot, ctrl0};
      curve = (SFT_Curve){pivot, curve.end, ctrl1};
    }
  }
  return 0;
#undef STACK_SIZE
}

static int tesselate_curves(SFT_Outline *outl) {
  unsigned int i;
  for (i = 0; i < outl->numCurves; ++i) {
    if (tesselate_curve(outl->curves[i], outl) < 0)
      return -1;
  }
  return 0;
}

// Draws a line into the buffer. Uses a custom 2D raycasting algorithm to do so.
static void draw_line(SFT_Raster buf, SFT_Point origin, SFT_Point goal) {
  SFT_Point delta;
  SFT_Point nextCrossing;
  SFT_Point crossingIncr;
  double halfDeltaX;
  double prevDistance = 0.0, nextDistance;
  double xAverage, yDifference;
  struct {
    int x, y;
  } pixel;
  struct {
    int x, y;
  } dir;
  int step, numSteps = 0;
  SFT_Cell *restrict cptr, cell;

  delta.x = goal.x - origin.x;
  delta.y = goal.y - origin.y;
  dir.x = SIGN(delta.x);
  dir.y = SIGN(delta.y);

  if (!dir.y) {
    return;
  }

  crossingIncr.x = dir.x ? fabs(1.0 / delta.x) : 1.0;
  crossingIncr.y = fabs(1.0 / delta.y);

  if (!dir.x) {
    pixel.x = fast_floor(origin.x);
    nextCrossing.x = 100.0;
  } else {
    if (dir.x > 0) {
      pixel.x = fast_floor(origin.x);
      nextCrossing.x = (origin.x - pixel.x) * crossingIncr.x;
      nextCrossing.x = crossingIncr.x - nextCrossing.x;
      numSteps += fast_ceil(goal.x) - fast_floor(origin.x) - 1;
    } else {
      pixel.x = fast_ceil(origin.x) - 1;
      nextCrossing.x = (origin.x - pixel.x) * crossingIncr.x;
      numSteps += fast_ceil(origin.x) - fast_floor(goal.x) - 1;
    }
  }

  if (dir.y > 0) {
    pixel.y = fast_floor(origin.y);
    nextCrossing.y = (origin.y - pixel.y) * crossingIncr.y;
    nextCrossing.y = crossingIncr.y - nextCrossing.y;
    numSteps += fast_ceil(goal.y) - fast_floor(origin.y) - 1;
  } else {
    pixel.y = fast_ceil(origin.y) - 1;
    nextCrossing.y = (origin.y - pixel.y) * crossingIncr.y;
    numSteps += fast_ceil(origin.y) - fast_floor(goal.y) - 1;
  }

  nextDistance = MIN(nextCrossing.x, nextCrossing.y);
  halfDeltaX = 0.5 * delta.x;

  for (step = 0; step < numSteps; ++step) {
    xAverage = origin.x + (prevDistance + nextDistance) * halfDeltaX;
    yDifference = (nextDistance - prevDistance) * delta.y;
    cptr = &buf.cells[pixel.y * buf.width + pixel.x];
    cell = *cptr;
    cell.cover += yDifference;
    xAverage -= (double)pixel.x;
    cell.area += (1.0 - xAverage) * yDifference;
    *cptr = cell;
    prevDistance = nextDistance;
    int alongX = nextCrossing.x < nextCrossing.y;
    pixel.x += alongX ? dir.x : 0;
    pixel.y += alongX ? 0 : dir.y;
    nextCrossing.x += alongX ? crossingIncr.x : 0.0;
    nextCrossing.y += alongX ? 0.0 : crossingIncr.y;
    nextDistance = MIN(nextCrossing.x, nextCrossing.y);
  }

  xAverage = origin.x + (prevDistance + 1.0) * halfDeltaX;
  yDifference = (1.0 - prevDistance) * delta.y;
  cptr = &buf.cells[pixel.y * buf.width + pixel.x];
  cell = *cptr;
  cell.cover += yDifference;
  xAverage -= (double)pixel.x;
  cell.area += (1.0 - xAverage) * yDifference;
  *cptr = cell;
}

static void draw_lines(SFT_Outline *outl, SFT_Raster buf) {
  unsigned int i;
  for (i = 0; i < outl->numLines; ++i) {
    SFT_Line line = outl->lines[i];
    SFT_Point origin = outl->points[line.beg];
    SFT_Point goal = outl->points[line.end];
    draw_line(buf, origin, goal);
  }
}

// Integrate the values in the buffer to arrive at the final grayscale image.
static void post_process(SFT_Raster buf, uint8_t *image) {
  SFT_Cell cell;
  double accum = 0.0, value;
  unsigned int i, num;
  num = (unsigned int)buf.width * (unsigned int)buf.height;
  for (i = 0; i < num; ++i) {
    cell = buf.cells[i];
    value = fabs(accum + cell.area);
    value = MIN(value, 1.0);
    value = value * 255.0 + 0.5;
    image[i] = (uint8_t)value;
    accum += cell.cover;
  }
}

static int render_outline(SFT_Outline *outl, double transform[6], SFT_Image image) {
  SFT_Cell *cells = NULL;
  SFT_Raster buf;
  unsigned int numPixels;

  numPixels = (unsigned int)image.width * (unsigned int)image.height;

  STACK_ALLOC(cells, SFT_Cell, 128 * 128, numPixels);
  if (!cells) {
    return -1;
  }
  memset(cells, 0, numPixels * sizeof *cells);
  buf.cells = cells;
  buf.width = image.width;
  buf.height = image.height;

  transform_points(outl->numPoints, outl->points, transform);

  clip_points(outl->numPoints, outl->points, image.width, image.height);

  if (tesselate_curves(outl) < 0) {
    STACK_FREE(cells);
    return -1;
  }

  draw_lines(outl, buf);

  post_process(buf, image.pixels);

  STACK_FREE(cells);
  return 0;
}

// Takes a UTF-8 string (uint8 array) and returns the first full UTF-8 character and assigns it to a uint32. Returns the number of bytes converted.
static int utf8_to_utf32(const uint8_t *utf8, uint32_t *utf32) {
  uint8_t c = utf8[0];
  if (c < 0x80) {
    *utf32 = c;
    return 1;
  }
  if ((c & 0xE0) == 0xC0) {
    *utf32 = (c & 0x1F) << 6 | (utf8[1] & 0x3F);
    return 2;
  }
  if ((c & 0xF0) == 0xE0) {
    *utf32 = (c & 0x0F) << 12 | (utf8[1] & 0x3F) << 6 | (utf8[2] & 0x3F);
    return 3;
  }
  if ((c & 0xF8) == 0xF0) {
    *utf32 = (c & 0x07) << 18 | (utf8[1] & 0x3F) << 12 | (utf8[2] & 0x3F) << 6 | (utf8[3] & 0x3F);
    return 4;
  }
  return 0;
}

// Steps through a UTF-8 string and measures the width of each character and the kerning between them to determine how many characters fit into a given width. Set measure_width to 0 to measure the entire string.
int SFT_MeasureUTF8(SFT *sft, uint8_t *text, int measure_width, int *extent, int *count) {
  SFT_UChar charCode;
  SFT_Glyph glyph;
  SFT_Glyph lastGlyph = 0;
  SFT_Kerning kerning;
  SFT_GMetrics metrics;
  double width = 0;
  int i = 0;
  int j = 0;
  // Loop through the string until null terminator
  while (text[i] != 0) {
    // i is incremented by the number of bytes in the UTF-8 character
    i += utf8_to_utf32(&text[i], &charCode);
    if (glyph_id(sft->font, charCode, &glyph) < 0) {
      printf("glyph_id failed\n");
      return -1;
    }
    if (sft_gmetrics(sft, glyph, &metrics) < 0) {
      printf("sft_metrics failed\n");
      return -1;
    }
    if (lastGlyph != 0) {
      if (sft_kerning(sft, lastGlyph, glyph, &kerning) < 0) return -1;
    }
    if (measure_width == 0 || width + kerning.xShift + metrics.advanceWidth < measure_width) {
      width += kerning.xShift + metrics.advanceWidth;
    } else {
      break;
    }
    j++;
    lastGlyph = glyph;
  }
  if (extent) {
    *extent = fast_ceil(width);
  }
  if (count) {
    *count = j;
  }
  return 0;
}

int SFT_text_width(SFT *sft, uint8_t *text, int *width_ref) {
  int width;
  if (SFT_MeasureUTF8(sft, text, 0, &width, 0) < 0) return -1;
  if (width_ref) {
    *width_ref = width;
  }
  return 0;
}

// Renders a UTF-8 string into an image using sft_render for each character.
int SFT_RenderUTF8(SFT *sft, uint8_t *text, SFT_Image image) {
  int i = 0;
  SFT_UChar charCode = 0;
  SFT_Glyph glyph = 0;
  SFT_GMetrics metrics;
  SFT_Glyph lastGlyph = 0;
  SFT_Kerning kerning;
  // Character start position from the left
  double charStart = 0;
  // Glyph start position from the left
  int glyph_start = 0;
  double rounding_error = 0;
  SFT_Image charImage;
  uint8_t *target_pixels = image.pixels;
  SFT_LMetrics lmetrics;
  sft_lmetrics(sft, &lmetrics);

  // Loop over the string
  while (text[i] != 0) {
    // Get the UTF-32 character code
    i += utf8_to_utf32(&text[i], &charCode);
    // Get the glyph for the character
    if (glyph_id(sft->font, charCode, &glyph) < 0) return -1;
    if (sft_gmetrics(sft, glyph, &metrics) < 0) return -1;
    // Allocate memory for the character image
    uint32_t num_pixels = (metrics.minWidth) * metrics.minHeight;
    uint8_t *char_pixels = (uint8_t *)malloc(num_pixels * sizeof(uint8_t));
    // Fill the pixels with 0
    memset(char_pixels, 0, num_pixels);
    charImage = (SFT_Image){
      .width = metrics.minWidth,
      .height = metrics.minHeight,
      .pixels = char_pixels
    };

    if (lastGlyph != 0) {
      if (sft_kerning(sft, lastGlyph, glyph, &kerning) < 0) return -1;
      charStart += kerning.xShift;
    }
    glyph_start = fast_floor(charStart + metrics.leftSideBearing);
    // Rounding error for subpixel placement
    rounding_error = (charStart + metrics.leftSideBearing) - glyph_start;
    sft->xOffset = rounding_error;
    sft_render(sft, glyph, charImage);

    // Copy the character pixels to the image
    for (int y = 0; y < charImage.height; y++) {
      int target_y = y + lmetrics.ascender + metrics.yOffset;
      if (target_y < image.height) {
        for (int x = 0; x < charImage.width; x++) {
          int target_x = glyph_start + x;
          if (target_x < image.width) {
            uint8_t *target_pixel = &target_pixels[(target_y * image.width) + target_x];
            if (x < charImage.width) {
              uint8_t pixel = char_pixels[y * charImage.width + x];
              // If glyphs overlap, take the maximum value
              *target_pixel = *target_pixel > pixel ? *target_pixel : pixel;
            }
          }
        }
      }
    }
    // Free the image pixels
    free(char_pixels);
    charStart += metrics.advanceWidth;
    lastGlyph = glyph;
  }
  return 0;
}

#define SCHRIFT_TTF
#endif