// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

// This file provides utility classes and functions shared between some tests.

#ifndef BLEND2D_TEST_FUZZ_BASEAPP_H_INCLUDED
#define BLEND2D_TEST_FUZZ_BASEAPP_H_INCLUDED

#include <blend2d.h>
#include "bl_test_fuzz_utilities.h"

class BaseFuzzerApp {
public:
  FuzzerOptions defaultOptions {};
  FuzzerOptions options {};

  BaseFuzzerApp()
    : defaultOptions{makeDefaultOptions()} {}

  static FuzzerOptions makeDefaultOptions() {
    FuzzerOptions opt {};
    opt.width = 513;
    opt.height = 513;
    opt.count = 1000; //25000;
    opt.threadCount = 0;
    opt.seed = 1;
    opt.command = FuzzerCommand::kAll;
    opt.style = FuzzerStyle::kGradientLinearDither;
    opt.verbose = false;
    opt.flushSync = false;
    return opt;
  }

  bool parseCommonOptions(const CmdLine& cmdLine) {
    options.width = cmdLine.valueAsUInt("--width", defaultOptions.width);
    options.height = cmdLine.valueAsUInt("--height", defaultOptions.height);
    options.count = cmdLine.valueAsUInt("--count", defaultOptions.count);
    options.threadCount = cmdLine.valueAsUInt("--thread-count", defaultOptions.threadCount);
    options.seed = cmdLine.valueAsUInt("--seed", defaultOptions.seed);
    options.command = StringUtils::parseCommand(cmdLine.valueOf("--command", StringUtils::commandToString(defaultOptions.command)));
    options.style = StringUtils::parseStyle(cmdLine.valueOf("--style", StringUtils::styleToString(defaultOptions.style)));
    options.verbose = cmdLine.hasArg("--verbose") || defaultOptions.verbose;
    options.flushSync = cmdLine.hasArg("--flush-sync") || defaultOptions.flushSync;
    options.storeImages = cmdLine.hasArg("--store") || defaultOptions.storeImages;

    if (options.command == FuzzerCommand::kUnknown || options.style == FuzzerStyle::kUnknown) {
      printf("Failed to process command line arguments:\n");

      if (options.command == FuzzerCommand::kUnknown)
        printf("  Unknown command '%s' - please use --help to list all available commands\n", cmdLine.valueOf("--command", ""));

      if (options.style == FuzzerStyle::kUnknown)
        printf("  Unknown style '%s' - please use --help to list all available styles\n", cmdLine.valueOf("--style", ""));

      return false;
    }

    return true;
  }

  bool shouldRun(FuzzerCommand cmd) const {
    return options.command == cmd || options.command == FuzzerCommand::kAll;
  }

  void printAppInfo(const char* title) {
    BLRuntimeBuildInfo buildInfo;
    BLRuntime::queryBuildInfo(&buildInfo);

    printf(
      "%s [use --help for command line options]\n"
      "  Version    : %u.%u.%u\n"
      "  Build Type : %s\n"
      "  Compiled By: %s\n\n",
      title,
      buildInfo.majorVersion,
      buildInfo.minorVersion,
      buildInfo.patchVersion,
      buildInfo.buildType == BL_RUNTIME_BUILD_TYPE_DEBUG ? "Debug" : "Release",
      buildInfo.compilerInfo);
    fflush(stdout);
  }

  void printCommonOptions(const FuzzerOptions& defaultOptions) {
    using StringUtils::boolToString;
    using StringUtils::commandToString;
    using StringUtils::styleToString;

    printf("Common Fuzzer Options:\n");
    printf("  --width=<uint>         - Image width                       [default=%u]\n", defaultOptions.width);
    printf("  --height=<uint>        - Image height                      [default=%u]\n", defaultOptions.height);
    printf("  --count=<uint>         - Count of render commands          [default=%u]\n", defaultOptions.count);
    printf("  --thread-count=<uint>  - Number of threads of MT context   [default=%u]\n", defaultOptions.threadCount);
    printf("  --seed=<uint>          - Random number generator seed      [default=%u]\n", defaultOptions.seed);
    printf("  --command=<string>     - Specify which command to run      [default=%s]\n", commandToString(defaultOptions.command));
    printf("  --style=<string>       - Style to render commands with     [default=%s]\n", styleToString(defaultOptions.style));
    printf("  --store                - Write resulting images to files   [default=%s]\n", boolToString(defaultOptions.storeImages));
    printf("  --verbose              - Debug each render command         [default=%s]\n", boolToString(defaultOptions.verbose));
    printf("  --flush-sync           - Do occasional syncs between calls [default=%s]\n", boolToString(defaultOptions.flushSync));
    printf("\n");
  }

  void printCommands() {
    using StringUtils::commandToString;

    printf("Fuzzer Commands:\n");
    printf("  %-22s - Fills aligned rectangles (int coordinates)\n", commandToString(FuzzerCommand::kFillRectI));
    printf("  %-22s - Fills unaligned rectangles (float coordinates)\n", commandToString(FuzzerCommand::kFillRectD));
    printf("  %-22s - Fills triangles\n", commandToString(FuzzerCommand::kFillTriangle));
    printf("  %-22s - Fills path having quadratic curves\n", commandToString(FuzzerCommand::kFillPathQuad));
    printf("  %-22s - Fills path having cubic curves\n", commandToString(FuzzerCommand::kFillPathCubic));
    printf("  %-22s - Fills text runs\n", commandToString(FuzzerCommand::kFillText));
    printf("  %-22s - Executes all commands\n", commandToString(FuzzerCommand::kAll));
    printf("\n");
  }

  void printStyles() {
    using StringUtils::styleToString;

    printf("Fuzzer Styles:\n");
    printf("  %-22s - Solid color\n", styleToString(FuzzerStyle::kSolid));
    printf("  %-22s - Linear gradient\n", styleToString(FuzzerStyle::kGradientLinear));
    printf("  %-22s - Radial gradient\n", styleToString(FuzzerStyle::kGradientRadial));
    printf("  %-22s - Conic gradient\n", styleToString(FuzzerStyle::kGradientConic));
    printf("  %-22s - Pattern with aligned translation and no scaling\n", styleToString(FuzzerStyle::kPatternAligned));
    printf("  %-22s - Pattern with fractional translation and no scaling\n", styleToString(FuzzerStyle::kPatternUnaligned));
    printf("  %-22s - Pattern with affine transformation\n", styleToString(FuzzerStyle::kPatternAffine));
    printf("  %-22s - Every render call uses a random style\n", styleToString(FuzzerStyle::kRandom));
    printf("\n");
  }
};

#endif // BLEND2D_TEST_FUZZ_BASEAPP_H_INCLUDED
