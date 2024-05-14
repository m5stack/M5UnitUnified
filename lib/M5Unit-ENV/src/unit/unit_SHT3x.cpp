/*!
  @file unit_SHT3x.cpp
  @brief SHT3x family Unit for M5UnitUnified

  @copyright M5Stack. All rights reserved.
  @license Licensed under the MIT license. See LICENSE file in the project root
  for full license information.
*/
#include "unit_SHT3x.hpp"
#include <M5Utility.hpp>

using namespace m5::utility::mmh3;

namespace {
struct Temperature {
    constexpr static float toFloat(const uint16_t u16) {
        return -45 + u16 * 175 / 65535.f;  // -45 + 175 * S / (2^16 - 1)
    }
};

bool delay1() {
    m5::utility::delay(1);
    return true;
}

}  // namespace

namespace m5 {
namespace unit {

using namespace sht3x::command;

const char UnitSHT30::name[] = "UnitSHT30";
const types::uid_t UnitSHT30::uid{"UnitSHT30"_mmh3};
const types::uid_t UnitSHT30::attr{0};

bool UnitSHT30::begin() {
    auto r = stopPeriodicMeasurement();
    if (!r) {
        M5_LIB_LOGD("Failed to stop");
        return false;
    }
    return startPeriodicMeasurement();
}

void UnitSHT30::update() {
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

bool UnitSHT30::measurementSingleShot(const sht3x::Repeatability rep,
                                      const bool stretch) {
    constexpr uint16_t cmd[] = {
        // Enable clock stretching
        SINGLE_SHOT_ENABLE_STRETCH_HIGH,
        SINGLE_SHOT_ENABLE_STRETCH_MEDIUM,
        SINGLE_SHOT_ENABLE_STRETCH_LOW,
        // Disable clock stretching
        SINGLE_SHOT_DISABLE_STRETCH_HIGH,
        SINGLE_SHOT_DISABLE_STRETCH_MEDIUM,
        SINGLE_SHOT_DISABLE_STRETCH_LOW,
    };
    // Latency when clock stretching is disabled
    constexpr unsigned long ms[] = {
        15,
        6,
        4,
    };

    if (inPeriodic()) {
        M5_LIB_LOGD("Periodic measurements are running");
        return false;
    }
    if (!writeRegister(cmd[m5::stl::to_underlying(rep) + (stretch ? 0 : 3)])) {
        return false;
    }
    if (!stretch) {
        m5::utility::delay(ms[m5::stl::to_underlying(rep)]);
    }
    return read_measurement();
}

bool UnitSHT30::startPeriodicMeasurement(const sht3x::MPS mps,
                                         const sht3x::Repeatability rep) {
    constexpr uint16_t cmd[] = {
        // 0.5 mps
        START_PERIODIC_MPS_HALF_HIGHT,
        START_PERIODIC_MPS_HALF_MEDIUM,
        START_PERIODIC_MPS_HALF_LOW,
        // 1 mps
        START_PERIODIC_MPS_1_HIGH,
        START_PERIODIC_MPS_1_MEDIUM,
        START_PERIODIC_MPS_1_LOW,
        // 2 mps
        START_PERIODIC_MPS_2_HIGH,
        START_PERIODIC_MPS_2_MEDIUM,
        START_PERIODIC_MPS_2_LOW,
        // 10 mps
        START_PERIODIC_MPS_10_HIGH,
        START_PERIODIC_MPS_10_MEDIUM,
        START_PERIODIC_MPS_10_LOW,
    };
    constexpr unsigned long ms[] = {
        2000,  // 0.5
        1000,  // 1
        500,   // 2
        100,   // 10
    };

    if (inPeriodic()) {
        M5_LIB_LOGD("Periodic measurements are running");
        return false;
    }

    _periodic = writeRegister(
        cmd[m5::stl::to_underlying(mps) * 3 + m5::stl::to_underlying(rep)]);
    if (_periodic) {
        _interval = ms[m5::stl::to_underlying(mps)];
        delay1();
    }
    return _periodic;
}

bool UnitSHT30::stopPeriodicMeasurement() {
    if (writeRegister(STOP_PERIODIC_MEASUREMENT)) {
        _periodic = false;
        delay1();
        return true;
    }
    return false;
}

bool UnitSHT30::readMeasurement() {
    if (!inPeriodic()) {
        M5_LIB_LOGD("Periodic measurements are NOT running");
        return false;
    }
    if (!writeRegister(READ_MEASUREMENT)) return false;
    return read_measurement() && delay1();
}

bool UnitSHT30::accelerateResponseTime() {
    if (writeRegister(ACCELERATED_RESPONSE_TIME)) {
        _interval = 1000 / 4;  // 4mps
        delay1();
        return true;
    }
    return false;
}

bool UnitSHT30::getStatus(sht3x::Status& s) {
    s.value = 0;
    return readRegister16(READ_STATUS, s.value, 1);
}

bool UnitSHT30::clearStatus() {
    return writeRegister(CLEAR_STATUS) && delay1();
}

bool UnitSHT30::softReset() {
    if (inPeriodic()) {
        M5_LIB_LOGD("Periodic measurements are running");
        return false;
    }
    return writeRegister(SOFT_RESET) && delay1();
}

#if 0
bool UnitSHT30::generalReset() {
    // TODO
    // How to use generic call with Wire/M5Bus?
    //constexpr uint16_t GENERAL_RESET{0x0006};
    if (inPeriodic()) {
        M5_LIB_LOGD("Periodic measurements are running");
        return false;
    }
    return false;
}
#endif

bool UnitSHT30::startHeater() {
    return writeRegister(START_HEATER) && delay1();
}

bool UnitSHT30::stopHeater() {
    return writeRegister(STOPE_HEATER) && delay1();
}

bool UnitSHT30::getSerialNumber(uint32_t& serialNumber) {
    serialNumber = 0;
    if (inPeriodic()) {
        M5_LIB_LOGD("Periodic measurements are running");
        return false;
    }
    if (!writeRegister(GET_SEREAL_NUMBER_ENABLE_STRETCH)) {
        return false;
    }

    // m5::utility::delay(1);

    std::array<uint8_t, 6> rbuf;
    return readWithTransaction(
        rbuf.data(), rbuf.size(), [this, &rbuf, &serialNumber]() {
            utility::ReadDataWithCRC16 data(rbuf.data(), 3);
            bool valid[2] = {data.valid(0), data.valid(1)};
            if (valid[0] && valid[1]) {
                for (uint_fast8_t i = 0; i < 3; ++i) {
                    serialNumber |= ((uint32_t)data.value(i >> 1))
                                    << (16U * (1 - i));
                }
                return true;
            }
            return false;
        });
}

bool UnitSHT30::getSerialNumber(char* serialNumber) {
    if (!serialNumber) {
        return false;
    }

    *serialNumber = '\0';
    uint32_t sno{};
    if (getSerialNumber(sno)) {
        uint_fast8_t i{8};
        while (i--) {
            *serialNumber++ =
                m5::utility::uintToHexChar((sno >> (i * 4)) & 0x0F);
        }
        *serialNumber = '\0';
        return true;
    }
    return false;
}

bool UnitSHT30::read_measurement() {
    std::array<uint8_t, 6> rbuf{};
    return readWithTransaction(rbuf.data(), rbuf.size(), [this, &rbuf] {
        utility::ReadDataWithCRC16 data(rbuf.data(), 2);
        bool valid[2] = {data.valid(0), data.valid(1)};
        if (valid[0]) {
            this->_temperature = Temperature::toFloat(data.value(0));
        }
        if (valid[1]) {
            this->_humidity = 100 * data.value(1) / 65536.f;
        }
        return valid[0] && valid[1];
    });
}

}  // namespace unit
}  // namespace m5
