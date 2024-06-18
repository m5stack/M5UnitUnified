/*!
  @file unit_SCD4x.cpp
  @brief SCD4X family Unit for M5UnitUnified

  SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD

  SPDX-License-Identifier: MIT
*/
#include "unit_SCD4x.hpp"
#include <M5Utility.hpp>
#include <limits>  // NaN
#include <array>

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

// Max command duration(ms)
constexpr uint16_t READ_MEASUREMENT_DURATION{1};
constexpr uint16_t STOP_PERIODIC_MEASUREMENT_DURATION{500};
constexpr uint16_t SET_TEMPERATURE_OFFSET_DURATION{1};
constexpr uint16_t GET_TEMPERATURE_OFFSET_DURATION{1};
constexpr uint16_t SET_SENSOR_ALTITUDE_DURATION{1};
constexpr uint16_t GET_SENSOR_ALTITUDE_DURATION{1};
constexpr uint16_t SET_AMBIENT_PRESSURE_DURATION{1};
constexpr uint16_t PERFORM_FORCED_CALIBRATION_DURATION{400};
constexpr uint16_t SET_AUTOMATIC_SELF_CALIBRATION_ENABLED_DURATION{1};
constexpr uint16_t GET_AUTOMATIC_SELF_CALIBRATION_ENABLED_DURATION{1};
constexpr uint16_t GET_DATA_READY_STATUS_READY{1};
constexpr uint16_t PERSIST_SETTINGS_DURATION{800};
constexpr uint16_t GET_SERIAL_NUMBER_DURATION{1};
constexpr uint16_t PERFORM_SELF_TEST_DURATION{10000};
constexpr uint16_t PERFORM_FACTORY_RESET_DURATION{1200};
constexpr uint16_t REINIT_DURATION{20};
constexpr uint16_t MEASURE_SINGLE_SHOT_DURATION{5000};
constexpr uint16_t MEASURE_SINGLE_SHOT_RHT_ONLY_DURATION{50};

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
    if (inPeriodic()) {
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
    if (inPeriodic()) {
        M5_LIB_LOGD("Periodic measurements are running");
        return false;
    }
    _periodic = writeRegister(START_PERIODIC_MEASUREMENT);
    if (_periodic) {
        _interval = SIGNAL_INTERVAL_MS;
    }
    return _periodic;
}

