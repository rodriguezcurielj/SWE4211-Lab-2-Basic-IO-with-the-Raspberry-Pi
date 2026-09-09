#!/bin/sh
cmake ../src -G "Eclipse CDT4 - Unix Makefiles" \
  -DCMAKE_ECLIPSE_VERSION=4.40 \
  -DCMAKE_ECLIPSE_GENERATE_SOURCE_PROJECT=TRUE \
  -DCMAKE_ECLIPSE_MAKE_ARGUMENTS=-j4 \
  -DCMAKE_TOOLCHAIN_FILE=../Toolchain-rpi.cmake \
  -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
  -Wno-dev -Wno-deprecated

