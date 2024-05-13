// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <blend2d.h>
#include <cmath>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bl_test_fuzz_baseapp.h"
#include "bl_test_fuzz_utilities.h"
#include "resources/abeezee_regular_ttf.h"

#if defined(_M_X64) || defined(__amd64) || defined(__amd64__) || defined(__x86_64) || defined(__x86_64__) || defined(_M_IX86) || defined(__i386) || defined(__i386__)
  #define BL_JIT_ARCH_X86
#elif defined(_M_ARM64) || defined(__ARM64__) || defined(__aarch64__)
  #define BL_JIT_ARCH_A64
#endif

class JITFuzzerApp : public BaseFuzzerApp {
public:
  BLString _cpuFeaturesString;

  bool iterateAllJitFeatures = false;
  uint32_t selectedCpuFeatures {};
  uint64_t mismatchCount {};
  uint32_t failedCount {};
  uint32_t passedCount {};

  JITFuzzerApp()
    : BaseFuzzerApp() {}

  int help() {
    using StringUtils::boolToString;

    printf("Usage:\n");
    printf("  bl_test_fuzz_jit [Options]\n");
    printf("\n");

    printf("Purpose:\n");
    printf("  JIT fuzzer is designed to verify whether JIT-compiled pipelines and\n");
    printf("  reference pipelines yield pixel identical output when used with the\n");
    printf("  same input data. Additionally, JIT fuzzer verifies that JIT compiled\n");
    printf("  pipelines are actually compiled successfully.\n");
    printf("\n");

    printCommonOptions(defaultOptions);

    printf("JIT Fuzzer Options:\n");
    printf("  --simd-level=<name>    - SIMD level                        [default=native]\n");
    printf("\n");

#if defined(BL_JIT_ARCH_X86)
    printf("JIT SIMD Levels (X86 and X86_64):\n");
    printf("  sse2                   - Enables SSE2   (x86 baseline)  [128-bit SIMD]\n");
    printf("  sse3                   - Enables SSE3   (if available)  [128-bit SIMD]\n");
    printf("  ssse3                  - Enables SSSE3  (if available)  [128-bit SIMD]\n");
    printf("  sse4.1                 - Enables SSE4.1 (if available)  [128-bit SIMD]\n");
    printf("  sse4.2                 - Enables SSE4.2 (if available)  [128-bit SIMD]\n");
    printf("  avx                    - Enables AVX    (if available)  [128-bit SIMD]\n");
    printf("  avx2                   - Enables AVX2   (if available)  [256-bit SIMD]\n");
    printf("  avx512                 - Enables AVX512 (F|CD|BW|DQ|VL) [512-bit SIMD]\n");
#elif defined(BL_JIT_ARCH_A64)
    printf("JIT SIMD Levels (AArch64):\n");
    printf("  asimd                  - Enables ADIMD (aarch64 baseline)\n");
#else
    printf("JIT SIMD Levels (Unknown Architecture!):\n");
#endif
    printf("  all                    - Execute all possible SIMD levels\n");
    printf("  native                 - Uses features detected by Blend2D\n");
    printf("\n");

    printCommands();
    printStyles();

    fflush(stdout);
    return 0;
  }

  void resetCounters() {
    mismatchCount = 0;
  }

