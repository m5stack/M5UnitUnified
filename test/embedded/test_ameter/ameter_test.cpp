/*
  UnitTest for UnitAmeter

  SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD

  SPDX-License-Identifier: MIT
*/
#include "../avmeter_template.hpp"
#include <unit/unit_Ameter.hpp>

using namespace m5::unit;

INSTANTIATE_TEST_SUITE_P(
    ParamValues, TestADS1115,
    ::testing::Values(TestParams{true, +UnitAmeter::DEFAULT_ADDRESS,
                                 +UnitAmeter::DEFAULT_EEPROM_ADDRESS},
                      TestParams{false, +UnitAmeter::DEFAULT_ADDRESS,
                                 +UnitAmeter::DEFAULT_EEPROM_ADDRESS}));
