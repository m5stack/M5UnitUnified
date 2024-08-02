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

// For UnitVmeter-specific testing
class TestVmeter : public ComponentTestBase<UnitVmeter, bool> {
   protected:
    virtual UnitVmeter* get_instance() override {
        auto ptr = new m5::unit::UnitVmeter();
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

// INSTANTIATE_TEST_SUITE_P(ParamValues, TestVmeter,
//                          ::testing::Values(false, true));
//  INSTANTIATE_TEST_SUITE_P(ParamValues, TestVmeter, ::testing::Values(true));
INSTANTIATE_TEST_SUITE_P(ParamValues, TestVmeter, ::testing::Values(false));

TEST_P(TestVmeter, Correction) {
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
