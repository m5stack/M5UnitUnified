/*
 * SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
/*!
  @file unit_ADS1113.cpp
  @brief ADS1113 Unit for M5UnitUnified
*/
#include "unit_ads1113.hpp"
#include <M5Utility.hpp>

using namespace m5::utility::mmh3;
using namespace m5::unit::types;
using namespace m5::unit::ads111x;
using namespace m5::unit::ads111x::command;

namespace m5 {
namespace unit {
// class UnitADS1113
const char UnitADS1113::name[] = "UnitADS1113";
const types::uid_t UnitADS1113::uid{"UnitADS1113"_mmh3};
const types::uid_t UnitADS1113::attr{0};
bool UnitADS1113::on_begin() {
    M5_LIB_LOGV("mux, gain, and comp_que  not support");
    return setSamplingRate(_cfg.rate);
}
}  // namespace unit
}  // namespace m5
