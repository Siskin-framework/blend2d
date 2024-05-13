// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

// This file provides utility classes and functions shared between some tests.

#ifndef BLEND2D_TEST_FUZZ_UTILITIES_H_INCLUDED
#define BLEND2D_TEST_FUZZ_UTILITIES_H_INCLUDED

#include <blend2d.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

class CmdLine {
public:
  int _argc;
  const char* const* _argv;

  CmdLine(int argc, const char* const* argv)
    : _argc(argc),
      _argv(argv) {}

  bool hasArg(const char* key) const {
    for (int i = 1; i < _argc; i++)
      if (strcmp(key, _argv[i]) == 0)
        return true;
    return false;
  }

  const char* valueOf(const char* key, const char* defaultValue) const {
    size_t keySize = strlen(key);
    for (int i = 1; i < _argc; i++) {
      const char* val = _argv[i];
      if (strlen(val) >= keySize + 1 && val[keySize] == '=' && memcmp(val, key, keySize) == 0)
        return val + keySize + 1;
    }

    return defaultValue;
  }

  int valueAsInt(const char* key, int defaultValue) const {
    const char* val = valueOf(key, nullptr);
    if (val == nullptr || val[0] == '\0')
      return defaultValue;

    return atoi(val);
  }

  unsigned valueAsUInt(const char* key, unsigned defaultValue) const {
    const char* val = valueOf(key, nullptr);
    if (val == nullptr || val[0] == '\0')
      return defaultValue;

    int v = atoi(val);
    if (v < 0)
      return defaultValue;
    else
      return unsigned(v);
  }
};

class Logger {
public:
  enum class Verbosity {
    Debug,
    Info,
    Silent
  };

  Verbosity _verbosity;

  inline Logger(Verbosity verbosity)
    : _verbosity(verbosity) {}

  inline Verbosity verbosity() const { return _verbosity; }

  inline Verbosity setVerbosity(Verbosity value) {
    Verbosity prev = _verbosity;
    _verbosity = value;
    return prev;
  }

  inline void print(const char* fmt) {
    puts(fmt);
    fflush(stdout);
  }

  template<typename... Args>
  inline void print(const char* fmt, Args&&... args) {
    printf(fmt, std::forward<Args>(args)...);
    fflush(stdout);
  }

  template<typename... Args>
  inline void debug(const char* fmt, Args&&... args) {
    if (_verbosity <= Verbosity::Debug)
      print(fmt, std::forward<Args>(args)...);
  }

  template<typename... Args>
  inline void info(const char* fmt, Args&&... args) {
    if (_verbosity <= Verbosity::Info)
      print(fmt, std::forward<Args>(args)...);
  }
};

class RandomDataGenerator {
public:
  enum class Mode {
    InBounds = 0
  };

  BLRandom _rnd;
  Mode _mode;
  BLBox _bounds;
  BLSize _size;

  RandomDataGenerator()
    : _rnd(0x12345678),
      _mode(Mode::InBounds),
      _bounds(),
      _size() {}

  inline Mode mode() const { return _mode; }
  inline void setMode(Mode mode) { _mode = mode; }

  inline const BLBox& bounds() const { return _bounds; }
  inline void setBounds(const BLBox& bounds) {
    _bounds = bounds;
    _size.reset(_bounds.x1 - _bounds.x0, _bounds.y1 - _bounds.y0);
  }

  inline void seed(uint64_t value) { _rnd.reset(value); }

  inline uint32_t nextUInt32() { return _rnd.nextUInt32(); }
  inline uint64_t nextUInt64() { return _rnd.nextUInt64(); }
  inline double nextDouble() { return _rnd.nextDouble(); }

  inline BLRgba32 nextRgb32() { return BLRgba32(_rnd.nextUInt32() | 0xFF000000u); }
  inline BLRgba32 nextRgba32() { return BLRgba32(_rnd.nextUInt32()); }

  inline BLExtendMode nextGradientExtend() { return BLExtendMode(_rnd.nextUInt32() % (BL_EXTEND_MODE_SIMPLE_MAX_VALUE + 1u)); }
  inline BLExtendMode nextPatternExtend() { return BLExtendMode(_rnd.nextUInt32() % (BL_EXTEND_MODE_MAX_VALUE + 1u)); }

