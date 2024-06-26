#!/bin/sh

CURRENT_DIR="`pwd`"
BUILD_DIR="${CURRENT_DIR}/../build"
BUILD_OPTIONS="-DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DBLEND2D_TEST=1"

if [ -n "${ASMJIT_DIR}" ]; then
  BUILD_OPTIONS="${BUILD_OPTIONS} -DASMJIT_DIR=\"${ASMJIT_DIR}\""
fi

echo "== [Configuring Build - Release_ASAN] =="
eval cmake "${CURRENT_DIR}/.." -B "${BUILD_DIR}/Release_ASAN" ${BUILD_OPTIONS} -DCMAKE_BUILD_TYPE=Release -DBLEND2D_SANITIZE=address
echo ""

echo "== [Configuring Build - Release_UBSAN] =="
eval cmake "${CURRENT_DIR}/.." -B "${BUILD_DIR}/Release_UBSAN" ${BUILD_OPTIONS} -DCMAKE_BUILD_TYPE=Release -DBLEND2D_SANITIZE=undefined
echo ""

echo "== [Configuring Build - Release_MSAN] =="
eval cmake "${CURRENT_DIR}/.." -B "${BUILD_DIR}/Release_MSAN" ${BUILD_OPTIONS} -DCMAKE_BUILD_TYPE=Release -DBLEND2D_SANITIZE=memory -DBLEND2D_NO_JIT=ON
echo ""

echo "== [Configuring Build - Release_TSAN] =="
eval cmake "${CURRENT_DIR}/.." -B "${BUILD_DIR}/Release_TSAN" ${BUILD_OPTIONS} -DCMAKE_BUILD_TYPE=Release -DBLEND2D_SANITIZE=thread
echo ""
