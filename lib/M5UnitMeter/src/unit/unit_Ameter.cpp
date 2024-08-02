/*
 * SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
/*!
  @file unit_Ameter.cpp
  @brief Ameter (ADS1115 + CA-IS3020S) Unit for M5UnitUnified
*/
#include "unit_Ameter.hpp"
#include <M5Utility.hpp>
namespace m5 {
namespace unit {

using namespace m5::utility::mmh3;
using namespace ads111x;
using namespace ads111x::command;

const char UnitAmeter::name[] = "UnitAmeter";
const types::uid_t UnitAmeter::uid{"UnitAmeter"_mmh3};
const types::uid_t UnitAmeter::attr{0};

void UnitAmeter::apply_coefficient(const ads111x::Gain gain) {
    UnitADS111x::apply_coefficient(gain);
    _correction = resolution() * calibrationFactor();
}

}  // namespace unit
}  // namespace m5