  inline int nextXCoordI() { return (int)((_rnd.nextDouble() * _size.w) + _bounds.x0); }
  inline int nextYCoordI() { return (int)((_rnd.nextDouble() * _size.h) + _bounds.y0); };

  inline double nextXCoordD() { return (_rnd.nextDouble() * _size.w) + _bounds.x0; }
  inline double nextYCoordD() { return (_rnd.nextDouble() * _size.h) + _bounds.y0; };

  inline BLPoint nextPointD() { return BLPoint(nextXCoordD(), nextYCoordD()); }
  inline BLPoint nextPointI() { return BLPointI(nextXCoordI(), nextYCoordI()); }

  inline BLBox nextBoxD() {
    double x0 = nextXCoordD();
    double y0 = nextYCoordD();
    double x1 = nextXCoordD();
    double y1 = nextYCoordD();
    return BLBox(blMin(x0, x1), blMin(y0, y1), blMax(x0, x1), blMax(y0, y1));
  }

  inline BLBoxI nextBoxI() {
    int x0 = nextXCoordI();
    int y0 = nextYCoordI();
    int x1 = nextXCoordI();
    int y1 = nextYCoordI();

    if (x0 > x1) std::swap(x0, x1);
    if (y0 > y1) std::swap(y0, y1);

    if (x0 == x1) x1++;
    if (y0 == y1) y1++;

    return BLBoxI(x0, y0, x1, y1);
  }

  inline BLRectI nextRectI() {
    BLBoxI box = nextBoxI();
    return BLRectI(box.x0, box.y0, box.x1 - box.x0, box.y1 - box.y0);
  }

  inline BLRect nextRectD() {
    BLBox box = nextBoxD();
    return BLRect(box.x0, box.y0, box.x1 - box.x0, box.y1 - box.y0);
  }

  inline BLTriangle nextTriangle() {
    BLTriangle out;
    out.x0 = nextXCoordD();
    out.y0 = nextYCoordD();
    out.x1 = nextXCoordD();
    out.y1 = nextYCoordD();
    out.x2 = nextXCoordD();
    out.y2 = nextYCoordD();
    return out;
  }
};

enum class FuzzerCommand : uint32_t {
  kFillRectI = 0,
  kFillRectD,
  kFillTriangle,
  kFillPoly10,
  kFillPathQuad,
  kFillPathCubic,
  kFillText,
  kAll,

  kMaxValue = kAll,
  kUnknown = 0xFFFFFFFFu
};

enum class FuzzerStyle : uint32_t {
  kSolid = 0,
  kSolidOpaque,
  kGradientLinear,
  kGradientLinearDither,
  kGradientRadial,
  kGradientRadialDither,
  kGradientConic,
  kGradientConicDither,
  kPatternAligned,
  kPatternUnaligned,
  kPatternAffine,
  kRandom,

  kMaxValue = kRandom,
  kUnknown = 0xFFFFFFFFu
};

