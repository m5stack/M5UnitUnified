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

// All M5Unit-* includes are conditional for CI builds without lib/ symlinks
#if __has_include(<M5UnitUnifiedENV.h>)
#include <M5UnitUnifiedENV.h>
#define HAS_UNIT_ENV
#endif
#if __has_include(<M5UnitUnifiedMETER.h>)
#include <M5UnitUnifiedMETER.h>
#define HAS_UNIT_METER
#endif
#if __has_include(<M5UnitUnifiedHUB.h>)
#include <M5UnitUnifiedHUB.h>
#define HAS_UNIT_HUB
#endif
#if __has_include(<M5UnitUnifiedGESTURE.h>)
#include <M5UnitUnifiedGESTURE.h>
#define HAS_UNIT_GESTURE
#endif
#if __has_include(<M5UnitUnifiedHEART.h>)
#include <M5UnitUnifiedHEART.h>
#define HAS_UNIT_HEART
#endif
#if __has_include(<M5UnitUnifiedTOF.h>)
#include <M5UnitUnifiedTOF.h>
#define HAS_UNIT_TOF
#endif
#if __has_include(<M5UnitUnifiedWEIGHT.h>)
#include <M5UnitUnifiedWEIGHT.h>
#define HAS_UNIT_WEIGHT
#endif
#if __has_include(<M5UnitUnifiedANADIG.h>)
#include <M5UnitUnifiedANADIG.h>
#define HAS_UNIT_ANADIG
#endif
#if __has_include(<M5UnitUnifiedCOLOR.h>)
#include <M5UnitUnifiedCOLOR.h>
#define HAS_UNIT_COLOR
#endif
#if __has_include(<M5UnitUnifiedTHERMO.h>)
#include <M5UnitUnifiedTHERMO.h>
#define HAS_UNIT_THERMO
#endif
#if __has_include(<M5UnitUnifiedDISTANCE.h>)
#include <M5UnitUnifiedDISTANCE.h>
#define HAS_UNIT_DISTANCE
#endif
#if __has_include(<M5UnitUnifiedEXTIO.h>)
#include <M5UnitUnifiedEXTIO.h>
#define HAS_UNIT_EXTIO
#endif
#if __has_include(<M5UnitUnifiedINFRARED.h>)
#include <M5UnitUnifiedINFRARED.h>
#define HAS_UNIT_INFRARED
#endif
#if __has_include(<M5UnitUnifiedRGB.h>)
#include <M5UnitUnifiedRGB.h>
#define HAS_UNIT_RGB
#endif
#if __has_include(<M5UnitUnifiedTUBE.h>)
#include <M5UnitUnifiedTUBE.h>
#define HAS_UNIT_TUBE
#endif
#if __has_include(<M5UnitUnifiedCRYPTO.h>)
#include <M5UnitUnifiedCRYPTO.h>
#define HAS_UNIT_CRYPTO
#endif
#if __has_include(<M5UnitUnifiedRFID.h>)
#include <M5UnitUnifiedRFID.h>
#define HAS_UNIT_RFID
#endif
#if __has_include(<M5UnitUnifiedKEYBOARD.h>)
#include <M5UnitUnifiedKEYBOARD.h>
#define HAS_UNIT_KEYBOARD
#endif
#if __has_include(<M5UnitUnifiedBUTTON.h>)
#include <M5UnitUnifiedBUTTON.h>
#define HAS_UNIT_BUTTON
#endif
#if __has_include(<M5UnitUnifiedDDS.h>)
#include <M5UnitUnifiedDDS.h>
#define HAS_UNIT_DDS
#endif
#if __has_include(<M5UnitUnifiedFINGER.h>)
#include <M5UnitUnifiedFINGER.h>
#define HAS_UNIT_FINGER
#endif
#if __has_include(<M5UnitUnifiedNFC.h>)
#include <M5UnitUnifiedNFC.h>
#define HAS_UNIT_NFC
#endif
#if __has_include(<M5UnitUnifiedRF433.h>)
#include <M5UnitUnifiedRF433.h>
#define HAS_UNIT_RF433
#endif
#if __has_include(<M5UnitUnifiedRS485.h>)
#include <M5UnitUnifiedRS485.h>
#define HAS_UNIT_RS485
#endif
#if __has_include(<M5UnitUnifiedRTC.h>)
#include <M5UnitUnifiedRTC.h>
#define HAS_UNIT_RTC
#endif

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

