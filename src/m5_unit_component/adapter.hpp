/*
 * SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
/*!
  @file adapter.hpp
  @brief Adapters to treat M5HAL and any connection in the same way
  @note  Currently handles Arduino directly, but will handle via M5HAL in the future
*/
#ifndef M5_UNIT_COMPONENT_ADAPTER_HPP
#define M5_UNIT_COMPONENT_ADAPTER_HPP
#include "adapter_base.hpp"
#include "adapter_i2c.hpp"
#include "identify_functions.hpp"
#if defined(M5_UNIT_UNIFIED_USING_RMT_V2)
#include "adapter_gpio_v2.hpp"
#else
#include "adapter_gpio_v1.hpp"
#endif
#endif
