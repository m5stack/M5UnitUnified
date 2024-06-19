/*
  UnitTest for UnitPAJ7620U2

  SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD

  SPDX-License-Identifier: MIT
*/

// Move to each libarry

#include <gtest/gtest.h>
#include <Wire.h>
#include <M5Unified.h>
#include <M5UnitUnified.hpp>
#include <googletest/test_template.hpp>
#include <unit/unit_PAJ7620U2.hpp>

using namespace m5::unit::googletest;
using namespace m5::unit;
using namespace m5::unit::paj7620u2;;
using namespace m5::unit::paj7620u2::command;

const ::testing::Environment* global_fixture =
    ::testing::AddGlobalTestEnvironment(new GlobalFixture<400000U>());

class TestPAJ7620U2 : public ComponentTestBase<UnitPAJ7620U2, bool> {
   protected:
    virtual UnitPAJ7620U2* get_instance() override {
        return new m5::unit::UnitPAJ7620U2();
    }
    virtual bool is_using_hal() const override {
        return GetParam();
    };
};

// INSTANTIATE_TEST_SUITE_P(ParamValues, TestPAJ7620U2,
//                          ::testing::Values(false, true));
//  INSTANTIATE_TEST_SUITE_P(ParamValues, TestPAJ7620U2,
//  ::testing::Values(true));
INSTANTIATE_TEST_SUITE_P(ParamValues, TestPAJ7620U2, ::testing::Values(false));

TEST_P(TestPAJ7620U2, Suspend) {

    EXPECT_TRUE(unit->suspend());

    EXPECT_TRUE(unit->resume());
}