template <class U, typename... Args>
void each_unit_test(Args&&... args)
{
    SCOPED_TRACE(U::name);

    U* unit = new U(std::forward<Args>(args)...);
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
        U tmp(std::forward<Args>(args)...);
        // Move constructor
        U mc(std::move(tmp));
        // assign by move
        U mc2(std::forward<Args>(args)...);
        mc2 = std::move(mc);
    }

    vec.push_back(unit);
}

}  // namespace

// Check each value and check duplicate uid
TEST(UnitUnified, EachUnit)
{
#ifdef HAS_UNIT_ENV
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
#endif

#ifdef HAS_UNIT_METER
    // UnitADS111x is abstract, UnitAVmeterBase/UnitINA226 have protected ctor
    each_unit_test<m5::unit::UnitADS1113>();
    each_unit_test<m5::unit::UnitADS1114>();
    each_unit_test<m5::unit::UnitADS1115>();
    each_unit_test<m5::unit::meter::UnitEEPROM>();
    each_unit_test<m5::unit::UnitAmeter>();
    each_unit_test<m5::unit::UnitVmeter>();
    each_unit_test<m5::unit::UnitKmeterISO>();
    each_unit_test<m5::unit::UnitDualKmeter>();
    each_unit_test<m5::unit::UnitINA226_10A>();
    each_unit_test<m5::unit::UnitINA226_1A>();
#endif

#ifdef HAS_UNIT_HUB
    each_unit_test<m5::unit::UnitPCA9548AP>();
    each_unit_test<m5::unit::UnitPbHub>();
#endif

#ifdef HAS_UNIT_GESTURE
    each_unit_test<m5::unit::UnitPAJ7620U2>();
#endif

#ifdef HAS_UNIT_HEART
    each_unit_test<m5::unit::UnitMAX30100>();
    each_unit_test<m5::unit::UnitMAX30102>();
#endif

#ifdef HAS_UNIT_TOF
    each_unit_test<m5::unit::UnitVL53L0X>();
    each_unit_test<m5::unit::UnitVL53L1X>();
#endif

#ifdef HAS_UNIT_WEIGHT
    each_unit_test<m5::unit::UnitWeightI2C>();
    each_unit_test<m5::unit::UnitMiniScales>();
#endif

#ifdef HAS_UNIT_ANADIG
    each_unit_test<m5::unit::UnitMCP4725>();
    each_unit_test<m5::unit::UnitGP8413>();
    each_unit_test<m5::unit::UnitADS11XX>();
    each_unit_test<m5::unit::UnitADS1110>();
    each_unit_test<m5::unit::UnitADS1100>();
#endif

#ifdef HAS_UNIT_COLOR
    each_unit_test<m5::unit::UnitTCS3472x>();
    each_unit_test<m5::unit::UnitTCS34725>();
    each_unit_test<m5::unit::UnitTCS34727>();
#endif

#ifdef HAS_UNIT_THERMO
    each_unit_test<m5::unit::UnitMLX90614>();
    each_unit_test<m5::unit::UnitMLX90614BAA>();
    each_unit_test<m5::unit::UnitNCIR2>();
    each_unit_test<m5::unit::UnitThermal2>();
#endif

#ifdef HAS_UNIT_DISTANCE
    each_unit_test<m5::unit::UnitRCWL9620>();
    each_unit_test<m5::unit::UnitUltraSonicI2C>();
    each_unit_test<m5::unit::UnitUltraSonicIO>();
#endif

#ifdef HAS_UNIT_EXTIO
    each_unit_test<m5::unit::UnitExtIO2>();
#endif

#ifdef HAS_UNIT_INFRARED
    each_unit_test<m5::unit::UnitSTHS34PF80>();
#endif

#ifdef HAS_UNIT_RGB
    // Base classes UnitSK6812/UnitWS2812/UnitLED have protected ctor
    each_unit_test<m5::unit::UnitRGB>();
    each_unit_test<m5::unit::UnitRGBLED15>();
    each_unit_test<m5::unit::UnitRGBLED29>();
    each_unit_test<m5::unit::UnitRGBLED72>();
    each_unit_test<m5::unit::UnitRGBLED144>();
    each_unit_test<m5::unit::UnitRGBLED288>();
    each_unit_test<m5::unit::UnitRGBLEDStrip30>();
    each_unit_test<m5::unit::UnitRGBLEDStrip60>();
    each_unit_test<m5::unit::UnitRGBLEDStrip120>();
    each_unit_test<m5::unit::UnitRGBLEDStrip300>();
    each_unit_test<m5::unit::UnitHex>();
    each_unit_test<m5::unit::UnitNeoHex>();
    each_unit_test<m5::unit::UnitNeco>();
    each_unit_test<m5::unit::UnitPuzzle>();
    each_unit_test<m5::unit::UnitRectLED<5, 5, m5::unit::UnitWS2812>>();
    each_unit_test<m5::unit::UnitAtomMatrixRGB>();
#endif

#ifdef HAS_UNIT_TUBE
    each_unit_test<m5::unit::UnitMCP_H10>(0.1f, 3.1f, 100.f, -110.f);
    each_unit_test<m5::unit::UnitMCP_H10_B200KPPN>();
#endif

#ifdef HAS_UNIT_CRYPTO
    each_unit_test<m5::unit::UnitATECC608B>();
    each_unit_test<m5::unit::UnitATECC608B_TNGTLS>();
#endif

#ifdef HAS_UNIT_RFID
    each_unit_test<m5::unit::UnitMFRC522>();
    each_unit_test<m5::unit::UnitWS1850S>();
#endif

#ifdef HAS_UNIT_KEYBOARD
    each_unit_test<m5::unit::UnitKeyboard>();
    each_unit_test<m5::unit::UnitKeyboardBitwise>();
    each_unit_test<m5::unit::UnitCardKB>();
    each_unit_test<m5::unit::UnitFacesQWERTY>();
#endif

#ifdef HAS_UNIT_BUTTON
    each_unit_test<m5::unit::UnitButton>();
    each_unit_test<m5::unit::UnitDualButton>();
    each_unit_test<m5::unit::UnitKey>();
#endif

#ifdef HAS_UNIT_DDS
    each_unit_test<m5::unit::UnitDDS>();
#endif

#ifdef HAS_UNIT_FINGER
    // UnitFPC1XXX has protected ctor
    each_unit_test<m5::unit::UnitFPC1020A>();
    each_unit_test<m5::unit::UnitFinger2>();
#endif

#ifdef HAS_UNIT_NFC
    each_unit_test<m5::unit::UnitST25R3916>();
    each_unit_test<m5::unit::CapST25R3916>();
#endif

#ifdef HAS_UNIT_RF433
    each_unit_test<m5::unit::UnitSYN115>();
    each_unit_test<m5::unit::UnitSYN531R>();
#endif

#ifdef HAS_UNIT_RS485
    each_unit_test<m5::unit::UnitSP485>();
#endif

#ifdef HAS_UNIT_RTC
    each_unit_test<m5::unit::UnitPCF8563>();
#endif

    for (auto&& e : vec) {
        delete e;
    }
    vec.clear();
}