namespace StringUtils {

static bool strieq(const char* a, const char* b) {
  size_t aLen = strlen(a);
  size_t bLen = strlen(b);

  if (aLen != bLen)
    return false;

  for (size_t i = 0; i < aLen; i++) {
    unsigned ac = (unsigned char)a[i];
    unsigned bc = (unsigned char)b[i];

    if (ac >= 'A' && ac <= 'Z') ac += 'A' - 'a';
    if (bc >= 'A' && bc <= 'Z') bc += 'A' - 'a';

    if (ac != bc)
      return false;
  }

  return true;
}

static const char* boolToString(bool value) {
  return value ? "true" : "false";
}

static const char* cpuX86FeatureToString(BLRuntimeCpuFeatures feature) {
  switch (feature) {
    case BL_RUNTIME_CPU_FEATURE_X86_SSE2   : return "sse2";
    case BL_RUNTIME_CPU_FEATURE_X86_SSE3   : return "sse3";
    case BL_RUNTIME_CPU_FEATURE_X86_SSSE3  : return "ssse3";
    case BL_RUNTIME_CPU_FEATURE_X86_SSE4_1 : return "sse4.1";
    case BL_RUNTIME_CPU_FEATURE_X86_SSE4_2 : return "sse4.2";
    case BL_RUNTIME_CPU_FEATURE_X86_AVX    : return "avx";
    case BL_RUNTIME_CPU_FEATURE_X86_AVX2   : return "avx2";
    case BL_RUNTIME_CPU_FEATURE_X86_AVX512 : return "avx512";

    default:
      return "unknown";
  }
}

static const char* commandToString(FuzzerCommand command) {
  switch (command) {
    case FuzzerCommand::kFillRectI         : return "fill-rect-i";
    case FuzzerCommand::kFillRectD         : return "fill-rect-d";
    case FuzzerCommand::kFillTriangle      : return "fill-triangle";
    case FuzzerCommand::kFillPoly10        : return "fill-poly-10";
    case FuzzerCommand::kFillPathQuad      : return "fill-path-quad";
    case FuzzerCommand::kFillPathCubic     : return "fill-path-cubic";
    case FuzzerCommand::kFillText          : return "fill-text";
    case FuzzerCommand::kAll               : return "all";

    default:
      return "unknown";
  }
}

static const char* styleToString(FuzzerStyle style) {
  switch (style) {
    case FuzzerStyle::kSolid               : return "solid";
    case FuzzerStyle::kSolidOpaque         : return "solid-opaque";
    case FuzzerStyle::kGradientLinear      : return "gradient-linear";
    case FuzzerStyle::kGradientLinearDither: return "gradient-linear-dither";
    case FuzzerStyle::kGradientRadial      : return "gradient-radial";
    case FuzzerStyle::kGradientRadialDither: return "gradient-radial-dither";
    case FuzzerStyle::kGradientConic       : return "gradient-conic";
    case FuzzerStyle::kGradientConicDither : return "gradient-conic-dither";
    case FuzzerStyle::kPatternAligned      : return "pattern-aligned";
    case FuzzerStyle::kPatternUnaligned    : return "pattern-unaligned";
    case FuzzerStyle::kPatternAffine       : return "pattern-affine";
    case FuzzerStyle::kRandom              : return "random";

    default:
      return "unknown";
  }
}

static FuzzerCommand parseCommand(const char* s) {
  for (uint32_t i = 0; i <= uint32_t(FuzzerCommand::kMaxValue); i++)
    if (StringUtils::strieq(s, commandToString(FuzzerCommand(i))))
      return FuzzerCommand(i);
  return FuzzerCommand::kUnknown;
}

static FuzzerStyle parseStyle(const char* s) {
  for (uint32_t i = 0; i <= uint32_t(FuzzerStyle::kMaxValue); i++)
    if (StringUtils::strieq(s, styleToString(FuzzerStyle(i))))
      return FuzzerStyle(i);
  return FuzzerStyle::kUnknown;
}

} // {StringUtils}

struct FuzzerOptions {
  uint32_t width {};
  uint32_t height {};
  uint32_t count {};
  uint32_t threadCount {};
  uint32_t seed {};
  FuzzerCommand command = FuzzerCommand::kAll;
  FuzzerStyle style = FuzzerStyle::kSolid;

  bool verbose {};
  bool flushSync {};
  bool storeImages {};
};

class ContextFuzzer {
public:
  RandomDataGenerator _rnd;
  BLRandom _rndSync;
  const char* _prefix {};
  Logger _logger;
  BLImage _img;
  BLContext _ctx;
  FuzzerStyle _style {};
  bool _storeImages {};
  bool _flushSync {};
  const char* _imagePrefix {};

  ContextFuzzer(const char* prefix, Logger::Verbosity verbosity)
    : _rndSync(0u),
      _prefix(prefix),
      _logger(verbosity),
      _storeImages(false),
      _flushSync(false),
      _imagePrefix("") {}

  BLResult init(int w, int h, BLFormat format, const BLContextCreateInfo& cci) {
    BL_PROPAGATE(_img.create(w, h, format));
    BL_PROPAGATE(_ctx.begin(_img, cci));

    double oob = 30;

    _rnd.setBounds(BLBox(0.0 - oob, 0.0 - oob, w + oob, h + oob));
    _ctx.clearAll();
    _ctx.setFillStyle(BLRgba32(0xFFFFFFFF));

    return BL_SUCCESS;
  }

