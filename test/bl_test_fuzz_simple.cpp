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

class SimpleFuzzerApp : public BaseFuzzerApp {
public:
  SimpleFuzzerApp()
    : BaseFuzzerApp() {}

  int help() {
    using StringUtils::boolToString;

    printf("Usage:\n");
    printf("  bl_test_fuzz_simple [Options]\n");
    printf("\n");

    printf("Purpose:\n");
    printf("  Simple fuzzer is designed to verify that the rendering context can\n");
    printf("  process input commands without crashing or causing undefined behavior.\n");
    printf("  It's also designed to be run with instrumentation (ASAN, UBSAN, etc...).\n");
    printf("\n");

    printCommonOptions(defaultOptions);
    printCommands();
    printStyles();

    fflush(stdout);
    return 0;
  }

  int run(CmdLine cmdLine) {
    printAppInfo("Blend2D Simple Fuzzer");

    if (cmdLine.hasArg("--help"))
      return help();

    if (!parseCommonOptions(cmdLine))
      return 1;

    // Fuzzing...
    ContextFuzzer fuzzer("", options.verbose ? Logger::Verbosity::Debug : Logger::Verbosity::Info);
    fuzzer.seed(options.seed);
    fuzzer.setStyle(options.style);
    fuzzer.setStoreImages(options.storeImages);
    fuzzer.setFlushSync(options.flushSync);
    fuzzer.setImagePrefix("fuzz-simple-");

    BLContextCreateInfo cci {};
    cci.threadCount = options.threadCount;

    if (fuzzer.init(int(options.width), int(options.height), BL_FORMAT_PRGB32, cci) != BL_SUCCESS) {
      printf("Failed to initialize the rendering context\n");
      return 1;
    }

    if (shouldRun(FuzzerCommand::kFillRectI)) {
      fuzzer.clear();
      fuzzer.fuzzFillRectI(options.count);
    }

    if (shouldRun(FuzzerCommand::kFillRectD)) {
      fuzzer.clear();
      fuzzer.fuzzFillRectD(options.count);
    }

    if (shouldRun(FuzzerCommand::kFillTriangle)) {
      fuzzer.clear();
      fuzzer.fuzzFillTriangle(options.count);
    }

    if (shouldRun(FuzzerCommand::kFillPoly10)) {
      fuzzer.clear();
      fuzzer.fuzzFillPoly10(options.count);
    }

    if (shouldRun(FuzzerCommand::kFillPathQuad)) {
      fuzzer.clear();
      fuzzer.fuzzFillPathQuads(options.count);
    }

    if (shouldRun(FuzzerCommand::kFillPathCubic)) {
      fuzzer.clear();
      fuzzer.fuzzFillPathCubics(options.count);
    }

    if (shouldRun(FuzzerCommand::kFillText)) {
      BLFontData fontData;
      fontData.createFromData(resource_abeezee_regular_ttf, sizeof(resource_abeezee_regular_ttf));

      fuzzer.clear();
      fuzzer.fuzzFillText(options.count, fontData, 0u, 20.0f);
    }

    fuzzer.reset();

    printf("Fuzzing finished...\n");
    return 0;
  }
};

int main(int argc, char* argv[]) {
  return SimpleFuzzerApp().run(CmdLine(argc, argv));
}
