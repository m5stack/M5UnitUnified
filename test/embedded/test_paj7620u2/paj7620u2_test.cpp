/*
 * SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
/*
  UnitTest for UnitPAJ7620U2
*/

// Move to each libarry

#include <gtest/gtest.h>
#include <Wire.h>
#include <M5Unified.h>
#include <M5UnitUnified.hpp>
#include <googletest/test_template.hpp>
#include <googletest/test_helper.hpp>
#include <unit/unit_PAJ7620U2.hpp>

using namespace m5::unit::googletest;
using namespace m5::unit;
using namespace m5::unit::paj7620u2;
using namespace m5::unit::paj7620u2::command;

const ::testing::Environment* global_fixture =
    ::testing::AddGlobalTestEnvironment(new GlobalFixture<400000U>());

struct TestParams {
    bool hal;
    bool store_on_change;
};

class TestPAJ7620U2 : public ComponentTestBase<UnitPAJ7620U2, TestParams> {
   protected:
    virtual UnitPAJ7620U2* get_instance() override {
        auto ptr = new m5::unit::UnitPAJ7620U2();
        if (ptr) {
            auto cfg            = ptr->config();
            cfg.start_periodic  = false;
            cfg.stored_size     = 8;
            cfg.store_on_change = GetParam().store_on_change;
            ptr->config(cfg);
        }
        return ptr;
    }
    virtual bool is_using_hal() const override {
        return GetParam().hal;
    };
};

// INSTANTIATE_TEST_SUITE_P(ParamValues, TestPAJ7620U2,
//                          ::testing::Values(false, true));
//  INSTANTIATE_TEST_SUITE_P(ParamValues, TestPAJ7620U2,
//  ::testing::Values(true));
// INSTANTIATE_TEST_SUITE_P(ParamValues, TestPAJ7620U2,
// ::testing::Values(false));

INSTANTIATE_TEST_SUITE_P(ParamValues, TestPAJ7620U2,
                         ::testing::Values(TestParams{false, true},
                                           TestParams{false, false}));

using check_param_callback = void (*)(UnitPAJ7620U2*);

TEST_P(TestPAJ7620U2, Suspend) {
    SCOPED_TRACE(ustr);

    EXPECT_TRUE(unit->suspend());
    EXPECT_TRUE(unit->resume());
}

TEST_P(TestPAJ7620U2, Gesture) {
    SCOPED_TRACE(ustr);

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

    //
    EXPECT_FALSE(unit->inPeriodic());
    EXPECT_TRUE(unit->startPeriodicMeasurement(10));
    EXPECT_TRUE(unit->inPeriodic());
    EXPECT_EQ(unit->interval(), 10U);

    test_periodic_measurement(unit.get(), 16, check_param_callback(nullptr),
                              true);

    EXPECT_TRUE(unit->stopPeriodicMeasurement());
    EXPECT_FALSE(unit->inPeriodic());

    uint32_t cnt{};

    // They do not accumulate in the same state
    if (GetParam().store_on_change) {
        EXPECT_FALSE(unit->full());
        EXPECT_FALSE(unit->empty());
        EXPECT_EQ(unit->available(), 1);

        while (unit->available()) {
            ++cnt;
            unit->discard();
        }
        EXPECT_EQ(cnt, 1);
        EXPECT_TRUE(unit->empty());
        EXPECT_FALSE(unit->full());
        EXPECT_EQ(unit->available(), 0U);

    } else {
        EXPECT_TRUE(unit->full());
        EXPECT_FALSE(unit->empty());
        EXPECT_EQ(unit->available(), 8);

        while (unit->available()) {
            ++cnt;
            unit->discard();
            EXPECT_EQ(unit->available(), 8U - cnt);
        }
        EXPECT_EQ(cnt, 8);
        EXPECT_TRUE(unit->empty());
        EXPECT_FALSE(unit->full());
        EXPECT_EQ(unit->available(), 0U);
    }
}

TEST_P(TestPAJ7620U2, Proximity) {
    SCOPED_TRACE(ustr);

    EXPECT_TRUE(unit->setMode(Mode::Proximity));

    Gesture ges{};
    uint8_t brightness{}, approach{};
    EXPECT_TRUE(unit->readGesture(ges));
    EXPECT_TRUE(unit->readProximity(brightness, approach));

    EXPECT_TRUE(unit->setApproachThreshold(98, 76));
    uint8_t high{}, low{};
    EXPECT_TRUE(unit->readApproachThreshold(high, low));
    EXPECT_EQ(high, 98);
    EXPECT_EQ(low, 76);

    unit->update();
    EXPECT_EQ(unit->cursorX(), 0xFFFF);
    EXPECT_EQ(unit->cursorY(), 0xFFFF);
}

TEST_P(TestPAJ7620U2, Cursor) {
    SCOPED_TRACE(ustr);

    EXPECT_TRUE(unit->setMode(Mode::Cursor));

    uint16_t x{}, y{};
    EXPECT_TRUE(unit->readCursor(x, y));

    unit->update();
    EXPECT_EQ(unit->brightness(), 0);
    EXPECT_FALSE(unit->approach());
}

TEST_P(TestPAJ7620U2, Flip) {
    SCOPED_TRACE(ustr);

    bool flip{}, flip2{};

    EXPECT_TRUE(unit->readHorizontalFlip(flip));
    EXPECT_TRUE(unit->setHorizontalFlip(!flip));
    EXPECT_TRUE(unit->readHorizontalFlip(flip2));
    EXPECT_NE(flip, flip2);

    EXPECT_TRUE(unit->readVerticalFlip(flip));
    EXPECT_TRUE(unit->setVerticalFlip(!flip));
    EXPECT_TRUE(unit->readVerticalFlip(flip2));
    EXPECT_NE(flip, flip2);
}