bool UnitSCD40::stopPeriodicMeasurement() {
    if (writeRegister(STOP_PERIODIC_MEASUREMENT)) {
        _periodic = false;
        m5::utility::delay(STOP_PERIODIC_MEASUREMENT_DURATION);
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

bool UnitSCD40::setTemperatureOffset(const float offset) {
    if (inPeriodic()) {
        M5_LIB_LOGD("Periodic measurements are running");
        return false;
    }
    if (offset < Temperature::OFFSET_MIN || offset >= Temperature::OFFSET_MAX) {
        M5_LIB_LOGE("offset is not a valid scope %f", offset);
        return false;
    }

    utility::WriteDataWithCRC16 wd(Temperature::toUint16(offset));
    if (writeRegister(SET_TEMPERATURE_OFFSET, wd.data(), wd.size())) {
        m5::utility::delay(SET_TEMPERATURE_OFFSET_DURATION);
        return true;
    }
    return false;
}

bool UnitSCD40::getTemperatureOffset(float& offset) {
    offset = 0.0f;
    if (inPeriodic()) {
        M5_LIB_LOGD("Periodic measurements are running");
        return false;
    }
    uint16_t u16{};
    auto ret = readRegister16(GET_TEMPERATURE_OFFSET, u16,
                              GET_TEMPERATURE_OFFSET_DURATION);
    offset   = Temperature::toFloat(u16);
    return ret;
}

bool UnitSCD40::setSensorAltitude(const uint16_t altitude) {
    if (inPeriodic()) {
        M5_LIB_LOGD("Periodic measurements are running");
        return false;
    }

    utility::WriteDataWithCRC16 wd(altitude);
    if (writeRegister(SET_SENSOR_ALTITUDE, wd.data(), wd.size())) {
        m5::utility::delay(SET_SENSOR_ALTITUDE_DURATION);
        return true;
    }
    return false;
}

bool UnitSCD40::getSensorAltitude(uint16_t& altitude) {
    altitude = 0;
    if (inPeriodic()) {
        M5_LIB_LOGD("Periodic measurements are running");
        return false;
    }
    return readRegister16(GET_SENSOR_ALTITUDE, altitude,
                          GET_SENSOR_ALTITUDE_DURATION);
}

bool UnitSCD40::setAmbientPressure(const float pressure) {
    if (pressure < 0.0f || pressure > 65535.f * 100) {
        M5_LIB_LOGE("pressure is not a valid scope %f", pressure);
        return false;
    }

    // uint16_t u16 = (uint16_t)(pressure / 100);
    //     auto ret     = sendCommand(SET_AMBIENT_PRESSURE, u16);

    utility::WriteDataWithCRC16 wd((uint16_t)(pressure / 100));
    if (writeRegister(SET_AMBIENT_PRESSURE, wd.data(), wd.size())) {
        m5::utility::delay(SET_AMBIENT_PRESSURE_DURATION);
        return true;
    }
    return false;
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
    if (inPeriodic()) {
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
    m5::utility::delay(PERFORM_FORCED_CALIBRATION_DURATION);

    std::array<uint8_t, 3> rbuf{};
    if (readWithTransaction(rbuf.data(), rbuf.size()) ==
        m5::hal::error::error_t::OK) {
        utility::ReadDataWithCRC16 data(rbuf.data(), 1);
        if (data.valid(0)) {
            correction = (int16_t)(data.value(0) - 0x8000);
            return data.value(0) != 0xFFFF;
        }
    }
    return false;
}

bool UnitSCD40::setAutomaticSelfCalibrationEnabled(const bool enabled) {
    if (inPeriodic()) {
        M5_LIB_LOGD("Periodic measurements are running");
        return false;
    }

    utility::WriteDataWithCRC16 wd(enabled ? 1 : 0);
    if (writeRegister(SET_AUTOMATIC_SELF_CALIBRATION_ENABLED, wd.data(),
                      wd.size())) {
        m5::utility::delay(SET_AUTOMATIC_SELF_CALIBRATION_ENABLED_DURATION);
        return true;
    }
    return false;
}

bool UnitSCD40::getAutomaticSelfCalibrationEnabled(bool& enabled) {
    enabled = false;
    if (inPeriodic()) {
        M5_LIB_LOGD("Periodic measurements are running");
        return false;
    }
    uint16_t u16{};
    if (readRegister16(GET_AUTOMATIC_SELF_CALIBRATION_ENABLED, u16, 1)) {
        enabled = (u16 == 0x0001);
        return true;
    }
    return false;
}

bool UnitSCD40::startLowPowerPeriodicMeasurement(void) {
    if (inPeriodic()) {
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
    return readRegister16(GET_DATA_READY_STATUS, res, READ_MEASUREMENT_DURATION)
               ? (res & 0x07FF) != 0
               : false;
}

bool UnitSCD40::persistSettings() {
    if (inPeriodic()) {
        M5_LIB_LOGD("Periodic measurements are running");
        return false;
    }

    if (writeRegister(PERSIST_SETTINGS)) {
        m5::utility::delay(PERSIST_SETTINGS_DURATION);
        return true;
    }
    return false;
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

bool UnitSCD40::getSerialNumber(uint64_t& serialNumber) {
    std::array<uint8_t, 9> rbuf;
    serialNumber = 0;
    if (inPeriodic()) {
        M5_LIB_LOGD("Periodic measurements are running");
        return false;
    }

    if (readRegister(GET_SERIAL_NUMBER, rbuf.data(), rbuf.size(),
                     GET_SERIAL_NUMBER_DURATION)) {
        utility::ReadDataWithCRC16 data(rbuf.data(), 3);
        bool valid[3] = {data.valid(0), data.valid(1), data.valid(2)};
        if (valid[0] && valid[1] && valid[2]) {
            for (uint_fast8_t i = 0; i < 3; ++i) {
                serialNumber |= ((uint64_t)data.value(i)) << (16U * (2 - i));
            }
            return true;
        }
    }
    return false;
}

bool UnitSCD40::performSelfTest(bool& malfunction) {
    if (inPeriodic()) {
        M5_LIB_LOGD("Periodic measurements are running");
        return false;
    }

    uint16_t response{};
    if (readRegister16(PERFORM_SELF_TEST, response,
                       PERFORM_SELF_TEST_DURATION)) {
        malfunction = (response != 0);
        return true;
    }
    return false;
}

bool UnitSCD40::performFactoryReset() {
    if (inPeriodic()) {
        M5_LIB_LOGD("Periodic measurements are running");
        return false;
    }
    if (writeRegister(PERFORM_FACTORY_RESET)) {
        m5::utility::delay(PERFORM_FACTORY_RESET_DURATION);
        return true;
    }
    return false;
}

bool UnitSCD40::reInit() {
    if (inPeriodic()) {
        M5_LIB_LOGD("Periodic measurements are running");
        return false;
    }

    if (writeRegister(REINIT)) {
        m5::utility::delay(REINIT_DURATION);
        return true;
    }
    return false;
}

// RHT only if false
bool UnitSCD40::read_measurement(const bool all) {
    _co2         = 0;
    _temperature = _humidity = std::numeric_limits<float>::quiet_NaN();

    std::array<uint8_t, 9> rbuf{};
    if (!readRegister(READ_MEASUREMENT, rbuf.data(), rbuf.size(),
                      READ_MEASUREMENT_DURATION)) {
        return false;
    }

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
}

// class UnitSCD41
const char UnitSCD41::name[] = "UnitSCD41";
const types::uid_t UnitSCD41::uid{"UnitSCD41"_mmh3};
const types::uid_t UnitSCD41::attr{0};

bool UnitSCD41::measureSingleShot(void) {
    if (inPeriodic()) {
        M5_LIB_LOGD("Periodic measurements are running");
        return false;
    }
    return writeRegister(MEASURE_SINGLE_SHOT) && readMeasurement();
}

bool UnitSCD41::measureSingleShotRHTOnly(void) {
    if (inPeriodic()) {
        M5_LIB_LOGD("Periodic measurements are running");
        return false;
    }
    return writeRegister(MEASURE_SINGLE_SHOT) && readMeasurement();
}

}  // namespace unit
}  // namespace m5

// TODO
// Comapti test old and new