  inline void seed(uint32_t seed) { _rnd.seed(seed); }
  inline void setStyle(FuzzerStyle style) { _style = style; }
  inline void setStoreImages(bool value) { _storeImages = value; }
  inline void setFlushSync(bool value) { _flushSync = value; }
  inline void setImagePrefix(const char* prefix) { _imagePrefix = prefix; }

  inline const BLImage& image() const { return _img; }

  void reset() {
    _ctx.reset();
    _img.reset();
  }

  void started(const char* fuzzName) {
    _logger.info("%sRunning '%s'\n", _prefix, fuzzName);
    _rndSync.reset(0xA29CF911A3B729AFu);
  }

  void finished(const char* fuzzName) {
    _ctx.flush(BL_CONTEXT_FLUSH_SYNC);

    if (_storeImages && _img) {
      BLString s;
      s.assignFormat("%s%s.png", _imagePrefix, fuzzName);
      _logger.info("%sStoring: %s\n", _prefix, s.data());
      _img.writeToFile(s.data());
    }
  }

  inline void recordIteration(size_t n) noexcept {
    if (_flushSync && _rndSync.nextUInt32() > 0xF0000000u)
      _ctx.flush(BL_CONTEXT_FLUSH_SYNC);
  }

  FuzzerStyle nextStyle() {
    FuzzerStyle style = _style;
    if (style >= FuzzerStyle::kRandom)
      style = FuzzerStyle(_rnd.nextUInt32() % uint32_t(FuzzerStyle::kRandom));
    return style;
  }

  void prepareContext(BLContext& ctx, FuzzerStyle style) noexcept {
    switch (style) {
      case FuzzerStyle::kGradientLinear:
      case FuzzerStyle::kGradientRadial:
      case FuzzerStyle::kGradientConic:
        ctx.setGradientQuality(BL_GRADIENT_QUALITY_NEAREST);
        break;

      case FuzzerStyle::kGradientLinearDither:
      case FuzzerStyle::kGradientRadialDither:
      case FuzzerStyle::kGradientConicDither:
        ctx.setGradientQuality(BL_GRADIENT_QUALITY_DITHER);
        break;

      default:
        break;
    }
  }

  BLVar getRandomStyleObject(FuzzerStyle style) {
    static constexpr double kPI = 3.14159265358979323846;

    switch (style) {
      default:
      case FuzzerStyle::kSolid: {
        return BLVar(_rnd.nextRgba32());
      }

      case FuzzerStyle::kSolidOpaque: {
        return BLVar(_rnd.nextRgb32());
      }

      case FuzzerStyle::kGradientLinear:
      case FuzzerStyle::kGradientLinearDither: {
        BLPoint pt0 = _rnd.nextPointD();
        BLPoint pt1 = _rnd.nextPointD();

        BLGradient gradient(BLLinearGradientValues(pt0.x, pt0.y, pt1.x, pt1.y));
        gradient.addStop(0.0, _rnd.nextRgba32());
        gradient.addStop(0.5, _rnd.nextRgba32());
        gradient.addStop(1.0, _rnd.nextRgba32());
        gradient.setExtendMode(_rnd.nextGradientExtend());
        return BLVar(gradient);
      }

      case FuzzerStyle::kGradientRadial:
      case FuzzerStyle::kGradientRadialDither: {
        // NOTE: It's tricky with radial gradients as FMA and non-FMA implementations will have a different output.
        // So, we quantize input coordinates to integers to minimize the damage, although we cannot avoid it even
        // in this case.
        double rad = std::floor(_rnd.nextDouble() * 500 + 20);
        double dist = std::floor(_rnd.nextDouble() * (rad - 10));

        double angle = _rnd.nextDouble() * kPI;
        double as = std::sin(angle);
        double ac = std::cos(angle);

        BLPoint pt0 = _rnd.nextPointI();
        BLPoint pt1 = BLPoint(std::floor(-as * dist), std::floor(ac * dist)) + pt0;

        BLGradient gradient(BLRadialGradientValues(pt0.x, pt0.y, pt1.x, pt1.y, rad));
        BLRgba32 c = _rnd.nextRgba32();
        gradient.addStop(0.0, c);
        gradient.addStop(0.5, _rnd.nextRgba32());
        gradient.addStop(1.0, c);
        gradient.setExtendMode(_rnd.nextGradientExtend());
        return BLVar(gradient);
      }

      case FuzzerStyle::kGradientConic:
      case FuzzerStyle::kGradientConicDither: {
        BLPoint pt0 = _rnd.nextPointI();
        double angle = _rnd.nextDouble() * kPI;

        BLGradient gradient(BLConicGradientValues(pt0.x, pt0.y, angle));
        gradient.addStop(0.0 , _rnd.nextRgba32());
        gradient.addStop(0.33, _rnd.nextRgba32());
        gradient.addStop(0.66, _rnd.nextRgba32());
        gradient.addStop(1.0 , _rnd.nextRgba32());
        return BLVar(gradient);
      }

      // TODO:
      // case FuzzerStyle::kPatternAffine: {
      // }
    }
  }

