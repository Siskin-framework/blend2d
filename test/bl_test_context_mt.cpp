// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <blend2d.h>
#include <cmath>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bl_test_cmdline.h"
#include "bl_test_context_baseapp.h"
#include "bl_test_context_utilities.h"
#include "bl_test_imageutils.h"

namespace ContextTests {

class MTTestApp : public BaseTestApp {
public:
  uint64_t mismatchCount {};
  uint32_t failedCount {};
  uint32_t passedCount {};

  MTTestApp() : BaseTestApp() {
    defaultOptions.threadCount = 2u;
  }

  int help() {
    using StringUtils::boolToString;

    printf("Usage:\n");
    printf("  bl_test_context_mt [options] [--help for help]\n");
    printf("\n");

    printf("Purpose:\n");
    printf("  Multi-threaded rendering context tester is designed to verify whether both\n");
    printf("  single-threaded and multi-threaded rendering contexts yield pixel identical\n");
    printf("  output when used with the same input data.\n");
    printf("\n");

    printCommonOptions(defaultOptions);

    printf("Multithreading Options:\n");
    printf("  --flush-sync            - Do occasional syncs between calls [default=%s]\n", boolToString(defaultOptions.flushSync));
    printf("  --thread-count=<uint>   - Number of threads of MT context   [default=%u]\n", defaultOptions.threadCount);
    printf("\n");

    printCommands();
    printStyles();
    printCompOps();
    printOpacityOps();
    printFormats();

    fflush(stdout);
    return 0;
  }

  bool parseMTOptions(CmdLine cmdLine) {
    options.flushSync = cmdLine.hasArg("--flush-sync") || defaultOptions.flushSync;
    options.threadCount = cmdLine.valueAsUInt("--thread-count", defaultOptions.threadCount);

    return true;
  }

  int run(CmdLine cmdLine) {
    printAppInfo("Blend2D Multi-Threaded Rendering Context Tester");

    if (cmdLine.hasArg("--help"))
      return help();

    if (!parseCommonOptions(cmdLine) || !parseMTOptions(cmdLine))
      return 1;

    ContextTester aTester("st");
    ContextTester bTester("mt");

    aTester.setStyle(options.style);
    bTester.setStyle(options.style);

    aTester.setFlushSync(options.flushSync);
    bTester.setFlushSync(options.flushSync);

    BLContextCreateInfo aCreateInfo {};
    BLContextCreateInfo bCreateInfo {};

    bCreateInfo.threadCount = options.threadCount;

    if (aTester.init(int(options.width), int(options.height), options.format, aCreateInfo) != BL_SUCCESS ||
        bTester.init(int(options.width), int(options.height), options.format, bCreateInfo) != BL_SUCCESS) {
      printf("Failed to initialize rendering contexts\n");
      return 1;
    }

    BLString testName;
    dispatchRuns([&](CommandId commandId, CompOp compOp, OpacityOp opacityOp) {
      printf("Testing [%s | %s | %s | %s]:\n",
        StringUtils::commandIdToString(commandId),
        StringUtils::compOpToString(compOp),
        StringUtils::opacityOpToString(opacityOp),
        StringUtils::styleIdToString(options.style));

      testName.assignFormat("%s-%s-%s-%s",
        StringUtils::commandIdToString(commandId),
        StringUtils::compOpToString(compOp),
        StringUtils::opacityOpToString(opacityOp),
        StringUtils::styleIdToString(options.style));

      aTester.setCompOp(compOp);
      bTester.setCompOp(compOp);

      aTester.setOpacityOp(opacityOp);
      bTester.setOpacityOp(opacityOp);

      if (runMultiple(commandId, testName.data(), aTester, bTester, 0))
        passedCount++;
      else
        failedCount++;
    });

    aTester.reset();
    bTester.reset();

    printf("Testing finished...\n");

    if (mismatchCount)
      printf("Found %llu mismatches!\n", (unsigned long long)mismatchCount);
    else
      printf("No mismatches found!\n");

    return mismatchCount ? 1 : 0;
  }
};

} // {ContextTests}

int main(int argc, char* argv[]) {
  BLRuntimeScope rtScope;
  ContextTests::MTTestApp app;

  return app.run(CmdLine(argc, argv));
}
