/*!
  @file conversion.hpp
  @brief Numeric conversion

  SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD

  SPDX-License-Identifier: MIT
*/
#ifndef M5_UTILITY_CONVERSION_HPP
#define M5_UTILITY_CONVERSION_HPP

#include <type_traits>
#include <cstdint>
#include <cstddef>

namespace m5 {
namespace utility {

/*!
  @brief Convert an unsigned integer of any maximum number of bits to a signed
  integer
  @tparam Bits Number of bits assumed by value
  @code {.cpp}
  uint32_t u24{0x00FFFFFF};
  // 24 bit unsigned int to int32_t
  uint32_t s32 = unsigned_to_signed<24>(u24);
  // s32 is -1 (Not 16777215)
  @endcode
 */
template <size_t Bits, typename T>
constexpr auto unsigned_to_signed(const T v) ->
    typename std::make_signed<T>::type {
    static_assert(std::is_integral<T>::value && std::is_unsigned<T>::value,
                  "T must be an unsigned integer");
    static_assert(Bits <= sizeof(T) * 8,
                  "Bits must be less than or equal to the number of bits in T");

    using S = typename std::make_signed<T>::type;
    return static_cast<S>((v & (1ULL << (Bits - 1)))
                              ? (v & ((1ULL << Bits) - 1)) - (1ULL << Bits)
                              : (v & ((1ULL << Bits) - 1)));
}

}  // namespace utility
}  // namespace m5
#endif
