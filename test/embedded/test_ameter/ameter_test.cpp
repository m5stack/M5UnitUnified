/*
  UnitTest for UnitAmeter

  SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD

  SPDX-License-Identifier: MIT
*/
#include "../avmeter_template.hpp"
#include <unit/unit_Ameter.hpp>

const ::testing::Environment* global_fixture =
    ::testing::AddGlobalTestEnvironment(new GlobalFixture<400000U>());

#if 0
INSTANTIATE_TEST_SUITE_P(
    ParamValues, TestADS1115,
    ::testing::Values(TestParams{false, +UnitAmeter::DEFAULT_ADDRESS,
                                 +UnitAmeter::DEFAULT_EEPROM_ADDRESS},
                      TestParams{true, +UnitAmeter::DEFAULT_ADDRESS,
                                 +UnitAmeter::DEFAULT_EEPROM_ADDRESS}));
#endif
INSTANTIATE_TEST_SUITE_P(ParamValues, TestADS1115,
                         ::testing::Values(TestParams{
                             false, +UnitAmeter::DEFAULT_ADDRESS,
                             +UnitAmeter::DEFAULT_EEPROM_ADDRESS}));
