# ---------------------------------------------------------------------------
# Cross-compilation toolchain: x86-64 Debian host -> Raspberry Pi (aarch64)
#
# Usage:  cmake ../src -DCMAKE_TOOLCHAIN_FILE=../Toolchain-rpi.cmake
#
# NOTE: values below are consumed during compiler detection at project() time
# and cached. After editing this file you MUST delete the cache:
#     rm -rf CMakeCache.txt CMakeFiles
# ---------------------------------------------------------------------------

# Single place to change if the sysroot ever moves.
set(RPI_SYSROOT /rpi_sysroot)

# --- Target system description ---------------------------------------------
set(CMAKE_SYSTEM_NAME      Linux)
set(CMAKE_SYSTEM_VERSION   1)
set(CMAKE_SYSTEM_PROCESSOR aarch64)

# --- Cross compiler locations ----------------------------------------------
set(CMAKE_C_COMPILER   /usr/bin/arm64-gcc)
set(CMAKE_CXX_COMPILER /usr/bin/arm64-g++)

# --- Sysroot ---------------------------------------------------------------
set(CMAKE_SYSROOT ${RPI_SYSROOT})

# Roots searched by find_library()/find_path(). CMake prepends CMAKE_SYSROOT
# automatically, but listing it explicitly keeps the intent obvious.
set(CMAKE_FIND_ROOT_PATH ${RPI_SYSROOT} /usr/aarch64-linux-gnu/)

# Run host programs, but only ever link target libraries / include target headers.
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# --- Eclipse CDT indexer support -------------------------------------------
# The "Eclipse CDT4" extra generator discovers the indexer's include paths by
# running the compiler itself, but it does NOT forward --sysroot. It forwards
# only COMPILER_ARG1 plus any -std=/-stdlib= found in CMAKE_CXX_FLAGS. Without
# the two lines below, Eclipse indexes against the compiler's default headers
# instead of the Pi's, so symbols like getpriority() appear unresolved even
# though the build succeeds. The duplicate --sysroot on the compile line
# (CMAKE_SYSROOT already adds one) is harmless.
set(CMAKE_C_COMPILER_ARG1   "--sysroot=${RPI_SYSROOT}")
set(CMAKE_CXX_COMPILER_ARG1 "--sysroot=${RPI_SYSROOT}")

# The detector reads -std= from CMAKE_CXX_FLAGS specifically; add_definitions()
# does not land there. Without this the indexer assumes C++17 while the build
# uses C++11, which produces phantom errors in <thread>, <chrono>, etc.
set(CMAKE_C_FLAGS_INIT   "-std=gnu11")
set(CMAKE_CXX_FLAGS_INIT "-std=c++11")

# Optional belt-and-braces: the detector is wrapped in
# "if (NOT CMAKE_EXTRA_GENERATOR_..._SYSTEM_INCLUDE_DIRS)", so preseeding these
# skips the probe entirely and makes the result deterministic across machines.
# Uncomment after confirming the real paths with:
#   arm64-g++ --sysroot=/rpi_sysroot -E -x c++ -v /dev/null 2>&1 \
#       | sed -n '/search starts here/,/End of search/p'
#
# set(CMAKE_EXTRA_GENERATOR_C_SYSTEM_INCLUDE_DIRS
#     "${RPI_SYSROOT}/usr/local/include;${RPI_SYSROOT}/usr/include"
#     CACHE INTERNAL "C compiler system include directories")
# set(CMAKE_EXTRA_GENERATOR_CXX_SYSTEM_INCLUDE_DIRS
#     "${RPI_SYSROOT}/usr/include/c++/14;${RPI_SYSROOT}/usr/include/aarch64-linux-gnu/c++/14;${RPI_SYSROOT}/usr/include/c++/14/backward;${RPI_SYSROOT}/usr/local/include;${RPI_SYSROOT}/usr/include"
#     CACHE INTERNAL "CXX compiler system include directories")

# --- Project module path ----------------------------------------------------
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/..")



#OLD Follows
# Define our host system
#SET(CMAKE_SYSTEM_NAME Linux)
#SET(CMAKE_SYSTEM_VERSION 1)
# Define the cross compiler locations
#SET(CMAKE_C_COMPILER   /usr/bin/arm64-gcc)
#SET(CMAKE_CXX_COMPILER /usr/bin/arm64-g++)

# Define the sysroot path for the RaspberryPi distribution in our tools folder 
#SET(CMAKE_FIND_ROOT_PATH /usr/aarch64-linux-gnu/)
#set(CMAKE_SYSROOT /rpi_sysroot)

# Use our definitions for compiler tools
#SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)



# Search for libraries and headers in the target directories only
#SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
#SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

#list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/..")

  

