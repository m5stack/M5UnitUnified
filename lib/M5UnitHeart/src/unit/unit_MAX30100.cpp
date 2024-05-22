/*!
  @file unit_MAX30100.cpp
  @brief MAX30100 Unit for M5UnitUnified

  SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD

  SPDX-License-Identifier: MIT
*/
#include "unit_MAX30100.hpp"
#include <M5Utility.hpp>
#include <limits>  // NaN

using namespace m5::utility::mmh3;

namespace m5 {
namespace unit {

using namespace max30100::command;

// class UnitMAX30100
const char UnitMAX30100::name[] = "UnitMAX30100";
const types::uid_t UnitMAX30100::uid{"UnitMAX30100"_mmh3};
const types::uid_t UnitMAX30100::attr{0};

bool UnitMAX30100::begin() {
#if 0
    if (!stopPeriodicMeasurement()) {
        M5_LIB_LOGE("Failed to stop");
        return false;
    }
    if (!setAutomaticSelfCalibrationEnabled(_cfg.auto_calibration)) {
        M5_LIB_LOGE("Failed to set calibration");
        return false;
    }
    return _cfg.start_periodic
               ? (_cfg.low_power ? startLowPowerPeriodicMeasurement()
                                 : startPeriodicMeasurement())
               : true;
#endif

    // Wait for communication to be established
    // TODO : Using begin/endTransaction
    uint32_t cnt{10};
    uint8_t pid{};
    while (!readRegister8(PART_ID, pid, 0) && cnt--) {
        m5::utility::delay(10);
    }

    return cnt != 0;
}

void UnitMAX30100::update() {
#if 0
    if (_periodic) {
        unsigned long at{m5::utility::millis()};
        if (!_latest || at >= _latest + _interval) {
            _updated = readMeasurement();
            if (_updated) {
                _latest = at;
            }
        } else {
            _updated = false;
        }
    }
#endif
}

bool UnitMAX30100::getModeConfiguration(max30100::ModeConfiguration& mc) {
    return get_mode_configration(mc.value);
}

bool UnitMAX30100::setModeConfiguration(const max30100::ModeConfiguration mc) {
    return set_mode_configration(mc.value);
}

bool UnitMAX30100::setMode(const max30100::Mode m) {
    max30100::ModeConfiguration mc{};
    if (get_mode_configration(mc.value)) {
        mc.mode(m);
        return set_mode_configration(mc.value);
    }
    return false;
}

bool UnitMAX30100::reset() {
    max30100::ModeConfiguration mc{};
    mc.reset(true);
    if (set_mode_configration(mc.value)) {
        uint32_t cnt{10};
        do {
            //  Is reset sequence completed?
            if (get_mode_configration(mc.value) && !mc.reset()) {
                return true;
            }
            m5::utility::delay(10);
        } while (cnt--);
    }
    return false;
}

bool UnitMAX30100::getSpO2Configuration(max30100::SpO2Configuration& sc) {
    return get_spo2_configration(sc.value);
}

bool UnitMAX30100::setSpO2Configuration(const max30100::SpO2Configuration sc) {
    return set_spo2_configration(sc.value);
}

bool UnitMAX30100::setSamplingRate(const max30100::SamplingRate rate) {
    max30100::SpO2Configuration sc{};
    if (get_spo2_configration(sc.value)) {
        sc.samplingRate(rate);
        return set_spo2_configration(sc.value);
    }
    return false;
}

bool UnitMAX30100::setLedPulseWidth(const max30100::LedPulseWidth width) {
    max30100::SpO2Configuration sc{};
    if (get_spo2_configration(sc.value)) {
        sc.ledPulseWidth(width);
        return set_spo2_configration(sc.value);
    }
    return false;
}

bool UnitMAX30100::getLedConfiguration(max30100::LedConfiguration& lc) {
    return get_led_configration(lc.value);
}
bool UnitMAX30100::setLedConfiguration(const max30100::LedConfiguration lc) {
    return set_led_configration(lc.value);
}

bool UnitMAX30100::setLedCurrent(const max30100::CurrentControl ir,
                                 const max30100::CurrentControl red) {
    max30100::LedConfiguration lc{};
    if (get_led_configration(lc.value)) {
        lc.irLed(ir);
        lc.redLed(red);
        return set_led_configration(lc.value);
    }
    return false;
}

bool UnitMAX30100::readFIFOData() {
    return false;
}

bool UnitMAX30100::measurementTemperature() {
    max30100::ModeConfiguration mc{};
    if (get_mode_configration(mc.value)) {
        mc.temperature(true);
        //        M5_LIB_LOGE(">> %x", mc.value);
        return set_mode_configration(mc.value);
    }
    return false;
}

bool UnitMAX30100::isMeasurementTemperature() {
    max30100::ModeConfiguration mc{};

#if 1
    return get_mode_configration(mc.value) && !mc.temperature();
#else
    if (get_mode_configration(mc.value)) {
        M5_LIB_LOGE("== %x", mc.value);
        return !mc.temperature();
    }
    return false;
#endif
}

bool UnitMAX30100::readMeasurementTemperature(float& temp) {
#if 0
    m5::types::big_uint16_t v{};
    temp = std::numeric_limits<float>::quiet_NaN();

    if (readRegister(TEMP_INTEGER, v.data(), v.size(), 0)) {
        M5_LIB_LOGE("[%02x:%02x]", v.u8[0], v.u8[1]);
        temp = (int8_t)v.u8[0] + v.u8[1] * 0.0625f;
        return true;
    }
#else
    uint8_t ti{}, tf{};
    if (readRegister8(READ_TEMP_INTEGER, ti, 0) &&
        readRegister8(READ_TEMP_FRACTION, tf, 0)) {

        M5_LIB_LOGE(">> %02x:%02x", ti, tf);

        temp = (int8_t)ti + tf * 0.0625f;
        return true;
    }
#endif

    return false;
}

//
bool UnitMAX30100::get_mode_configration(uint8_t& c) {
    return readRegister8(MODE_CONFIGURATION, c, 0);
}

bool UnitMAX30100::set_mode_configration(const uint8_t c) {
    // The write is considered successful even if the value is not allowed
    // in the configuration. However, the value is not actually set, so
    // check it.
    uint8_t chk{};
#if 1
    return writeRegister8(MODE_CONFIGURATION, c) &&
           get_mode_configration(chk) && (c & 0x87) == (chk & 0x87);
#else
    if (writeRegister8(MODE_CONFIGURATION, c)) {
        if (get_mode_configration(chk)) {
            M5_LIB_LOGE("== %x : %x", c, chk);
            return (c & 0x87) == (chk & 0x87);
        }
    }
    return false;

#endif
}

bool UnitMAX30100::enable_power_save(const bool enabled) {
    max30100::ModeConfiguration mc{};
    if (get_mode_configration(mc.value)) {
        mc.shdn(enabled);
        return set_mode_configration(mc.value);
    }
    return false;
}

bool UnitMAX30100::get_spo2_configration(uint8_t& c) {
    return readRegister8(SPO2_CONFIGURATION, c, 0);
}

bool UnitMAX30100::set_spo2_configration(const uint8_t c) {
    // The write is considered successful even if the value is not allowed
    // in the configuration. However, the value is not actually set, so
    // check it.
    uint8_t chk{};
#if 1
    return writeRegister8(SPO2_CONFIGURATION, c) &&
           get_spo2_configration(chk) && (chk == c);
#else
    if (writeRegister8(SPO2_CONFIGURATION, c)) {
        if (get_spo2_configration(chk)) {
            M5_LIB_LOGE(">> %x : %x", c, chk);
            return (chk == c);
        }
    }
    return false;
#endif
}

bool UnitMAX30100::enable_high_resolution(const bool enabled) {
    max30100::SpO2Configuration sc{};
    if (get_spo2_configration(sc.value)) {
        sc.highResolution(enabled);
        return set_spo2_configration(sc.value);
    }
    return false;
}

bool UnitMAX30100::get_led_configration(uint8_t& c) {
    return readRegister8(LED_CONFIGURATION, c, 0);
}
bool UnitMAX30100::set_led_configration(const uint8_t c) {
    return writeRegister8(LED_CONFIGURATION, c);
}

}  // namespace unit
}  // namespace m5
