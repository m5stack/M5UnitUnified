/*!
  @file stl_extension.hpp
  @brief STL extensions

  Add features that cannot be used depending on the C++ version, etc.

  @copyright M5Stack. All rights reserved.
  @license Licensed under the MIT license. See LICENSE file in the project root
  for full license information.
*/
#ifndef M5_UTILITY_STL_EXTENSION_HPP
#define M5_UTILITY_STL_EXTENSION_HPP

#include <cstddef>
#include <type_traits>

namespace m5 {
namespace stl {

/*! @brief Like std::size  C++17 or later.(for container) */
template <class C>
constexpr auto size(const C& c) -> decltype(c.size()) {
    return c.size();
}

/*! @brief Like std::size C++17 or later.(for raw array) */
template <typename T, size_t N>
constexpr auto size(const T (&)[N]) noexcept -> size_t {
    return N;
}

/*!
  @brief Converts an enumeration to its underlying type.(Like std::to_underlying
  C++23 or later)
  @tparam E Type of enum
  @param e Enumeration value to convert
  @return The integer value of the underlying type of Enum, converted from e.
*/
template <typename E>
constexpr inline typename std::underlying_type<E>::type to_underlying(
    const E e) noexcept {
    return static_cast<typename std::underlying_type<E>::type>(e);
}

}  // namespace stl
}  // namespace m5

#endif
