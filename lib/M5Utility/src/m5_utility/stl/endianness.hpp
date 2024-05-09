/*!
  @file endianness.hpp
  @brief Compile-time endian identification

  @copyright M5Stack. All rights reserved.
  @license Licensed under the MIT license. See LICENSE file in the project root
  for full license information.
*/
#ifndef M5_UTILITY_STL_ENDIANESS_HPP
#define M5_UTILITY_STL_ENDIANESS_HPP

#if __cplusplus >= 202002L
// #pragma message "Using std::endian"
#include <bit>
#elif __has_include(<endian.h>)
// #pragma message "Using endian.h"
#include <endian.h>
#elif __has_include(<machine/endian.h>)
// #pragma message "Using machine/endian.h"
#include <machine/endian.h>
#else
// #pragma message "Using hacked"
#include <cstdint>
#endif

namespace m5 {
namespace stl {

// C++20 or later
#if __cplusplus >= 202002L || DOXYGEN_PROCESS

using endian = std::endian;

// endian header
#elif __has_include(<endian.h>) || __has_include(<machine/endian.h>)

enum class endian {
#if defined(__BYTE_ORDER)
    little = __LITTLE_ENDIAN,
    big    = __BIG_ENDIAN,
    native = __BYTE_ORDER
#elif defined(_BYTE_ORDER)
    little = _LITTLE_ENDIAN,
    big    = _BIG_ENDIAN,
    native = _BYTE_ORDER
#elif defined(BYTE_ORDER)
    little = LITTLE_ENDIAN,
    big    = BIG_ENDIAN,
    native = BYTE_ORDER
#else
    little = 0,
    big    = 0,
    native = 0,
#endif
};

#else

/// @cond
constexpr uint32_t val32 = 0x11223344;
constexpr uint8_t ref8   = static_cast<const uint8_t&>(val32);
/// @endcond
enum class endian { little = 0x44, big = 0x11, native = ref8 };
#endif
}  // namespace stl

/*!
  @namespace enidan
  @brief endianness detection
 */
namespace endian {
///@name endian type
///@{
constexpr bool little =
    m5::stl::endian::native ==
    m5::stl::endian::little;  //!< @brief true if little endian.
constexpr bool big = m5::stl::endian::native ==
                     m5::stl::endian::big;  //!< @brief true if big endian.
constexpr bool other = !little && !big;     //!< @brief true if other endian.
///@}

static_assert(little || big || other, "Unable to determine endianness");
static_assert(((int)little + (int)big + (int)other) == 1,
              "Endian matches more than one");
}  // namespace endian
}  // namespace m5

#endif