  bool parseJITOptions(CmdLine cmdLine) {
    const char* simdLevel = cmdLine.valueOf("--simd-level", "all");

    if (simdLevel) {
      if (StringUtils::strieq(simdLevel, "native")) {
        // Nothing to do if configured to auto-detect.
      }
      else if (StringUtils::strieq(simdLevel, "all")) {
        iterateAllJitFeatures = true;
      }
#if defined(BL_JIT_ARCH_X86)
      else if (StringUtils::strieq(simdLevel, "sse2")) {
        selectedCpuFeatures = BL_RUNTIME_CPU_FEATURE_X86_SSE2;
      }
      else if (StringUtils::strieq(simdLevel, "sse3")) {
        selectedCpuFeatures = BL_RUNTIME_CPU_FEATURE_X86_SSE3;
      }
      else if (StringUtils::strieq(simdLevel, "ssse3")) {
        selectedCpuFeatures = BL_RUNTIME_CPU_FEATURE_X86_SSSE3;
      }
      else if (StringUtils::strieq(simdLevel, "sse4.1")) {
        selectedCpuFeatures = BL_RUNTIME_CPU_FEATURE_X86_SSE4_1;
      }
      else if (StringUtils::strieq(simdLevel, "sse4.2")) {
        selectedCpuFeatures = BL_RUNTIME_CPU_FEATURE_X86_SSE4_2;
      }
      else if (StringUtils::strieq(simdLevel, "avx")) {
        selectedCpuFeatures = BL_RUNTIME_CPU_FEATURE_X86_AVX;
      }
      else if (StringUtils::strieq(simdLevel, "avx2")) {
        selectedCpuFeatures = BL_RUNTIME_CPU_FEATURE_X86_AVX2;
      }
      else if (StringUtils::strieq(simdLevel, "avx512")) {
        selectedCpuFeatures = BL_RUNTIME_CPU_FEATURE_X86_AVX512;
      }
#elif defined(BL_JIT_ARCH_A64)
      else if (strcmp(simdLevel, "asimd") == 0) {
        // Currently the default...
      }
#endif
      else {
        printf("Failed to process command line arguments:\n");
        printf("  Unknown simd-level '%s' - please use --help to list all available simd levels\n", simdLevel);
        return false;
      }
    }

    return true;
  }

  template<typename FuzzFunc>
  void fuzz(FuzzerCommand cmd, ContextFuzzer& aFuzzer, ContextFuzzer& bFuzzer, FuzzFunc&& fuzzFunc) {
    printf("Testing [%s | %s | %s]:\n",
      StringUtils::commandToString(cmd),
      StringUtils::styleToString(options.style),
      _cpuFeaturesString.data());

    BLString op;
    op.assignFormat("%s-%s-%s",
      StringUtils::commandToString(cmd),
      StringUtils::styleToString(options.style),
      _cpuFeaturesString.data());

    aFuzzer.clear();
    bFuzzer.clear();

    aFuzzer.seed(options.seed);
    bFuzzer.seed(options.seed);

    fuzzFunc(&aFuzzer, options.count);
    fuzzFunc(&bFuzzer, options.count);

    if (check(op.data(), aFuzzer.image(), bFuzzer.image())) {
      passedCount++;
    }
    else {
      failedCount++;
      findProblem(op.data(), aFuzzer, bFuzzer, fuzzFunc);
    }
  }

  bool check(const char* prefix, const BLImage& aImage, const BLImage& bImage) {
    ImageUtils::DiffInfo diffInfo = ImageUtils::diffInfo(aImage, bImage);
    if (diffInfo.maxDiff == 0)
      return true;

    mismatchCount++;
    BLString baseName;
    baseName.assignFormat("%s-bug-%05llu", prefix, (unsigned long long)mismatchCount);
    printf("  Mismatch: %s (maxDiff=%u cumulative=%llu)\n", baseName.data(), diffInfo.maxDiff, (unsigned long long)diffInfo.cumulativeDiff);

    if (options.storeImages) {
      BLImage d = ImageUtils::diffImage(aImage, bImage);

      BLString diffName = baseName;
      BLString aImageName = baseName;
      BLString bImageName = baseName;

      diffName.append(".png");
      aImageName.append("-ref.png");
      bImageName.append("-jit.png");

      d.writeToFile(diffName.data());
      aImage.writeToFile(aImageName.data());
      bImage.writeToFile(bImageName.data());
    }

    return false;
  }

