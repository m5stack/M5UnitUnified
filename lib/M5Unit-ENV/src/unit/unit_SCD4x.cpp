/*!
  @file unit_SCD4x.cpp
  @brief SCD4X family Unit for M5UnitUnified

  SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD

  SPDX-License-Identifier: MIT
*/
#include "unit_SCD4x.hpp"
#include <M5Utility.hpp>
#include <limits>  // NaN

using namespace m5::utility::mmh3;

namespace {
struct Temperature {
    constexpr static float toFloat(const uint16_t u16) {
        return u16 * 175.f / 65536.f;
    }
    constexpr static uint16_t toUint16(const float f) {
        return f * 65536 / 175;
    }
    constexpr static float OFFSET_MIN{0.0f};
    constexpr static float OFFSET_MAX{175.0f};
};

}  // namespace

namespace m5 {
namespace unit {

using namespace scd4x::command;

// class UnitSCD40
const char UnitSCD40::name[] = "UnitSCD40";
const types::uid_t UnitSCD40::uid{"UnitSCD40"_mmh3};
const types::uid_t UnitSCD40::attr{0};

bool UnitSCD40::begin() {
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
}

void UnitSCD40::update() {
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
}

bool UnitSCD40::startPeriodicMeasurement(void) {
    if (_periodic) {
        M5_LIB_LOGD("Periodic measurements are running");
        return false;
    }
    _periodic = writeRegister(START_PERIODIC_MEASUREMENT);
    if (_periodic) {
        _interval = SIGNAL_INTERVAL_MS;
    }
    return _periodic;
}

bool UnitSCD40::stopPeriodicMeasurement(const uint16_t delayMillis) {
    if (writeRegister(STOP_PERIODIC_MEASUREMENT)) {
        _periodic = false;
        m5::utility::delay(delayMillis);
        return true;
    }
    return false;
}

bool UnitSCD40::readMeasurement() {
    if (!getDataReadyStatus()) {
        M5_LIB_LOGD("Not ready");
        return false;
    }

    return read_measurement();
}

bool UnitSCD40::setTemperatureOffset(const float offset,
                                     const uint16_t delayMillis) {
    if (_periodic) {
        M5_LIB_LOGD("Periodic measurements are running");
        return false;
    }
    if (offset < Temperature::OFFSET_MIN || offset >= Temperature::OFFSET_MAX) {
        M5_LIB_LOGE("offset is not a valid scope %f", offset);
        return false;
    }

    utility::WriteDataWithCRC16 wd(Temperature::toUint16(offset));
    auto ret = writeRegister(SET_TEMPERATURE_OFFSET, wd.data(), wd.size());
    m5::utility::delay(delayMillis);
    return ret;
}

float UnitSCD40::getTemperatureOffset(void) {
    float offset{};
    getTemperatureOffset(offset);
    return offset;
}

bool UnitSCD40::getTemperatureOffset(float& offset) {
    offset = 0.0f;
    if (_periodic) {
        M5_LIB_LOGD("Periodic measurements are running");
        return false;
    }
    uint16_t u16{};
    auto ret = readRegister16(GET_TEMPERATURE_OFFSET, u16, 1);
    offset   = Temperature::toFloat(u16);
    return ret;
}

bool UnitSCD40::setSensorAltitude(const uint16_t altitude,
                                  const uint16_t delayMillis) {
    if (_periodic) {
        M5_LIB_LOGD("Periodic measurements are running");
        return false;
    }

    utility::WriteDataWithCRC16 wd(altitude);
    auto ret = writeRegister(SET_SENSOR_ALTITUDE, wd.data(), wd.size());
    m5::utility::delay(delayMillis);
    return ret;
}

uint16_t UnitSCD40::getSensorAltitude(void) {
    uint16_t altitude{};
    getSensorAltitude(altitude);
    return altitude;
}

bool UnitSCD40::getSensorAltitude(uint16_t& altitude) {
    altitude = 0;
    if (_periodic) {
        M5_LIB_LOGD("Periodic measurements are running");
        return false;
    }
    return readRegister16(GET_SENSOR_ALTITUDE, altitude, 1);
}

bool UnitSCD40::setAmbientPressure(const float pressure,
                                   const uint16_t delayMillis) {
    if (pressure < 0.0f || pressure > 65535.f * 100) {
        M5_LIB_LOGE("pressure is not a valid scope %f", pressure);
        return false;
    }

    // uint16_t u16 = (uint16_t)(pressure / 100);
    //     auto ret     = sendCommand(SET_AMBIENT_PRESSURE, u16);

    utility::WriteDataWithCRC16 wd((uint16_t)(pressure / 100));
    auto ret = writeRegister(SET_AMBIENT_PRESSURE, wd.data(), wd.size());
    m5::utility::delay(delayMillis);
    return ret;
}

int16_t UnitSCD40::performForcedRecalibration(const uint16_t concentration) {
    int16_t correction{};
    performForcedRecalibration(concentration, correction);
    return correction;
}

bool UnitSCD40::performForcedRecalibration(const uint16_t concentration,
                                           int16_t& correction) {
    // 1. Operate the SCD4x in the operation mode later used in normal sensor
    // operation (periodic measurement, low power periodic measurement or single
    // shot) for > 3 minutes in an environment with homogenous and constant CO2
    // concentration.
    // 2. Issue stop_periodic_measurement. Wait 500 ms for the stop command to
    // complete.
    correction = 0;
    if (_periodic) {
        M5_LIB_LOGD("Periodic measurements are running");
        return false;
    }

    utility::WriteDataWithCRC16 wd(concentration);
    if (!writeRegister(PERFORM_FORCED_CALIBRATION, wd.data(), wd.size())) {
        return false;
    }

    // 3. Subsequently issue the perform_forced_recalibration command and
    // optionally read out the FRC correction (i.e. the magnitude of the
    // correction) after waiting for 400 ms for the command to complete.
    m5::utility::delay(400);

    std::array<uint8_t, 3> rbuf{};
    return readWithTransaction(
        rbuf.data(), rbuf.size(), [this, &rbuf, &correction]() {
            utility::ReadDataWithCRC16 data(rbuf.data(), 1);
            if (data.valid(0)) {
                correction = (int16_t)(data.value(0) - 0x8000);
                return data.value(0) != 0xFFFF;
            }
            return false;
        });
}

bool UnitSCD40::setAutomaticSelfCalibrationEnabled(const bool enabled,
                                                   const uint16_t delayMillis) {
    if (_periodic) {
        M5_LIB_LOGD("Periodic measurements are running");
        return false;
    }

    utility::WriteDataWithCRC16 wd(enabled ? 1 : 0);
    auto ret = writeRegister(SET_AUTOMATIC_SELF_CALIBRATION_ENABLED, wd.data(),
                             wd.size());
    m5::utility::delay(delayMillis);
    return ret;
}

bool UnitSCD40::getAutomaticSelfCalibrationEnabled(void) {
    bool enabled{};
    return getAutomaticSelfCalibrationEnabled(enabled) ? enabled : false;
}

bool UnitSCD40::getAutomaticSelfCalibrationEnabled(bool& enabled) {
    enabled = false;
    if (_periodic) {
        M5_LIB_LOGD("Periodic measurements are running");
        return false;
    }
    uint16_t u16{};
    auto ret = readRegister16(GET_AUTOMATIC_SELF_CALIBRATION_ENABLED, u16, 1);
    enabled  = (u16 == 0x0001);
    return ret;
}

bool UnitSCD40::startLowPowerPeriodicMeasurement(void) {
    if (_periodic) {
        M5_LIB_LOGD("Periodic measurements are running");
        return false;
    }
    _periodic = writeRegister(START_LOW_POWER_PERIODIC_MEASUREMENT);
    if (_periodic) {
        _interval = SIGNAL_INTERVAL_LOW_MS;
    }
    return _periodic;
}

bool UnitSCD40::getDataReadyStatus() {
    uint16_t res{};
    return readRegister16(GET_DATA_READY_STATUS, res, 1) ? (res & 0x07FF) != 0
                                                         : false;
}

bool UnitSCD40::persistSettings(uint16_t delayMillis) {
    if (_periodic) {
        M5_LIB_LOGD("Periodic measurements are running");
        return false;
    }

    auto ret = writeRegister(PERSIST_SETTINGS);
    m5::utility::delay(delayMillis);
    return ret;
}

bool UnitSCD40::getSerialNumber(char* serialNumber) {
    if (!serialNumber) {
        return false;
    }

    *serialNumber = '\0';
    uint64_t sno{};
    if (getSerialNumber(sno)) {
        uint_fast8_t i{12};
        while (i--) {
            *serialNumber++ =
                m5::utility::uintToHexChar((sno >> (i * 4)) & 0x0F);
        }
        *serialNumber = '\0';
        return true;
    }
    return false;
}

uint64_t UnitSCD40::getSerialNumber() {
    uint64_t sno{};
    return getSerialNumber(sno) ? sno : 0ULL;
}

bool UnitSCD40::getSerialNumber(uint64_t& serialNumber) {
    serialNumber = 0;
    if (_periodic) {
        M5_LIB_LOGD("Periodic measurements are running");
        return false;
    }
    if (!writeRegister(GET_SERIAL_NUMBER)) {
        return false;
    }

    m5::utility::delay(1);

    std::array<uint8_t, 9> rbuf;
    return readWithTransaction(
        rbuf.data(), rbuf.size(), [this, &rbuf, &serialNumber]() {
            utility::ReadDataWithCRC16 data(rbuf.data(), 3);
            bool valid[3] = {data.valid(0), data.valid(1), data.valid(2)};
            if (valid[0] && valid[1] && valid[2]) {
                for (uint_fast8_t i = 0; i < 3; ++i) {
                    serialNumber |= ((uint64_t)data.value(i))
                                    << (16U * (2 - i));
                }
                return true;
            }
            return false;
        });
}

bool UnitSCD40::performSelfTest(void) {
    bool malfunction{};
    return performSelfTest(malfunction) && !malfunction;
}

bool UnitSCD40::performSelfTest(bool& malfunction) {
    if (_periodic) {
        M5_LIB_LOGD("Periodic measurements are running");
        return false;
    }

    uint16_t response{};
    auto ret    = readRegister16(PERFORM_SELF_TEST, response, 10 * 1000);
    malfunction = (response != 0);
    return ret;
}

bool UnitSCD40::performFactoryReset(uint16_t delayMillis) {
    if (_periodic) {
        M5_LIB_LOGD("Periodic measurements are running");
        return false;
    }
    auto ret = writeRegister(PERFORM_FACTORY_RESET);
    m5::utility::delay(delayMillis);
    return ret;
}

bool UnitSCD40::reInit(uint16_t delayMillis) {
    if (_periodic) {
        M5_LIB_LOGD("Periodic measurements are running");
        return false;
    }

    auto ret = writeRegister(REINIT);
    m5::utility::delay(delayMillis);
    return ret;
}

// RHT only if false
bool UnitSCD40::read_measurement(const bool all) {
    if (!writeRegister(READ_MEASUREMENT)) {
        return false;
    }

    m5::utility::delay(1);

    _co2         = 0;
    _temperature = _humidity = std::numeric_limits<float>::quiet_NaN();

    std::array<uint8_t, 9> rbuf{};
    return readWithTransaction(rbuf.data(), rbuf.size(), [this, &rbuf, &all] {
        utility::ReadDataWithCRC16 data(rbuf.data(), 3);
        bool valid[3] = {data.valid(0), data.valid(1), data.valid(2)};

        if (all && valid[0]) {
            this->_co2 = data.value(0);
        }
        if (valid[1]) {
            this->_temperature = -45 + Temperature::toFloat(data.value(1));
        }
        if (valid[2]) {
            this->_humidity = 100.f * data.value(2) / 65536.f;
        }
        return (!all || valid[0]) && valid[1] && valid[2];
    });
}

// class UnitSCD41
const char UnitSCD41::name[] = "UnitSCD41";
const types::uid_t UnitSCD41::uid{"UnitSCD41"_mmh3};
const types::uid_t UnitSCD41::attr{0};

bool UnitSCD41::measureSingleShot(void) {
    if (_periodic) {
        M5_LIB_LOGD("Periodic measurements are running");
        return false;
    }
    if (writeRegister(MEASURE_SINGLE_SHOT) && readMeasurement()) {
        m5::utility::delay(5000);
        return true;
    }
    return false;
}

bool UnitSCD41::measureSingleShotRHTOnly(void) {
    if (_periodic) {
        M5_LIB_LOGD("Periodic measurements are running");
        return false;
    }
    if (writeRegister(MEASURE_SINGLE_SHOT) && readMeasurement()) {
        m5::utility::delay(50);
        return true;
    }
    return false;
}

}  // namespace unit
}  // namespace m5

// TODO
// Comapti test old and new
