/*
 * SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
/*!
  @file adapter.cpp
  @brief Adapters to treat M5HAL and any connection in the same way
  @note  Currently handles Arduino directly, but will handle via M5HAL in the future
*/
#include "adapter.hpp"
#include <cassert>
#if defined(ARDUINO)
#include <Wire.h>
#endif
#include <M5HAL.hpp>
#include <M5Utility.hpp>
#include <soc/gpio_struct.h>
#include <soc/gpio_sig_map.h>

namespace m5 {
namespace unit {

// Adapter

}  // namespace unit
}  // namespace m5
