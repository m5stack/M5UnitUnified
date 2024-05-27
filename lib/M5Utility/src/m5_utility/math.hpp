/*!
  @file math.hpp
  @brief Maths-related

  SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD

  SPDX-License-Identifier: MIT
*/
#ifndef M5_UTILITY_MATH_HPP
#define M5_UTILITY_MATH_HPP

#include <cstdint>
#include <type_traits>

namespace m5 {
namespace math {
template <typename T>

/*!
  @brief Is value power of 2?
  @tparam Type of the value
  @param v Value
  @return True if value is power of 2
 */
inline constexpr bool is_powerof2(const T v) {
    static_assert(std::is_integral<T>::value,
                  "The argument v is only an integer value.");
    return v > 0 && ((v & (v - 1)) == 0);
}

}  // namespace math
}  // namespace m5
#endif
