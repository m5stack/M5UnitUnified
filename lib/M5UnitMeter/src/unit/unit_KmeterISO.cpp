/*!
  @file unit_KmeterISO.cpp
  @brief KmeterISO Unit for M5UnitUnified

  SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD

  SPDX-License-Identifier: MIT
*/
#include "unit_KmeterISO.hpp"
#include <M5Utility.hpp>
#include <array>
#include <thread>

namespace {

template <typename T>
T array_to_type(const std::array<uint8_t, 4>& a) {
    static_assert(std::is_integral<T>::value && sizeof(T) == 4, "Invalid type");
    return (((uint32_t)a[3]) << 24) | (((uint32_t)a[2]) << 16) |
           (((uint32_t)a[1]) << 8) | (((uint32_t)a[0]) << 0);
}
}  // namespace

namespace m5 {
namespace unit {

using namespace m5::utility::mmh3;
using namespace kmeter;
using namespace kmeter::command;

// class UnitKmeterISO
const char UnitKmeterISO::name[] = "UnitKmeterISO";
const types::uid_t UnitKmeterISO::uid{"UnitKmeterISO"_mmh3};
const types::uid_t UnitKmeterISO::attr{0};

bool UnitKmeterISO::begin() {
    uint8_t ver{0x00};
    if (!readFirmwareVersion(ver) && (ver == 0x00)) {
        M5_LIB_LOGE("Failed to read version");
        return false;
    }
    return _cfg.periodic ? startPeriodicMeasurement(_cfg.interval) : true;
}

void UnitKmeterISO::update(const bool force) {
    _updated = false;
    if (inPeriodic()) {
        unsigned long at{m5::utility::millis()};
        if (force || !_latest || at >= _latest + _interval) {
            _updated = read_measurement();
            if (_updated) {
                _latest = at;
            }
        }
    }
}

bool UnitKmeterISO::startPeriodicMeasurement(const uint32_t interval) {
    _interval = interval;
    _periodic = true;
    _latest   = 0;
    return true;
}

bool UnitKmeterISO::stopPeriodicMeasurement() {
    _periodic = _updated = false;
    return true;
}

bool UnitKmeterISO::readStatus(uint8_t& status) {
    return readRegister8(ERROR_STATUS_REG, status, 0);
}

bool UnitKmeterISO::readFirmwareVersion(uint8_t& ver) {
    return readRegister8(FIRMWARE_VERSION_REG, ver, 0);
}

bool UnitKmeterISO::readCelsiusTemperature(int32_t& ct) {
    std::array<uint8_t, 4> rbuf{};
    if (readRegister(TEMP_CELSIUS_VAL_REG, rbuf.data(), rbuf.size(), 0)) {
        ct = array_to_type<int32_t>(rbuf);
        return true;
    }
    return false;
}

bool UnitKmeterISO::readFahrenheitTemperature(int32_t& ft) {
    std::array<uint8_t, 4> rbuf{};
    if (readRegister(TEMP_FAHRENHEIT_VAL_REG, rbuf.data(), rbuf.size(), 0)) {
        ft = array_to_type<int32_t>(rbuf);
        return true;
    }
    return false;
}

bool UnitKmeterISO::readInternalCelsiusTemperature(int32_t& ct) {
    std::array<uint8_t, 4> rbuf{};
    if (readRegister(INTERNAL_TEMP_CELSIUS_VAL_REG, rbuf.data(), rbuf.size(),
                     0)) {
        ct = array_to_type<int32_t>(rbuf);
        return true;
    }
    return false;
}

bool UnitKmeterISO::readInternalFahrenheitTemperature(int32_t& ft) {
    std::array<uint8_t, 4> rbuf{};
    if (readRegister(INTERNAL_TEMP_FAHRENHEIT_VAL_REG, rbuf.data(), rbuf.size(),
                     0)) {
        ft = array_to_type<int32_t>(rbuf);
        return true;
    }
    return false;
}

bool UnitKmeterISO::readCelsiusTemperatureString(char* str) {
    if (str && readRegister(TEMP_CELSIUS_STRING_REG, (uint8_t*)str, 8U, 0)) {
        str[8] = '\0';
        return true;
    }
    return false;
}

bool UnitKmeterISO::readFahrenheitTemperatureString(char* str) {
    if (str && readRegister(TEMP_FAHRENHEIT_STRING_REG, (uint8_t*)str, 8U, 0)) {
        str[8] = '\0';
        return true;
    }
    return false;
}

bool UnitKmeterISO::readInternalCelsiusTemperatureString(char* str) {
    if (readRegister(INTERNAL_TEMP_CELSIUS_STRING_REG, (uint8_t*)str, 8U, 0)) {
        str[8] = '\0';
        return true;
    }
    return false;
}

bool UnitKmeterISO::readInternalFahrenheitTemperatureString(char* str) {
    if (str && readRegister(INTERNAL_TEMP_FAHRENHEIT_STRING_REG, (uint8_t*)str,
                            8U, 0)) {
        str[8] = '\0';
        return true;
    }
    return false;
}

bool UnitKmeterISO::changeI2CAddress(const uint8_t i2c_address) {
    if (!m5::utility::isValidI2CAddress(i2c_address)) {
        M5_LIB_LOGE("Invalid address : %02X", i2c_address);
        return false;
    }
    if (writeRegister8(I2C_ADDRESS_REG, i2c_address) &&
        changeAddress(i2c_address)) {
        // Wait wakeup
        uint8_t v{};
        bool done{};
        auto timeout_at = m5::utility::millis() + 100;
        do {
            done = readRegister8(I2C_ADDRESS_REG, v, 0);
            std::this_thread::yield();
        } while (!done && m5::utility::millis() <= timeout_at);
        return done;
    }
    return false;
}

bool UnitKmeterISO::readI2CAddress(uint8_t& i2c_address) {
    return readRegister8(I2C_ADDRESS_REG, i2c_address, 0);
}

//
bool UnitKmeterISO::read_measurement() {
    uint8_t status{0xFF};
    if (!readStatus(status) || (status != 0)) {
        M5_LIB_LOGW("Not read or error:%Xx", status);
        return false;
    }
    int32_t c{}, f{};
    if (readCelsiusTemperature(c) && readFahrenheitTemperature(f)) {
        _temperatureC = conversion(c);
        _temperatureF = conversion(f);
        return true;
    }
    return false;
}

}  // namespace unit
}  // namespace m5