  void clear() { _ctx.clearAll(); }

  void fuzzFillRectI(size_t n) {
    const char* fuzzName = StringUtils::commandToString(FuzzerCommand::kFillRectI);
    started(fuzzName);

    for (size_t i = 0; i < n; i++) {
      FuzzerStyle style = nextStyle();
      prepareContext(_ctx, style);

      BLRectI rect = _rnd.nextRectI();

      _logger.debug("%sFillRectI(%d, %d, %d, %d)\n", _prefix, rect.x, rect.y, rect.w, rect.h);
      _ctx.fillRect(rect, getRandomStyleObject(style));

      recordIteration(i);
    }

    finished(fuzzName);
  }

  void fuzzFillRectD(size_t n) {
    const char* fuzzName = StringUtils::commandToString(FuzzerCommand::kFillRectD);
    started(fuzzName);

    for (size_t i = 0; i < n; i++) {
      FuzzerStyle style = nextStyle();
      prepareContext(_ctx, style);

      BLRect rect = _rnd.nextRectD();

      _logger.debug("%sFillRectD(%g, %g, %g, %g)\n", _prefix, rect.x, rect.y, rect.w, rect.h);
      _ctx.fillRect(rect, getRandomStyleObject(style));

      recordIteration(i);
    }

    finished(fuzzName);
  }

  void fuzzFillTriangle(size_t n) {
    const char* fuzzName = StringUtils::commandToString(FuzzerCommand::kFillTriangle);
    started(fuzzName);

    for (size_t i = 0; i < n; i++) {
      FuzzerStyle style = nextStyle();
      prepareContext(_ctx, style);

      BLTriangle t = _rnd.nextTriangle();

      _logger.debug("%sFillTriangle(%g, %g, %g, %g, %g, %g)\n", _prefix, t.x0, t.y0, t.x1, t.y1, t.x2, t.y2);
      _ctx.fillTriangle(t, getRandomStyleObject(style));

      recordIteration(i);
    }

    finished(fuzzName);
  }

  void fuzzFillPoly10(size_t n) {
    const char* fuzzName = StringUtils::commandToString(FuzzerCommand::kFillPoly10);
    started(fuzzName);

    constexpr uint32_t kPointCount = 10;
    BLPoint pt[kPointCount];

    BLString s;

    for (size_t i = 0; i < n; i++) {
      FuzzerStyle style = nextStyle();
      prepareContext(_ctx, style);

      for (uint32_t j = 0; j < kPointCount; j++)
        pt[j] = _rnd.nextPointD();

      if (_logger.verbosity() > Logger::Verbosity::Debug) {
        s.clear();
        for (uint32_t j = 0; j < kPointCount; j++)
          s.appendFormat("%s%g %g", j == 0 ? "" : ", ", pt[j].x, pt[j].y);
        _logger.debug("%sFillPoly10(%s)\n", _prefix, s.data());
      }

      _ctx.fillPolygon(pt, 10, getRandomStyleObject(style));
      recordIteration(i);
    }

    finished(fuzzName);
  }

