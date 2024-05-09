/*!
  @file string.hpp
  @brief Utilities for string

  @copyright M5Stack. All rights reserved.
  @license Licensed under the MIT license. See LICENSE file in the project root
  for full license information.
*/
#ifndef M5_UTILITY_STRING_HPP
#define M5_UTILITY_STRING_HPP

#include <string>

namespace m5 {
namespace utility {

//! @ brief Create a string in a format similar to printf
std::string formatString(const char* fmt, ...);

///@name Trim
///@warning The string entered will be changed
///@{
/*! @brief Trim right */
std::string& trimRight(std::string& s);
//! @brief Trim left
std::string& trimLeft(std::string& s);
//! @brief Trim both ends
std::string& trim(std::string& s);
///@}

///@name Convert
///@{
/*! @brief Convert from 0~15 to hexadecimal character */
inline char uintToHex(const uint8_t v)
{
    return v < 10 ? '0' + v : 'A' + (v - 10);
}

/*!
  @brief Convert any one unsigned integer to a hexadecimal string
  @tparam T Value type (Must be unsigned integer)
 */
template <typename T>
std::string unsignedToHex(const T& v) {
    static_assert(std::is_integral<T>::value && std::is_unsigned<T>::value,
                  "T must be unsigned integer");
    std::string s;
    for (size_t i = sizeof(T); i > 0; --i) {
        uint8_t u8 = (v >> ((i - 1) * 8)) & 0xFF;
        s += uintToHex(u8 & 0x0F);
        s += uintToHex((u8 >> 4) & 0x0F);
    }
    return s;
}
///@}

}  // namespace utility
}  // namespace m5
#endif