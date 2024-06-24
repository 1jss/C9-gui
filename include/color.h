#include "types.h" // u8

typedef struct {
  u8 r;
  u8 g;
  u8 b;
} C9_RGB;

typedef struct {
  C9_RGB start;
  C9_RGB end;
} C9_Gradient;

// getGradientColor returns a color between the start and end colors of a gradient. The t parameter is a value between 0 and 1.
C9_RGB getGradientColor(C9_Gradient gradient, f32 t) {
  return (C9_RGB){
    .r = gradient.start.r + (gradient.end.r - gradient.start.r) * t,
    .g = gradient.start.g + (gradient.end.g - gradient.start.g) * t,
    .b = gradient.start.b + (gradient.end.b - gradient.start.b) * t
  };
}