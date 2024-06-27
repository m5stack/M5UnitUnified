/*
 * SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
/*
  UnitTest for UnitVmeter
*/
#include "../avmeter_template.hpp"
#include <unit/unit_Vmeter.hpp>

const ::testing::Environment* global_fixture =
    ::testing::AddGlobalTestEnvironment(new GlobalFixture<400000U>());

#if 0
INSTANTIATE_TEST_SUITE_P(
    ParamValues, TestADS1115,
    ::testing::Values(TestParams{false, +UnitVmeter::DEFAULT_ADDRESS,
                                 +UnitVmeter::DEFAULT_EEPROM_ADDRESS},
                      TestParams{true, +UnitVmeter::DEFAULT_ADDRESS,
                                 +UnitVmeter::DEFAULT_EEPROM_ADDRESS}));
#endif
INSTANTIATE_TEST_SUITE_P(ParamValues, TestADS1115,
                         ::testing::Values(TestParams{
                             false, +UnitVmeter::DEFAULT_ADDRESS,
                             +UnitVmeter::DEFAULT_EEPROM_ADDRESS}));