  void fuzzFillPathQuads(size_t n) {
    const char* fuzzName = StringUtils::commandToString(FuzzerCommand::kFillPathQuad);
    started(fuzzName);

    for (size_t i = 0; i < n; i++) {
      FuzzerStyle style = nextStyle();
      prepareContext(_ctx, style);

      BLPath path;
      path.moveTo(_rnd.nextPointD());
      path.quadTo(_rnd.nextPointD(), _rnd.nextPointD());

      _ctx.fillPath(path, getRandomStyleObject(style));
      recordIteration(i);
    }

    finished(fuzzName);
  }

  void fuzzFillPathCubics(size_t n) {
    const char* fuzzName = StringUtils::commandToString(FuzzerCommand::kFillPathCubic);
    started(fuzzName);

    for (size_t i = 0; i < n; i++) {
      FuzzerStyle style = nextStyle();
      prepareContext(_ctx, style);

      BLPath path;
      path.moveTo(_rnd.nextPointD());
      path.cubicTo(_rnd.nextPointD(), _rnd.nextPointD(), _rnd.nextPointD());

      _ctx.fillPath(path, getRandomStyleObject(style));
      recordIteration(i);
    }

    finished(fuzzName);
  }

  void fuzzFillText(size_t n, const BLFontData& fontData, uint32_t faceIndex, float fontSize) {
    const char* fuzzName = StringUtils::commandToString(FuzzerCommand::kFillText);
    static const char alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz01234567890!@#$%^&*()_{}:;<>?|";

    started(fuzzName);

    for (size_t i = 0; i < n; i++) {
      FuzzerStyle style = nextStyle();
      prepareContext(_ctx, style);

      BLFontFace face;
      face.createFromData(fontData, faceIndex);

      BLFont font;
      font.createFromFace(face, fontSize);

      // We want to render at least two text runs so there is a chance that text processing
      // and rendering happens in parallel in case the rendering context uses multi-threading.
      uint32_t rnd0 = _rnd.nextUInt32();
      uint32_t rnd1 = _rnd.nextUInt32();

      char str0[5] {};
      str0[0] = alphabet[((rnd0 >>  0) & 0xFF) % (sizeof(alphabet) - 1u)];
      str0[1] = alphabet[((rnd0 >>  8) & 0xFF) % (sizeof(alphabet) - 1u)];
      str0[2] = alphabet[((rnd0 >> 16) & 0xFF) % (sizeof(alphabet) - 1u)];
      str0[3] = alphabet[((rnd0 >> 24) & 0xFF) % (sizeof(alphabet) - 1u)];

      char str1[5] {};
      str1[0] = alphabet[((rnd1 >>  0) & 0xFF) % (sizeof(alphabet) - 1u)];
      str1[1] = alphabet[((rnd1 >>  8) & 0xFF) % (sizeof(alphabet) - 1u)];
      str1[2] = alphabet[((rnd1 >> 16) & 0xFF) % (sizeof(alphabet) - 1u)];
      str1[3] = alphabet[((rnd1 >> 24) & 0xFF) % (sizeof(alphabet) - 1u)];

      BLPoint pt0 = _rnd.nextPointD();
      BLPoint pt1 = _rnd.nextPointD();

      BLVar v = getRandomStyleObject(style);
      _ctx.fillUtf8Text(pt0, font, BLStringView{str0, 4}, v);
      _ctx.fillUtf8Text(pt1, font, BLStringView{str1, 4}, v);
      recordIteration(i);
    }

    finished(fuzzName);
  }
};

