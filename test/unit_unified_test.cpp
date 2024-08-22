/*
 * SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
/*
  UnitTest for M5UnitUnified
*/
#include <gtest/gtest.h>
#include <M5Unified.h>
#include <M5UnitComponent.hpp>
#include <M5UnitUnified.hpp>
#include <algorithm>
#include <utility>

#include <M5UnitUnifiedENV.h>
#include <M5UnitUnifiedMETER.h>
#include <M5UnitUnifiedHEART.h>
#include <M5UnitUnifiedHUB.h>
#include <M5UnitUnifiedGESTURE.h>
#include <M5UnitUnifiedKmeterISO.h>
#include <M5UnitUnifiedTVOC.h>

#if 0
#include <unit/unit_MFRC522.hpp>
#include <unit/unit_WS1850S.hpp>
#endif

namespace {

#if 0
// Get the equivalent of a unique type name without RTTI
template <typename U>
const char* TypeName() {
    return __PRETTY_FUNCTION__;
}
#endif

std::vector<m5::unit::Component*> vec;

template <class U>
void each_unit_test() {
    SCOPED_TRACE(U::name);

    U* unit = new U();
    EXPECT_TRUE((bool)unit);

    M5_LOGI(">>%02XH %08X [%s]", U::DEFAULT_ADDRESS, U::uid, U::name);

    // Are the values the same via class and via instance?
    EXPECT_EQ(+U::DEFAULT_ADDRESS, unit->address());
    EXPECT_STREQ(U::name, unit->deviceName());
    EXPECT_EQ(U::uid, unit->identifier());
    EXPECT_EQ(U::attr, unit->attribute());

    // Identical IDs exist?
    for (auto&& e : vec) {
        EXPECT_NE(unit->identifier(), e->identifier()) << unit->deviceName() << " / " << e->deviceName();
    }

    // Move
    {
        U tmp;
        // Move constructor
        U mc(std::move(tmp));
        // assign by move
        U mc2;
        mc2 = std::move(mc);
    }

    vec.push_back(unit);
}

}  // namespace

// Check each value and check duplicate uid
TEST(UnitUnified, EachUnit) {
    // ENV
    each_unit_test<m5::unit::UnitSCD40>();
    each_unit_test<m5::unit::UnitSCD41>();
    each_unit_test<m5::unit::UnitSHT30>();
    each_unit_test<m5::unit::UnitQMP6988>();
    each_unit_test<m5::unit::UnitENV3>();
    each_unit_test<m5::unit::UnitBME688>();
    // METER
    each_unit_test<m5::unit::UnitADS1113>();
    each_unit_test<m5::unit::UnitADS1114>();
    each_unit_test<m5::unit::UnitADS1115>();
    each_unit_test<m5::unit::UnitADS1115WithEEPROM>();
    each_unit_test<m5::unit::UnitAmeter>();
    each_unit_test<m5::unit::UnitVmeter>();
    // HEART
    each_unit_test<m5::unit::UnitMAX30100>();
    // HUB
    each_unit_test<m5::unit::UnitPCA9548AP>();
    // GESTURE
    each_unit_test<m5::unit::UnitPAJ7620U2>();
    // KmeterISO
    each_unit_test<m5::unit::UnitKmeterISO>();
    // TVOC
    each_unit_test<m5::unit::UnitSGP30>();
#if 0
    each_unit_test<m5::unit::UnitMFRC522>();
    each_unit_test<m5::unit::UnitWS1850S>();
#endif
    for (auto&& e : vec) {
        delete e;
    }
    vec.clear();
}
