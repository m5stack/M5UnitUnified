/*
  UnitTest for UnitVmeter

  SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD

  SPDX-License-Identifier: MIT
*/
#include "../avmeter_template.hpp"
#include <unit/unit_Vmeter.hpp>

using namespace m5::unit;

INSTANTIATE_TEST_SUITE_P(
    ParamValues, TestADS1115,
    ::testing::Values(TestParams{true, +UnitVmeter::DEFAULT_ADDRESS,
                                 +UnitVmeter::DEFAULT_EEPROM_ADDRESS},
                      TestParams{false, +UnitVmeter::DEFAULT_ADDRESS,
                                 +UnitVmeter::DEFAULT_EEPROM_ADDRESS}));