  template<typename FuzzFunc>
  void findProblem(const char* prefix, ContextFuzzer& aFuzzer, ContextFuzzer& bFuzzer, FuzzFunc&& fuzzFunc) {
    // Do a binary search to find exactly the failing command.
    size_t base = 0;
    size_t size = options.count;

    Logger& logger = aFuzzer._logger;
    logger.print("  Bisecting to match the problematic command...");

    Logger::Verbosity aLoggerVerbosity = aFuzzer._logger.setVerbosity(Logger::Verbosity::Silent);
    Logger::Verbosity bLoggerVerbosity = bFuzzer._logger.setVerbosity(Logger::Verbosity::Silent);

    while (size_t half = size / 2u) {
      size_t middle = base + half;
      size -= half;

      logger.print("  Verifying range [%zu %zu)\n", base, base + size);

      aFuzzer.clear();
      bFuzzer.clear();

      aFuzzer.seed(options.seed);
      bFuzzer.seed(options.seed);

      fuzzFunc(&aFuzzer, base + size);
      fuzzFunc(&bFuzzer, base + size);

      check(prefix, aFuzzer.image(), bFuzzer.image());

      if (ImageUtils::diffInfo(aFuzzer.image(), bFuzzer.image()).maxDiff == 0)
        base = middle;
    }

    logger.print("  Mismatch command index: %zu\n", base);

    aFuzzer.clear();
    bFuzzer.clear();

    aFuzzer.seed(options.seed);
    bFuzzer.seed(options.seed);

    if (base) {
      fuzzFunc(&aFuzzer, base - 1);
      fuzzFunc(&bFuzzer, base - 1);
    }

    aFuzzer._logger.setVerbosity(Logger::Verbosity::Debug);
    bFuzzer._logger.setVerbosity(Logger::Verbosity::Debug);

    fuzzFunc(&aFuzzer, 1);
    fuzzFunc(&bFuzzer, 1);

    aFuzzer._logger.setVerbosity(aLoggerVerbosity);
    bFuzzer._logger.setVerbosity(aLoggerVerbosity);

    check(prefix, aFuzzer.image(), bFuzzer.image());
  }

