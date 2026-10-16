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
target_include_directories(m5unitunified_native_check PRIVATE "${M5UU_DIR}/src")
# SYSTEM: warnings raised inside the dependencies are not actionable here
target_include_directories(m5unitunified_native_check SYSTEM PRIVATE
  "${M5HAL_DIR}/src" "${M5UTILITY_DIR}/src")

# Warnings for this library only. M5HAL / M5Utility are built here as well, and
# their warnings are not actionable from this repository.
if(MSVC)
  set_source_files_properties(${m5uu_sources} PROPERTIES COMPILE_OPTIONS "/W4")
else()
  set_source_files_properties(${m5uu_sources} PROPERTIES COMPILE_OPTIONS "-Wall;-Wextra")
endif()

# Without this MSVC reports __cplusplus as 199711L, so the C++ version checks in
# M5Utility (std::optional / std::expected and the like) would select a
# different configuration than the other compilers.
if(MSVC)
  target_compile_options(m5unitunified_native_check PRIVATE /Zc:__cplusplus)
  set(wiring_gate_flags "/Zc:__cplusplus")
else()
  set(wiring_gate_flags "")
endif()

# The wiring helpers are declared only when <M5Unified.h> is included before the
# wiring header (README). Compile only, at configure time: control.cpp must
# compile, and the other two must fail on the missing helper itself.
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
function(wiring_gate_check name expect_ok)
  try_compile(gate_ok "${CMAKE_BINARY_DIR}/wiring_gate/${name}"
    SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/wiring_gate/${name}.cpp"
    CMAKE_FLAGS "-DINCLUDE_DIRECTORIES=${M5UU_DIR}/src;${M5HAL_DIR}/src;${M5UTILITY_DIR}/src"
    COMPILE_DEFINITIONS ${wiring_gate_flags}
    CXX_STANDARD 11
    CXX_STANDARD_REQUIRED ON
    OUTPUT_VARIABLE gate_out)
  if(expect_ok AND NOT gate_ok)
    message(FATAL_ERROR "wiring gate: ${name}.cpp must compile\n${gate_out}")
  endif()
  if(NOT expect_ok)
    if(gate_ok)
      message(FATAL_ERROR "wiring gate: ${name}.cpp must not compile (the wiring helpers are visible)")
    endif()
    # GCC: ‘i2cPins’ is not a member of ..., MSVC: 'i2cPins': is not a member of ...,
    # Clang: no member named 'i2cPins'. GCC quotes with Unicode quotes, so match any quote
    if(NOT gate_out MATCHES "i2cPins[^ ]* is not a member|no member named [^ ]*i2cPins")
      message(FATAL_ERROR "wiring gate: ${name}.cpp failed for another reason\n${gate_out}")
    endif()
  endif()
  message(STATUS "wiring gate: ${name} OK")
endfunction()
wiring_gate_check(control TRUE)
wiring_gate_check(without_m5unified FALSE)
wiring_gate_check(m5unified_after FALSE)
