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

class MTFuzzerApp : public BaseFuzzerApp {
public:
  typedef void FuzzFunc(ContextFuzzer* fuzzer, size_t n);

  uint64_t mismatchCount {};

  MTFuzzerApp() : BaseFuzzerApp() {
    defaultOptions.threadCount = 2u;
  }

  int help() {
    using StringUtils::boolToString;

    printf("Usage:\n");
    printf("  bl_test_fuzz_mt [Options]\n");
    printf("\n");

    printf("Purpose:\n");
    printf("  Multi-threaded fuzzer is designed to verify whether single-threaded and\n");
    printf("  multi-threaded rendering yields pixel identical output when used with\n");
    printf("  the same input data.\n");
    printf("\n");

    printCommonOptions(defaultOptions);
    printCommands();
    printStyles();

    fflush(stdout);
    return 0;
  }

  template<typename FuzzFunc>
  void fuzz(const char* fuzzName, ContextFuzzer& aFuzzer, ContextFuzzer& bFuzzer, FuzzFunc&& fuzzFunc) {
    aFuzzer.clear();
    bFuzzer.clear();

    aFuzzer.seed(options.seed);
    bFuzzer.seed(options.seed);

    fuzzFunc(&aFuzzer, options.count);
    fuzzFunc(&bFuzzer, options.count);

    if (check(fuzzName, aFuzzer.image(), bFuzzer.image()))
      return;

    findProblem(fuzzName, aFuzzer, bFuzzer, fuzzFunc);
  }

  bool check(const char* fuzzName, const BLImage& aImage, const BLImage& bImage) {
    ImageUtils::DiffInfo diffInfo = ImageUtils::diffInfo(aImage, bImage);
    if (diffInfo.maxDiff == 0)
      return true;

    mismatchCount++;
    BLString fileName;
    fileName.assignFormat("fuzz-mt-%s-bug-%05llu.png", fuzzName, (unsigned long long)mismatchCount);
    printf("Mismatch: %s\n", fileName.data());

    if (options.storeImages) {
      BLImage d = ImageUtils::diffImage(aImage, bImage);
      d.writeToFile(fileName.data());
    }

    return false;
  }

  template<typename FuzzFunc>
  void findProblem(const char* fuzzName, ContextFuzzer& aFuzzer, ContextFuzzer& bFuzzer, FuzzFunc&& fuzzFunc) {
    // Do a binary search to find exactly the failing command.
    size_t base = 0;
    size_t size = options.count;

    Logger& logger = aFuzzer._logger;
    logger.print("Bisecting to match the problematic command...\n");

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

      check(fuzzName, aFuzzer.image(), bFuzzer.image());

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

    check(fuzzName, aFuzzer.image(), bFuzzer.image());
  }

  int run(CmdLine cmdLine) {
    printAppInfo("Blend2D Multi-Threaded Fuzzer");

    if (cmdLine.hasArg("--help"))
      return help();

    if (!parseCommonOptions(cmdLine))
      return 1;

    ContextFuzzer aFuzzer("[ST] ", options.verbose ? Logger::Verbosity::Debug : Logger::Verbosity::Info);
    ContextFuzzer bFuzzer("[MT] ", Logger::Verbosity::Info);

    aFuzzer.setStyle(options.style);
    bFuzzer.setStyle(options.style);

    aFuzzer.setFlushSync(options.flushSync);
    bFuzzer.setFlushSync(options.flushSync);

    BLContextCreateInfo aCreateInfo {};
    BLContextCreateInfo bCreateInfo {};

    bCreateInfo.threadCount = options.threadCount;

    if (aFuzzer.init(int(options.width), int(options.height), BL_FORMAT_PRGB32, aCreateInfo) != BL_SUCCESS ||
        bFuzzer.init(int(options.width), int(options.height), BL_FORMAT_PRGB32, bCreateInfo) != BL_SUCCESS) {
      printf("Failed to initialize rendering contexts\n");
      return 1;
    }

    if (shouldRun(FuzzerCommand::kFillRectI)) {
      const char* fuzzerName = StringUtils::commandToString(FuzzerCommand::kFillRectI);
      fuzz(fuzzerName, aFuzzer, bFuzzer, [](ContextFuzzer* fuzzer, size_t n) { fuzzer->fuzzFillRectI(n); });
    }

    if (shouldRun(FuzzerCommand::kFillRectD)) {
      const char* fuzzerName = StringUtils::commandToString(FuzzerCommand::kFillRectD);
      fuzz(fuzzerName, aFuzzer, bFuzzer, [](ContextFuzzer* fuzzer, size_t n) { fuzzer->fuzzFillRectD(n); });
    }

    if (shouldRun(FuzzerCommand::kFillTriangle)) {
      const char* fuzzerName = StringUtils::commandToString(FuzzerCommand::kFillTriangle);
      fuzz(fuzzerName, aFuzzer, bFuzzer, [](ContextFuzzer* fuzzer, size_t n) { fuzzer->fuzzFillTriangle(n); });
    }

    if (shouldRun(FuzzerCommand::kFillPoly10)) {
      const char* fuzzerName = StringUtils::commandToString(FuzzerCommand::kFillPoly10);
      fuzz(fuzzerName, aFuzzer, bFuzzer, [](ContextFuzzer* fuzzer, size_t n) { fuzzer->fuzzFillPoly10(n); });
    }

    if (shouldRun(FuzzerCommand::kFillPathQuad)) {
      const char* fuzzerName = StringUtils::commandToString(FuzzerCommand::kFillPathQuad);
      fuzz(fuzzerName, aFuzzer, bFuzzer, [](ContextFuzzer* fuzzer, size_t n) { fuzzer->fuzzFillPathQuads(n); });
    }

    if (shouldRun(FuzzerCommand::kFillPathCubic)) {
      const char* fuzzerName = StringUtils::commandToString(FuzzerCommand::kFillPathCubic);
      fuzz(fuzzerName, aFuzzer, bFuzzer, [](ContextFuzzer* fuzzer, size_t n) { fuzzer->fuzzFillPathCubics(n); });
    }

    if (shouldRun(FuzzerCommand::kFillText)) {
      BLFontData fontData;
      fontData.createFromData(resource_abeezee_regular_ttf, sizeof(resource_abeezee_regular_ttf));

      const char* fuzzerName = StringUtils::commandToString(FuzzerCommand::kFillText);
      fuzz(fuzzerName, aFuzzer, bFuzzer, [&](ContextFuzzer* fuzzer, size_t n) { fuzzer->fuzzFillText(n, fontData, 0u, 20.0f); });
    }

    aFuzzer.reset();
    bFuzzer.reset();

    printf("Fuzzing finished...\n");

    if (mismatchCount)
      printf("Found %llu mismatches!\n", (unsigned long long)mismatchCount);
    else
      printf("No mismatches found!\n");

    return mismatchCount ? 1 : 0;
  }
};

int main(int argc, char* argv[]) {
  return MTFuzzerApp().run(CmdLine(argc, argv));
}
