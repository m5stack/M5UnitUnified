/*!
  @file unit_SCD4x.cpp
  @brief SCD4X family Unit for M5UnitUnified

  @copyright M5Stack. All rights reserved.
  @license Licensed under the MIT license. See LICENSE file in the project root
  for full license information.
*/
#include "unit_SCD4x.hpp"

using namespace m5::utility::mmh3;

namespace {
struct Temperature {
    constexpr static float toFloat(const uint16_t u16) {
        return u16 * 175 / 65536.f;
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
    auto r = stopPeriodicMeasurement();
    if(!r) { M5_LIB_LOGD("Failed to stop"); return false; }

    return startPeriodicMeasurement();
}

void UnitSCD40::update() {
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

bool UnitSCD40::startPeriodicMeasurement(void) {
    if (_periodic) {
        M5_LIB_LOGD("Periodic measurements are running");
        return false;
    }
    _periodic = sendCommand(SCD4x_COMMAND_START_PERIODIC_MEASUREMENT);
    if (_periodic) {
        _interval = SIGNAL_INTERVAL_MS;
    }
    return _periodic;
}

bool UnitSCD40::stopPeriodicMeasurement(const uint16_t delayMillis) {
    if (sendCommand(SCD4x_COMMAND_STOP_PERIODIC_MEASUREMENT)) {
        _periodic = false;
        m5::utility::delay(delayMillis);
        return true;
    }
    return false;
}

bool UnitSCD40::readMeasurement(void) {
    if (!getDataReadyStatus()) {
        M5_LIB_LOGD("Not ready");
        return false;
    }
    if (!sendCommand(SCD4x_COMMAND_READ_MEASUREMENT)) {
        return false;
    }

    m5::utility::delay(1);

    std::array<uint8_t, 9> rbuf{};
    return readWithTransaction(rbuf.data(), rbuf.size(), [this, &rbuf] {
        m5::types::big_uint16_t co2{}, temp{}, humidity{};
        bool valid_co2{}, valid_temp{}, valid_humidity{};
        m5::utility::CRC8_Maxim crc;
        uint_fast8_t idx{};
        for (auto&& e : rbuf) {
            switch (idx) {
                case 0:
                case 1:
                    co2.u8[idx] = e;
                    break;
                case 2:
                    valid_co2 = crc.get(co2.data(), co2.size()) == e;
                    break;
                case 3:
                case 4:
                    temp.u8[idx - 3] = e;
                    break;
                case 5:
                    valid_temp = crc.get(temp.data(), temp.size()) == e;
                    break;
                case 6:
                case 7:
                    humidity.u8[idx - 6] = e;
                    break;
                case 8:
                    valid_humidity =
                        crc.get(humidity.data(), humidity.size()) == e;
                    break;
                default:
                    break;
            }
            ++idx;
        }
        if (valid_co2) {
            this->_co2 = co2.get();
        }
        if (valid_temp) {
            this->_temperature = -45 + Temperature::toFloat(temp.get());
        }
        if (valid_humidity) {
            this->_humidity = 100 * humidity.get() / 65536.f;
        }
        return valid_co2 && valid_temp && valid_humidity;
    });
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
    uint16_t u16 = Temperature::toUint16(offset);
    auto ret     = sendCommand(SCD4x_COMMAND_SET_TEMPERATURE_OFFSET, u16);
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
    auto ret = readRegister(SCD4x_COMMAND_GET_TEMPERATURE_OFFSET, u16, 1);
    offset   = Temperature::toFloat(u16);
    return ret;
}

bool UnitSCD40::setSensorAltitude(const uint16_t altitude,
                                  const uint16_t delayMillis) {
    if (_periodic) {
        M5_LIB_LOGD("Periodic measurements are running");
        return false;
    }
    auto ret = sendCommand(SCD4x_COMMAND_SET_SENSOR_ALTITUDE, altitude);
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
    return readRegister(SCD4x_COMMAND_GET_SENSOR_ALTITUDE, altitude, 1);
}

bool UnitSCD40::setAmbientPressure(const float pressure,
                                   const uint16_t delayMillis) {
    if (pressure < 0.0f || pressure > 65535.f * 100) {
        M5_LIB_LOGE("pressure is not a valid scope %f", pressure);
        return false;
    }
    uint16_t u16 = (uint16_t)(pressure / 100);
    auto ret     = sendCommand(SCD4x_COMMAND_SET_AMBIENT_PRESSURE, u16);
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
    if (!sendCommand(SCD4x_COMMAND_PERFORM_FORCED_CALIBRATION, concentration)) {
        return false;
    }

    // 3. Subsequently issue the perform_forced_recalibration command and
    // optionally read out the FRC correction (i.e. the magnitude of the
    // correction) after waiting for 400 ms for the command to complete.
    m5::utility::delay(400);

    std::array<uint8_t, 3> buf{};
    if (!readWithTransaction(buf.data(), buf.size(), [this, &buf] {
            m5::utility::CRC8_Maxim crc;
            return crc.get(buf.data(), 2) == buf[2];
        })) {
        return false;
    }
    m5::types::big_uint16_t ub16{buf[0], buf[1]};
    correction = (int16_t)(ub16.get() - 0x8000);
    return (ub16.get() != 0xFFFF);
}

bool UnitSCD40::setAutomaticSelfCalibrationEnabled(const bool enabled,
                                                   const uint16_t delayMillis) {
    if (_periodic) {
        M5_LIB_LOGD("Periodic measurements are running");
        return false;
    }
    auto ret = sendCommand(SCD4x_COMMAND_SET_AUTOMATIC_SELF_CALIBRATION_ENABLED,
                           enabled ? 1 : 0);
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
    auto ret = readRegister(
        SCD4x_COMMAND_GET_AUTOMATIC_SELF_CALIBRATION_ENABLED, u16, 1);
    enabled = (u16 == 0x0001);
    return ret;
}

bool UnitSCD40::startLowPowerPeriodicMeasurement(void) {
    if (_periodic) {
        M5_LIB_LOGD("Periodic measurements are running");
        return false;
    }
    _periodic = sendCommand(SCD4x_COMMAND_START_LOW_POWER_PERIODIC_MEASUREMENT);
    if (_periodic) {
        _interval = SIGNAL_INTERVAL_LOW_MS;
    }
    return _periodic;
}

bool UnitSCD40::getDataReadyStatus() {
    uint16_t res{};
    return readRegister(SCD4x_COMMAND_GET_DATA_READY_STATUS, res, 1)
               ? (res & 0x07FF) != 0
               : false;
}

bool UnitSCD40::persistSettings(uint16_t delayMillis) {
    if (_periodic) {
        M5_LIB_LOGD("Periodic measurements are running");
        return false;
    }

    auto ret = sendCommand(SCD4x_COMMAND_PERSIST_SETTINGS);
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
            *serialNumber++ = m5::utility::uintToHex((sno >> (i * 4)) & 0x0F);
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
    if (!sendCommand(SCD4x_COMMAND_GET_SERIAL_NUMBER)) {
        return false;
    }

    m5::utility::delay(1);

    std::array<uint8_t, 9> buf;
    return readWithTransaction(
        buf.data(), buf.size(), [this, &buf, &serialNumber]() {
            uint_fast8_t idx{};
            m5::types::big_uint16_t tmp{};
            m5::utility::CRC8_Maxim crc;
            for (auto&& e : buf) {
                switch (idx % 3) {
                    case 0:
                    case 1:
                        tmp.u8[idx % 3] = e;
                        break;
                    default:
                        if (crc.get(tmp.data(), 2) != e) {
                            return false;
                        }
                        serialNumber |=
                            ((uint64_t)tmp.get() << (2 - (idx / 3)) * 16U);
                        break;
                }
                ++idx;
            }
            return true;
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
    auto ret =
        readRegister(SCD4x_COMMAND_PERFORM_SELF_TEST, response, 10 * 1000);
    malfunction = (response != 0);
    return ret;
}

bool UnitSCD40::performFactoryReset(uint16_t delayMillis) {
    if (_periodic) {
        M5_LIB_LOGD("Periodic measurements are running");
        return false;
    }
    auto ret = sendCommand(SCD4x_COMMAND_PERFORM_FACTORY_RESET);
    m5::utility::delay(delayMillis);
    return ret;
}

bool UnitSCD40::reInit(uint16_t delayMillis) {
    if (_periodic) {
        M5_LIB_LOGD("Periodic measurements are running");
        return false;
    }

    auto ret = sendCommand(SCD4x_COMMAND_REINIT);
    m5::utility::delay(delayMillis);
    return ret;
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
    return sendCommand(SCD4x_COMMAND_MEASURE_SINGLE_SHOT);
}

bool UnitSCD41::measureSingleShotRHTOnly(void) {
    if (_periodic) {
        M5_LIB_LOGD("Periodic measurements are running");
        return false;
    }
    return sendCommand(SCD4x_COMMAND_MEASURE_SINGLE_SHOT_RHT_ONLY);
}

}  // namespace unit
}  // namespace m5

// TODO
// Comapti test old and new
