/*!
  @file murmurhash3.hpp
  @brief MurmurHash3

  MurmurHash (public domain) by Austin Appleby in 2008
  @sa https://en.wikipedia.org/wiki/MurmurHash

  SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD

  SPDX-License-Identifier: MIT
*/
#ifndef M5_UTILITY_MURMURHASH3_HPP
#define M5_UTILITY_MURMURHASH3_HPP

#include <cstdint>
#include <cstddef>
#include <cstring>
#include <type_traits>
#include "./stl/endianness.hpp"

namespace m5 {
namespace utility {
/*!
  @namespace mmh3
  @brief For Murmurhash3
 */
namespace mmh3 {

/// @cond
constexpr uint32_t mul(const uint32_t v, const uint32_t vv) {
    return v * vv;
}

constexpr uint32_t xor_value(const uint32_t v, const uint32_t x) {
    return v ^ x;
}

constexpr uint32_t shift_right(const uint32_t v, const uint8_t s) {
    return v >> s;
}

constexpr uint32_t xor_by_shift_right(const uint32_t v, const uint32_t s) {
    return xor_value(v, shift_right(v, s));
}

constexpr uint32_t str2uint32_little(const char* str,
                                     const size_t getc = sizeof(uint32_t),
                                     uint32_t v        = 0) {
    return getc ? str2uint32_little(str, getc - 1, (v << 8) | str[getc - 1])
                : v;
}

constexpr uint32_t str2uint32_big(const char* str,
                                  const size_t getc = sizeof(uint32_t),
                                  uint32_t v        = 0) {
    return getc ? str2uint32_big(str + 1, getc - 1, (v << 8) | *str) : v;
}

// Switch between little and big enfian.
template <uint32_t Endian>
constexpr typename std::enable_if<Endian, uint32_t>::type str2uint32(
    const char* str, const size_t getc = sizeof(uint32_t), uint32_t v = 0) {
    return str2uint32_little(str, getc, v);
}

template <uint32_t Endian>
constexpr typename std::enable_if<!Endian, uint32_t>::type str2uint32(
    const char* str, const size_t getc = sizeof(uint32_t), uint32_t v = 0) {
    return str2uint32_big(str, getc, v);
}

constexpr uint32_t scramble_sub(const uint32_t k) {
    return (k << 15) | (k >> 17);
}

constexpr uint32_t scramble(const uint32_t k, const uint32_t h) {
    return h ^ mul(scramble_sub(mul(k, 0xcc9e2d51)), 0x1b873593);
}

constexpr uint32_t group_of_4_sub_3(const uint32_t h) {
    return h * 5 + 0xe6546b64;
}

constexpr uint32_t group_of_4_sub_2(const uint32_t h) {
    return (h << 13) | (h >> 19);
}

constexpr uint32_t group_of_4_sub_1(const uint32_t k, const uint32_t h) {
    return group_of_4_sub_3(group_of_4_sub_2(scramble(k, h)));
}

constexpr uint32_t group_of_4(const char* str, const size_t len,
                              const uint32_t h = 0) {
    return len ? group_of_4(
                     str + sizeof(uint32_t), len - 1,
                     group_of_4_sub_1(str2uint32<m5::endian::little>(str), h))
               : h;
}

constexpr uint32_t rest(const char* str, const size_t len,
                        const uint32_t h = 0) {
    return len ? scramble(str2uint32<m5::endian::little>(str, len), h) : h;
}

constexpr uint32_t finalize(uint32_t h, size_t len) {
    return xor_by_shift_right(
        mul(xor_by_shift_right(
                mul(xor_by_shift_right(xor_value(h, len), 16), 0x85ebca6b), 13),
            0xc2b2ae35),
        16);
}
/// @endcond

/*!
  @brief MurmurHash3 by compile-time calculation
  @param str String
  @param len Length of string
  @return 32bit MurmurHash3 from input string
*/
constexpr uint32_t calculate(const char* str, const size_t len) {
    return finalize(rest(str + ((len >> 2) * sizeof(uint32_t)), (len & 3),
                         group_of_4(str, len >> 2)),
                    len);
}

/*!
  @brief User-defined literals "_mmh3"
  @return 32bit MurmurHash3 from input string
  @code
  using namespace m5::unit::mmh3;
  uint32_t h = "M5 Stack"_mmh3;
  @endcode
*/
constexpr uint32_t operator"" _mmh3(const char* str, const size_t len) {
    return calculate(str, len);
};

/*!
  @brief Calculate MurmurHash3 from string
  @param str String
  @return 32bit MurmurHash3 from input string
*/
uint32_t calculate(const char* str);

}  // namespace mmh3
}  // namespace utility
}  // namespace m5
#endif
