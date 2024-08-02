/*
 * SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
/*!
  @file unit_ADS1114.cpp
  @brief ADS1114 Unit for M5UnitUnified
*/
#include "unit_ads1114.hpp"
#include <M5Utility.hpp>

using namespace m5::utility::mmh3;
using namespace m5::unit::types;
using namespace m5::unit::ads111x;
using namespace m5::unit::ads111x::command;

namespace m5 {
namespace unit {
// class UnitADS1114
const char UnitADS1114::name[] = "UnitADS1114";
const types::uid_t UnitADS1114::uid{"UnitADS1114"_mmh3};
const types::uid_t UnitADS1114::attr{0};
bool UnitADS1114::on_begin() {
    M5_LIB_LOGV("mux is not support");
    return setSamplingRate(_cfg.rate) && setGain(_cfg.gain) &&
           setComparatorQueue(_cfg.comp_que);
}
}  // namespace unit
}  // namespace m5