namespace ImageUtils {

struct DiffInfo {
  uint32_t maxDiff;
  uint64_t cumulativeDiff;
};

static DiffInfo diffInfo(const BLImage& aImage, const BLImage& bImage) noexcept {
  DiffInfo info {};
  BLImageData aData;
  BLImageData bData;

  if (aImage.size() != bImage.size())
    return info;

  size_t w = size_t(aImage.width());
  size_t h = size_t(aImage.height());

  if (aImage.getData(&aData) != BL_SUCCESS)
    return info;

  if (bImage.getData(&bData) != BL_SUCCESS)
    return info;

  intptr_t aStride = aData.stride;
  intptr_t bStride = bData.stride;

  const uint8_t* aLine = static_cast<const uint8_t*>(aData.pixelData);
  const uint8_t* bLine = static_cast<const uint8_t*>(bData.pixelData);

  for (size_t y = 0; y < h; y++) {
    const uint32_t* aPtr = reinterpret_cast<const uint32_t*>(aLine);
    const uint32_t* bPtr = reinterpret_cast<const uint32_t*>(bLine);

    for (size_t x = 0; x < w; x++) {
      uint32_t aVal = aPtr[x];
      uint32_t bVal = bPtr[x];

      if (aVal != bVal) {
        int aDiff = blAbs(int((aVal >> 24) & 0xFF) - int((bVal >> 24) & 0xFF));
        int rDiff = blAbs(int((aVal >> 16) & 0xFF) - int((bVal >> 16) & 0xFF));
        int gDiff = blAbs(int((aVal >>  8) & 0xFF) - int((bVal >>  8) & 0xFF));
        int bDiff = blAbs(int((aVal      ) & 0xFF) - int((bVal      ) & 0xFF));
        int maxDiff = blMax(aDiff, rDiff, gDiff, bDiff);

        info.maxDiff = blMax(info.maxDiff, uint32_t(maxDiff));
        info.cumulativeDiff += maxDiff;
      }
    }

    aLine += aStride;
    bLine += bStride;
  }

  return info;
}

static BLImage diffImage(const BLImage& aImage, const BLImage& bImage) noexcept {
  BLImage result;
  BLImageData rData;
  BLImageData aData;
  BLImageData bData;

  if (aImage.size() != bImage.size())
    return result;

  size_t w = size_t(aImage.width());
  size_t h = size_t(aImage.height());

  if (aImage.getData(&aData) != BL_SUCCESS)
    return result;

  if (bImage.getData(&bData) != BL_SUCCESS)
    return result;

  if (result.create(w, h, BL_FORMAT_XRGB32) != BL_SUCCESS)
    return result;

  if (result.getData(&rData) != BL_SUCCESS)
    return result;

  intptr_t dStride = rData.stride;
  intptr_t aStride = aData.stride;
  intptr_t bStride = bData.stride;

  uint8_t* dLine = static_cast<uint8_t*>(rData.pixelData);
  const uint8_t* aLine = static_cast<const uint8_t*>(aData.pixelData);
  const uint8_t* bLine = static_cast<const uint8_t*>(bData.pixelData);

  for (size_t y = 0; y < h; y++) {
    uint32_t* dPtr = reinterpret_cast<uint32_t*>(dLine);
    const uint32_t* aPtr = reinterpret_cast<const uint32_t*>(aLine);
    const uint32_t* bPtr = reinterpret_cast<const uint32_t*>(bLine);

    for (size_t x = 0; x < w; x++) {
      uint32_t aVal = aPtr[x];
      uint32_t bVal = bPtr[x];

      int aDiff = blAbs(int((aVal >> 24) & 0xFF) - int((bVal >> 24) & 0xFF));
      int rDiff = blAbs(int((aVal >> 16) & 0xFF) - int((bVal >> 16) & 0xFF));
      int gDiff = blAbs(int((aVal >>  8) & 0xFF) - int((bVal >>  8) & 0xFF));
      int bDiff = blAbs(int((aVal      ) & 0xFF) - int((bVal      ) & 0xFF));

      int maxDiff = blMax(aDiff, rDiff, gDiff, bDiff);
      uint32_t dVal = 0xFF000000u;

      if (maxDiff) {
        if (maxDiff <= 4)
          dVal = 0xFF000000u + unsigned((maxDiff * 64 - 1) << 0);
        else if (maxDiff <= 16)
          dVal = 0xFF000000u + unsigned((maxDiff * 16 - 1) << 8);
        else
          dVal = 0xFF000000u + unsigned((127 + maxDiff / 2) << 16);
      }

      dPtr[x] = dVal;
    }

    dLine += dStride;
    aLine += aStride;
    bLine += bStride;
  }

  return result;
}

} // {ImageUtils}

#endif // BLEND2D_TEST_FUZZ_UTILITIES_H_INCLUDED
