/*
 * SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
/*!
  @file types.hpp
  @brief Type and enumerator definitions
*/
#ifndef M5_UNIT_COMPONENT_TYPES_HPP
#define M5_UNIT_COMPONENT_TYPES_HPP

#include <cstdint>

namespace m5 {
namespace unit {
/*!
  @namespace types
  @brief Type and enumerator definitions
 */
namespace types {

using uid_t          = uint32_t;       //!< @brief Component unique identifier
using attr_t         = uint32_t;       //!< @brief Component attribute bits
using elapsed_time_t = unsigned long;  //!< @brief Elapsed time unit (ms)

}  // namespace types
}  // namespace unit
}  // namespace m5
#endif
