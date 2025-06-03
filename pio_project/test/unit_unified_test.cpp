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

#include <M5UnitUnifiedENV.h>
#include <M5UnitUnifiedMETER.h>
#include <M5UnitUnifiedHUB.h>
#include <M5UnitUnifiedGESTURE.h>
#include <M5UnitUnifiedHEART.h>
#include <M5UnitUnifiedTOF.h>
#include <M5UnitUnifiedWEIGHT.h>
#include <M5UnitUnifiedANADIG.h>
#include <M5UnitUnifiedCOLOR.h>
#include <M5UnitUnifiedTHERMO.h>
#include <M5UnitUnifiedDISTANCE.h>
#include <M5UnitUnifiedEXTIO.h>
#include <M5UnitUnifiedINFRARED.h>
#include <M5UnitUnifiedCRYPTO.h>

#include <M5UnitUnifiedRFID.h>
#include <M5UnitUnifiedKEYBOARD.h>

#include <algorithm>
#include <utility>

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
void each_unit_test()
{
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
TEST(UnitUnified, EachUnit)
{
    // ENV
    each_unit_test<m5::unit::UnitSCD40>();
    each_unit_test<m5::unit::UnitSCD41>();
    each_unit_test<m5::unit::UnitSHT30>();
    each_unit_test<m5::unit::UnitQMP6988>();
    each_unit_test<m5::unit::UnitENV3>();
    each_unit_test<m5::unit::UnitBME688>();
    each_unit_test<m5::unit::UnitSGP30>();
    each_unit_test<m5::unit::UnitSHT40>();
    each_unit_test<m5::unit::UnitBMP280>();
    each_unit_test<m5::unit::UnitENV4>();

    // METER
    each_unit_test<m5::unit::UnitADS1113>();
    each_unit_test<m5::unit::UnitADS1114>();
    each_unit_test<m5::unit::UnitADS1115>();
    each_unit_test<m5::unit::meter::UnitEEPROM>();
    each_unit_test<m5::unit::UnitAmeter>();
    each_unit_test<m5::unit::UnitVmeter>();
    each_unit_test<m5::unit::UnitKmeterISO>();
    each_unit_test<m5::unit::UnitDualKmeter>();

    // HUB
    each_unit_test<m5::unit::UnitPCA9548AP>();

    // GESTURE
    each_unit_test<m5::unit::UnitPAJ7620U2>();

    // HEART
    each_unit_test<m5::unit::UnitMAX30100>();
    each_unit_test<m5::unit::UnitMAX30102>();

    // TOF
    each_unit_test<m5::unit::UnitVL53L0X>();
    each_unit_test<m5::unit::UnitVL53L1X>();

    // WEIGHT
    each_unit_test<m5::unit::UnitWeightI2C>();
    each_unit_test<m5::unit::UnitMiniScales>();

    // ANADIG
    each_unit_test<m5::unit::UnitMCP4725>();
    each_unit_test<m5::unit::UnitGP8413>();
    each_unit_test<m5::unit::UnitADS11XX>();
    each_unit_test<m5::unit::UnitADS1110>();
    each_unit_test<m5::unit::UnitADS1100>();

    // COLOR
    each_unit_test<m5::unit::UnitTCS34725>();

    // THERMO
    each_unit_test<m5::unit::UnitMLX90614>();
    each_unit_test<m5::unit::UnitMLX90614BAA>();
    each_unit_test<m5::unit::UnitNCIR2>();

    // DISTANCE
    each_unit_test<m5::unit::UnitRCWL9620>();

    // EXTIO
    each_unit_test<m5::unit::UnitExtIO2>();

    // INFRARED
    each_unit_test<m5::unit::UnitSTHS34PF80>();

    // CRYPTO
    each_unit_test<m5::unit::UnitATECC608B>();
    each_unit_test<m5::unit::UnitATECC608B_TNGTLS>();

    // RFID
    each_unit_test<m5::unit::UnitMFRC522>();
    each_unit_test<m5::unit::UnitWS1850S>();

    // KEYBOARD
    each_unit_test<m5::unit::UnitKeyboard>();
    each_unit_test<m5::unit::UnitKeyboardBitwise>();
    each_unit_test<m5::unit::UnitCardKB>();
    each_unit_test<m5::unit::UnitFacesQWERTY>();

    for (auto&& e : vec) {
        delete e;
    }
    vec.clear();
}
