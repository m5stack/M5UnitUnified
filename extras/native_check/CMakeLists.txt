# Host (native) build check for M5UnitUnified. Not part of the library package.
#
# PlatformIO's native platform always uses GCC, so this CMake project is the
# only way to reach MSVC. Run from the repository root:
#
#   cmake -S extras/native_check -B build/native_check
#   cmake --build build/native_check --config Release
#
# M5HAL / M5Utility are expected next to this repository (the layout used by
# the CI checkout and by a local libraries/ directory). Override with
# -DM5HAL_DIR=... / -DM5UTILITY_DIR=... when they live elsewhere.
cmake_minimum_required(VERSION 3.16)
project(m5unitunified_native_check CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

get_filename_component(M5UU_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../.." ABSOLUTE)
set(M5HAL_DIR "${M5UU_DIR}/../M5HAL" CACHE PATH "M5HAL checkout")
set(M5UTILITY_DIR "${M5UU_DIR}/../M5Utility" CACHE PATH "M5Utility checkout")

file(GLOB_RECURSE m5uu_sources "${M5UU_DIR}/src/*.cpp")
# Test scaffolding pulls in <M5Unified.h>; not part of the library
list(FILTER m5uu_sources EXCLUDE REGEX "/googletest/")
file(GLOB_RECURSE m5hal_sources "${M5HAL_DIR}/src/*.cpp")
file(GLOB_RECURSE m5utility_sources "${M5UTILITY_DIR}/src/*.cpp")

add_executable(m5unitunified_native_check main.cpp ${m5uu_sources} ${m5hal_sources} ${m5utility_sources})
target_include_directories(m5unitunified_native_check PRIVATE
  "${M5UU_DIR}/src" "${M5HAL_DIR}/src" "${M5UTILITY_DIR}/src")

# Without this MSVC reports __cplusplus as 199711L, so the C++ version checks in
# M5Utility (std::optional / std::expected and the like) would select a
# different configuration than the other compilers.
if(MSVC)
  target_compile_options(m5unitunified_native_check PRIVATE /Zc:__cplusplus)
endif()
