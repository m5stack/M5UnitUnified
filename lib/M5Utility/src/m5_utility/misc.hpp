/*!
  @file misc.hpp
  @brief Miscellaneous features

  SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD

  SPDX-License-Identifier: MIT
*/
#ifndef M5_UTILITY_MISC_HPP
#define M5_UTILITY_MISC_HPP

#include <cstdint>

namespace m5 {
namespace utility {

//! @brief Reversing the bit order
inline uint8_t reverseBitOrder(const uint8_t u8) {
#if defined(__clang__) && 0
#pragma message "Using clang builtin"
    return __builtin_bitreverse8(u8);
#else
    uint8_t v{u8};
    v = ((v & 0xF0) >> 4) | ((v & 0x0F) << 4);
    v = ((v & 0xCC) >> 2) | ((v & 0x33) << 2);
    v = ((v & 0xAA) >> 1) | ((v & 0x55) << 1);
    return v;
#endif
}

//! @brief Reversing the bit order
inline uint16_t reverseBitOrder(const uint16_t u16) {
#if defined(__clang__) && 0
#pragma message "Using clang builtin"
    return __builtin_bitreverse16(u16);
#else
    uint16_t v{u16};
    v = ((v & 0xFF00) >> 8) | ((v & 0x00FF) << 8);
    v = ((v & 0xF0F0) >> 4) | ((v & 0x0F0F) << 4);
    v = ((v & 0xCCCC) >> 2) | ((v & 0x3333) << 2);
    v = ((v & 0xAAAA) >> 1) | ((v & 0x5555) << 1);
    return v;
#endif
}

}  // namespace utility
}  // namespace m5
#endif
