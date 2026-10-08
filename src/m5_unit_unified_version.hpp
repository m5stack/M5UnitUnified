/*
 * SPDX-FileCopyrightText: 2026 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
/*!
  @file m5_unit_unified_version.hpp
  @brief Version of M5UnitUnified, to test at compile time (as ESP_IDF_VERSION)
  @details Keep the values the same as library.properties / library.json / idf_component.yml.
  M5UnitUnified 0.6.0 and older do not have these macros, so test that they are defined first:
  @code
  #if defined(M5_UNIT_UNIFIED_VERSION)
  #if M5_UNIT_UNIFIED_VERSION >= M5_UNIT_UNIFIED_VERSION_VAL(0, 6, 1)
  // Use what 0.6.1 added
  #endif
  #endif
  @endcode
  (The two #if are needed: in one line, an undefined M5_UNIT_UNIFIED_VERSION_VAL() is an error even after a false
  defined() test.)
*/
#ifndef M5_UNIT_UNIFIED_VERSION_HPP
#define M5_UNIT_UNIFIED_VERSION_HPP

#define M5_UNIT_UNIFIED_VERSION_MAJOR 0
#define M5_UNIT_UNIFIED_VERSION_MINOR 6
#define M5_UNIT_UNIFIED_VERSION_PATCH 0

//! @brief Make a version value comparable with M5_UNIT_UNIFIED_VERSION
#define M5_UNIT_UNIFIED_VERSION_VAL(major, minor, patch) (((major) << 16) | ((minor) << 8) | (patch))

//! @brief Current version as a comparable value
#define M5_UNIT_UNIFIED_VERSION                                                               \
    M5_UNIT_UNIFIED_VERSION_VAL(M5_UNIT_UNIFIED_VERSION_MAJOR, M5_UNIT_UNIFIED_VERSION_MINOR, \
                                M5_UNIT_UNIFIED_VERSION_PATCH)

#endif
