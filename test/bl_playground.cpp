#include <blend2d.h>

#include "bl_test_context_utilities.h"

static BLImage generateTexture() noexcept {
  int size = 55;
  BLFormat format = BL_FORMAT_PRGB32;

  BLImage texture;
  texture.create(size, size, format);

  // Disable JIT here as we may be testing it in the future.
  // If there is a bug in JIT we want to find it by tests,
  // and not face it here...
  BLContextCreateInfo cci{};
  cci.flags = BL_CONTEXT_CREATE_FLAG_DISABLE_JIT;
  BLContext ctx(texture, cci);
  ctx.clearAll();

  double s = double(size);
  double half = s * 0.5;

  ctx.fillCircle(half, half, half * 1.00, BLRgba32(0xFFFFFFFF));
  ctx.setCompOp(BL_COMP_OP_SRC_COPY);
  ctx.fillCircle(half + 10, half, half * 0.66, BLRgba32(0x4FFF0000));
  ctx.fillCircle(half, half, half * 0.33, BLRgba32(0xFF0000FF));

  return texture;
}

int main(int argc, char* argv[]) {
  BLImage texture = generateTexture();
  BLImage img(513, 513, BL_FORMAT_PRGB32);

  BLContextCreateInfo cci{};
  BLContext ctx(img, cci);

  ctx.fillAll(BLRgba32(0xFF0000FF));

  BLPattern p(texture);
  p.setExtendMode(BL_EXTEND_MODE_PAD);
  p.rotate(0.22);
  p.translate(100, 190);
  // p.translate(0.5, 0.0);

  ctx.setCompOp(BL_COMP_OP_SRC_OVER);
  ctx.setPatternQuality(BL_PATTERN_QUALITY_NEAREST);
  ctx.fillRect(BLRectI(60, 177, 248, 113), p);

  ctx.end();
  img.writeToFile("bl_playground_output.png");
  return 0;
}
