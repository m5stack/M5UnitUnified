/*!
  @file compatibility_feature.cpp
  @brief Maintain compatibility with Arduino API, etc.

  SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD

  SPDX-License-Identifier: MIT
*/
#include "compatibility_feature.hpp"
#include <ctime>
#include <thread>

namespace {
using clock                      = std::chrono::high_resolution_clock;
const clock::time_point start_at = clock::now();

}  // namespace

namespace m5 {
namespace utility {

//! @weak
__attribute__((weak)) unsigned long millis() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(clock::now() -
                                                                 start_at)
        .count();
}
//! @weak
__attribute__((weak)) unsigned long micros() {
    return std::chrono::duration_cast<std::chrono::microseconds>(clock::now() -
                                                                 start_at)
        .count();
}
//! @weak
__attribute__((weak)) void delay(const unsigned long ms) {
#if 0
    auto abst = clock::now() + std::chrono::milliseconds(ms);
    std::this_thread::sleep_until(abst);
#else
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
#endif
}

//! @weak
__attribute__((weak)) void delayMicroseconds(const unsigned int us) {
#if 0
    auto abst = clock::now() + std::chrono::microseconds(us);
    std::this_thread::sleep_until(abst);
#else
    std::this_thread::sleep_for(std::chrono::microseconds(us));
#endif
}

}  // namespace utility
}  // namespace m5
