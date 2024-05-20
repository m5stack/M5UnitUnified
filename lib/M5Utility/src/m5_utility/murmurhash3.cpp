/*!
  @file murmurhash3.cpp
  @brief MurmurHash3

  SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD

  SPDX-License-Identifier: MIT
*/
#include "murmurhash3.hpp"

namespace m5 {
namespace utility {
namespace mmh3 {

uint32_t calculate(const char* str) {
    auto len = strlen(str);
    return finalize(rest(str + ((len >> 2) * sizeof(uint32_t)), (len & 3),
                         group_of_4(str, len >> 2)),
                    len);
}

}  // namespace mmh3
}  // namespace utility
}  // namespace m5
