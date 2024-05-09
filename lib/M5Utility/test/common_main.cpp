/*!
  main for UnitTest on native

  copyright M5Stack. All rights reserved.
  Licensed under the MIT license. See LICENSE file in the project root for full
  license information.
*/
#include <gtest/gtest.h>

// C++ version
#if __cplusplus >= 202002L
#pragma message "C++20 or later"
#elif __cplusplus >= 201703L
#pragma message "C++17 or later"
#elif __cplusplus >= 201402L
#pragma message "C++14 or later"
#elif __cplusplus >= 201103L
#pragma message "C++11 or later"
#else
#error "Need C++11 or later"
#endif
// Compiler
#if defined(__clang__)
#pragma message "Clang"
#elif defined(_MSC_VER)
#pragma message "MSVC"
#elif defined(__BORLANDC__)
#pragma message "BORANDC"
#elif defined(__MINGW32__) || defined(__MINGW64__)
#pragma message "MINGW"
#elif defined(__INTEL_COMPILER)
#pragma message "ICC"
#elif defined(__GNUG__)
#pragma message "GCC"
#else
#pragma message "Unknown compiler"
#endif

/*
  For native test, this main() is used.
  For embedded test, overwrite by main() in library.
*/
__attribute__((weak)) int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);

#ifdef GTEST_FILTER
    ::testing::GTEST_FLAG(filter) = GTEST_FILTER;
#endif

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-result"
    RUN_ALL_TESTS();
#pragma GCC diagnostic pop
    // Always return zero-code and allow PlatformIO to parse results
    return 0;
}