  bool runWithFeatures(uint32_t cpuFeatures) {
    resetCounters();

    if (cpuFeatures)
      _cpuFeaturesString.assign(StringUtils::cpuX86FeatureToString(BLRuntimeCpuFeatures(cpuFeatures)));
    else
      _cpuFeaturesString.assign("native");

    BLString aFuzzerName;
    BLString bFuzzerName;

    ContextFuzzer aFuzzer("  [ref] ", options.verbose ? Logger::Verbosity::Debug : Logger::Verbosity::Info);
    ContextFuzzer bFuzzer("  [jit] ", Logger::Verbosity::Info);

    aFuzzer.setStyle(options.style);
    bFuzzer.setStyle(options.style);

    aFuzzer.setFlushSync(options.flushSync);
    bFuzzer.setFlushSync(options.flushSync);

    BLContextCreateInfo aCreateInfo {};
    BLContextCreateInfo bCreateInfo {};

    aCreateInfo.flags = BL_CONTEXT_CREATE_FLAG_DISABLE_JIT;

    if (cpuFeatures) {
      bCreateInfo.flags = BL_CONTEXT_CREATE_FLAG_ISOLATED_JIT_RUNTIME |
                          BL_CONTEXT_CREATE_FLAG_OVERRIDE_CPU_FEATURES;
      bCreateInfo.cpuFeatures = cpuFeatures;
    }

    if (aFuzzer.init(int(options.width), int(options.height), BL_FORMAT_PRGB32, aCreateInfo) != BL_SUCCESS ||
        bFuzzer.init(int(options.width), int(options.height), BL_FORMAT_PRGB32, bCreateInfo) != BL_SUCCESS) {
      printf("Failed to initialize rendering contexts\n");
      return 1;
    }

    if (shouldRun(FuzzerCommand::kFillRectI)) {
      fuzz(FuzzerCommand::kFillRectI, aFuzzer, bFuzzer, [](ContextFuzzer* fuzzer, size_t n) { fuzzer->fuzzFillRectI(n); });
    }

    if (shouldRun(FuzzerCommand::kFillRectD)) {
      fuzz(FuzzerCommand::kFillRectD, aFuzzer, bFuzzer, [](ContextFuzzer* fuzzer, size_t n) { fuzzer->fuzzFillRectD(n); });
    }

    if (shouldRun(FuzzerCommand::kFillTriangle)) {
      fuzz(FuzzerCommand::kFillTriangle, aFuzzer, bFuzzer, [](ContextFuzzer* fuzzer, size_t n) { fuzzer->fuzzFillTriangle(n); });
    }

    if (shouldRun(FuzzerCommand::kFillPoly10)) {
      fuzz(FuzzerCommand::kFillPoly10, aFuzzer, bFuzzer, [](ContextFuzzer* fuzzer, size_t n) { fuzzer->fuzzFillPoly10(n); });
    }

    if (shouldRun(FuzzerCommand::kFillPathQuad)) {
      fuzz(FuzzerCommand::kFillPathQuad, aFuzzer, bFuzzer, [](ContextFuzzer* fuzzer, size_t n) { fuzzer->fuzzFillPathQuads(n); });
    }

    if (shouldRun(FuzzerCommand::kFillPathCubic)) {
      fuzz(FuzzerCommand::kFillPathCubic, aFuzzer, bFuzzer, [](ContextFuzzer* fuzzer, size_t n) { fuzzer->fuzzFillPathCubics(n); });
    }

    if (shouldRun(FuzzerCommand::kFillText)) {
      BLFontData fontData;
      fontData.createFromData(resource_abeezee_regular_ttf, sizeof(resource_abeezee_regular_ttf));
      fuzz(FuzzerCommand::kFillText, aFuzzer, bFuzzer, [&](ContextFuzzer* fuzzer, size_t n) { fuzzer->fuzzFillText(n, fontData, 0u, 20.0f); });
    }

    aFuzzer.reset();
    bFuzzer.reset();

    if (mismatchCount)
      printf("Found %llu mismatches!\n\n", (unsigned long long)mismatchCount);
    else
      printf("\n");

    return !mismatchCount;
  }

  int run(CmdLine cmdLine) {
    printAppInfo("Blend2D JIT Fuzzer");

    if (cmdLine.hasArg("--help"))
      return help();

    if (!parseCommonOptions(cmdLine) || !parseJITOptions(cmdLine))
      return 1;

    if (iterateAllJitFeatures) {
#if defined(BL_JIT_ARCH_X86)
      static constexpr uint32_t x86FeaturesList[] = {
        BL_RUNTIME_CPU_FEATURE_X86_SSE2,
        BL_RUNTIME_CPU_FEATURE_X86_SSE3,
        BL_RUNTIME_CPU_FEATURE_X86_SSSE3,
        BL_RUNTIME_CPU_FEATURE_X86_SSE4_1,
        BL_RUNTIME_CPU_FEATURE_X86_SSE4_2,
        BL_RUNTIME_CPU_FEATURE_X86_AVX,
        BL_RUNTIME_CPU_FEATURE_X86_AVX2,
        BL_RUNTIME_CPU_FEATURE_X86_AVX512
      };

      BLRuntimeSystemInfo systemInfo {};
      BLRuntime::querySystemInfo(&systemInfo);

      for (const uint32_t& feature : x86FeaturesList) {
        if (!(systemInfo.cpuFeatures & feature))
          break;
        runWithFeatures(feature);
      }
#endif

      // Now run with all features if everything above has passed.
      runWithFeatures(0);
    }
    else {
      runWithFeatures(selectedCpuFeatures);
    }

    if (failedCount) {
      printf("[FAILED] %u tests out of %u failed\n", failedCount, passedCount + failedCount);
      return 1;
    }
    else {
      printf("[PASSED] %u tests passed\n", passedCount);
      return 0;
    }
  }
};

int main(int argc, char* argv[]) {
  return JITFuzzerApp().run(CmdLine(argc, argv));
}
