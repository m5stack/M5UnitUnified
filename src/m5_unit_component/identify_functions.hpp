/*
 * SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
/*!
  @file identify_functions.hpp
  @brief Identification of functions to be used
*/
#ifndef M5_UNIT_UNIFIED_IDENTIFY_FUNCTIONS_HPP
#define M5_UNIT_UNIFIED_IDENTIFY_FUNCTIONS_HPP

// Detect ESP-IDF version
#if __has_include(<esp_idf_version.h>)
#include <esp_idf_version.h>
#else  // esp_idf_version.h has been introduced in Arduino 1.0.5 (ESP-IDF3.3)
#define ESP_IDF_VERSION_VAL(major, minor, patch) ((major << 16) | (minor << 8) | (patch))
#define ESP_IDF_VERSION                          ESP_IDF_VERSION_VAL(3, 2, 0)
#endif

// RMT
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 0, 0)
#define M5_UNIT_UNIFIED_USING_RMT_V2
#define M5_UNIT_UNIFIED_USING_ADC_ONESHOT
#endif

#endif
