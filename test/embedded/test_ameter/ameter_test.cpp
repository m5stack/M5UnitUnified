/*
 * SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
/*
  UnitTest for UnitAmeter
*/
#include "../avmeter_template.hpp"
#include <unit/unit_Ameter.hpp>

using namespace m5::unit;
using namespace m5::unit::googletest;
using namespace m5::unit::ads111x;

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

// For UnitAmeter-specific testing
class TestAmeter : public ComponentTestBase<UnitAmeter, bool> {
   protected:
    virtual UnitAmeter* get_instance() override {
        auto ptr = new m5::unit::UnitAmeter();
        if (ptr) {
            auto cfg = ptr->config();
            ptr->config(cfg);
        }
        return ptr;
    }
    virtual bool is_using_hal() const override {
        return GetParam();
    };
};

// INSTANTIATE_TEST_SUITE_P(ParamValues, TestAmeter,
//                          ::testing::Values(false, true));
//  INSTANTIATE_TEST_SUITE_P(ParamValues, TestAmeter, ::testing::Values(true));
INSTANTIATE_TEST_SUITE_P(ParamValues, TestAmeter, ::testing::Values(false));

TEST_P(TestAmeter, Correction) {
    constexpr Gain gain_table[] = {
        Gain::PGA_6144, Gain::PGA_4096, Gain::PGA_2048,
        Gain::PGA_1024, Gain::PGA_512,  Gain::PGA_256,
    };

    float prev = unit->correction();
    EXPECT_TRUE(std::isfinite(prev));

    for (auto&& e : gain_table) {
        EXPECT_TRUE(unit->setGain(e));
        auto now = unit->correction();

        EXPECT_TRUE(std::isfinite(now));
        EXPECT_NE(now, prev);
    }
}
