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
using namespace m5::unit::paj7620u2;
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

TEST_P(TestPAJ7620U2, Gesture) {
    EXPECT_TRUE(unit->setMode(Mode::Gesture));

    EXPECT_TRUE(unit->setFrequency(Frequency::Gaming));
    EXPECT_EQ(unit->frequency(), Frequency::Gaming);
    EXPECT_TRUE(unit->setFrequency(Frequency::Normal));
    EXPECT_EQ(unit->frequency(), Frequency::Normal);

    Gesture ges{};
    uint16_t size{}, x{}, y{};
    uint8_t noobj{}, nomot{};
    EXPECT_TRUE(unit->readGesture(ges));
    EXPECT_TRUE(unit->readObjectSize(size));
    EXPECT_TRUE(unit->readObjectCenter(x, y));
    EXPECT_TRUE(unit->readNoObjectCount(noobj));
    EXPECT_TRUE(unit->readNoMotionCount(nomot));

    unit->update();
    EXPECT_EQ(unit->brightness(), 0);
    EXPECT_FALSE(unit->approach());
    EXPECT_EQ(unit->cursorX(), 0xFFFF);
    EXPECT_EQ(unit->cursorY(), 0xFFFF);
}

TEST_P(TestPAJ7620U2, Proximity) {
    EXPECT_TRUE(unit->setMode(Mode::Proximity));

    Gesture ges{};
    uint8_t brightness{}, approach{};
    EXPECT_TRUE(unit->readGesture(ges));
    EXPECT_TRUE(unit->readProximity(brightness, approach));

    EXPECT_TRUE(unit->setApprochThreshold(98, 76));
    uint8_t high{}, low{};
    EXPECT_TRUE(unit->getApprochThreshold(high, low));
    EXPECT_EQ(high, 98);
    EXPECT_EQ(low, 76);

    unit->update();
    EXPECT_EQ(unit->cursorX(), 0xFFFF);
    EXPECT_EQ(unit->cursorY(), 0xFFFF);
}

TEST_P(TestPAJ7620U2, Cursor) {
    EXPECT_TRUE(unit->setMode(Mode::Cursor));

    uint16_t x{}, y{};
    EXPECT_TRUE(unit->readCursor(x, y));

    unit->update();
    EXPECT_EQ(unit->brightness(), 0);
    EXPECT_FALSE(unit->approach());
}

TEST_P(TestPAJ7620U2, Other) {
    bool flip{}, flip2{};

    EXPECT_TRUE(unit->getHorizontalFlip(flip));
    EXPECT_TRUE(unit->setHorizontalFlip(!flip));
    EXPECT_TRUE(unit->getHorizontalFlip(flip2));
    EXPECT_NE(flip, flip2);

    EXPECT_TRUE(unit->getVerticalFlip(flip));
    EXPECT_TRUE(unit->setVerticalFlip(!flip));
    EXPECT_TRUE(unit->getVerticalFlip(flip2));
    EXPECT_NE(flip, flip2);
}
