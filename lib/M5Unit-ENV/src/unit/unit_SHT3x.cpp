/*!
  @file unit_SHT3x.cpp
  @brief SHT3x family Unit for M5UnitUnified

  SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD

  SPDX-License-Identifier: MIT
*/
#include "unit_SHT3x.hpp"
#include <M5Utility.hpp>
#include <limits>  // NaN
#include <array>

using namespace m5::utility::mmh3;

namespace {
struct Temperature {
    constexpr static float toFloat(const uint16_t u16) {
        return -45 + u16 * 175 / 65535.f;  // -45 + 175 * S / (2^16 - 1)
    }
};

// After sending a command to the sensor a minimalwaiting time of 1ms is needed
// before another commandcan be received by the sensor.
bool delay1() {
    m5::utility::delay(1);
    return true;
}

}  // namespace

namespace m5 {
namespace unit {

using namespace sht3x;
using namespace sht3x::command;

const char UnitSHT30::name[] = "UnitSHT30";
const types::uid_t UnitSHT30::uid{"UnitSHT30"_mmh3};
const types::uid_t UnitSHT30::attr{0};

bool UnitSHT30::begin() {
    if (!stopPeriodicMeasurement()) {
        M5_LIB_LOGE("Failed to stop");
        return false;
    }

    if (!softReset()) {
        M5_LIB_LOGE("Failed to reset");
        return false;
    }

    auto r = _cfg.start_heater ? startHeater() : stopHeater();
    if (!r) {
        M5_LIB_LOGE("Failed to heater %d", _cfg.start_heater);
        return false;
    }
    return _cfg.start_periodic ? startPeriodicMeasurement(_cfg.mps, _cfg.rep)
                               : true;
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

    uint32_t idx = m5::stl::to_underlying(rep) + (stretch ? 0 : 3);
    if (idx >= m5::stl::size(cmd)) {
        M5_LIB_LOGE("Invalid arg : %u", (int)rep);
        return false;
    }

    if (writeRegister(cmd[idx])) {
        m5::utility::delay(stretch ? 1 : ms[m5::stl::to_underlying(rep)]);
        return read_measurement();
    }
    return false;
}

bool UnitSHT30::startPeriodicMeasurement(const sht3x::MPS mps,
                                         const sht3x::Repeatability rep) {
    constexpr static uint16_t periodic_cmd[] = {
        // 0.5 mps
        START_PERIODIC_MPS_HALF_HIGH,
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
        // 4 mps
        START_PERIODIC_MPS_4_HIGH,
        START_PERIODIC_MPS_4_MEDIUM,
        START_PERIODIC_MPS_4_LOW,
        // 10 mps
        START_PERIODIC_MPS_10_HIGH,
        START_PERIODIC_MPS_10_MEDIUM,
        START_PERIODIC_MPS_10_LOW,
    };
    constexpr static unsigned long interval_table[] = {
        2000,  // 0.5
        1000,  // 1
        500,   // 2
        250,   // 4
        100,   // 10
    };

    if (inPeriodic()) {
        M5_LIB_LOGD("Periodic measurements are running");
        return false;
    }

    _periodic = writeRegister(periodic_cmd[m5::stl::to_underlying(mps) * 3 +
                                           m5::stl::to_underlying(rep)]);
    if (_periodic) {
        _interval = interval_table[m5::stl::to_underlying(mps)];
        m5::utility::delay(16);
        return true;
    }
    return _periodic;
}

bool UnitSHT30::stopPeriodicMeasurement() {
    if (writeRegister(STOP_PERIODIC_MEASUREMENT)) {
        _periodic = false;
        // Upon reception of the break command the sensor will abort the ongoing
        // measurement and enter the single shot mode. This takes 1ms
        return delay1();
    }
    return false;
}

bool UnitSHT30::readMeasurement() {
    if (!inPeriodic()) {
        M5_LIB_LOGD("Periodic measurements are NOT running");
        return false;
    }

    if (!writeRegister(READ_MEASUREMENT)) {
        return false;
    }
    m5::utility::delay(1);
    return read_measurement();
}

bool UnitSHT30::accelerateResponseTime() {
    if (writeRegister(ACCELERATED_RESPONSE_TIME)) {
        _interval = 1000 / 4;  // 4mps
        m5::utility::delay(16);
        return true;
    }
    return false;
}

bool UnitSHT30::getStatus(sht3x::Status& s) {
    std::array<uint8_t, 3> rbuf{};
    if (readRegister(READ_STATUS, rbuf.data(), rbuf.size(), 0)) {
        utility::ReadDataWithCRC16 data(rbuf.data(), 1);
        if (data.valid(0)) {
            s.value = data.value(0);
            return true;
        }
    }
    return false;
}

bool UnitSHT30::clearStatus() {
    return writeRegister(CLEAR_STATUS) && delay1();
}

bool UnitSHT30::softReset() {
    if (inPeriodic()) {
        M5_LIB_LOGE("Periodic measurements are running");
        return false;
    }

    if (writeRegister(SOFT_RESET)) {
        // Max 1.5 ms
        // Time between ACK of soft reset command and sensor entering idle state
        m5::utility::delay(2);
        return true;
    }
    return false;
}

bool UnitSHT30::generalReset() {
    uint8_t cmd{0x06};  // reset command

    if (!clearStatus()) {
        return false;
    }
    // Reset does not return ACK, which is an error, but should be ignored
    generalCall(&cmd, 1);

    m5::utility::delay(1);
    
    auto timeout_at = m5::utility::millis() + 10;
    bool done{};
    do {
        Status s{};
        // The ALERT pin will also become active (high) after powerup and after
        // resets
        if (getStatus(s) && (s.reset() || s.alertPending())) {
            done = true;
            break;
        }
        m5::utility::delay(1);
    } while (!done && m5::utility::millis() <= timeout_at);
    return done;
}

bool UnitSHT30::startHeater() {
    return writeRegister(START_HEATER) && delay1();
}

bool UnitSHT30::stopHeater() {
    return writeRegister(STOP_HEATER) && delay1();
}

bool UnitSHT30::getSerialNumber(uint32_t& serialNumber) {
    serialNumber = 0;
    if (inPeriodic()) {
        M5_LIB_LOGE("Periodic measurements are running");
        return false;
    }

    if (!writeRegister(GET_SERIAL_NUMBER_ENABLE_STRETCH)) {
        return false;
    }

    m5::utility::delay(1);

    std::array<uint8_t, 6> rbuf;
    if (readWithTransaction(rbuf.data(), rbuf.size()) ==
        m5::hal::error::error_t::OK) {
        utility::ReadDataWithCRC16 data(rbuf.data(), 2);
        bool valid[2] = {data.valid(0), data.valid(1)};
        if (valid[0] && valid[1]) {
            for (uint_fast8_t i = 0; i < 2; ++i) {
                serialNumber |= ((uint32_t)data.value(i)) << (16U * (1 - i));
            }
            return true;
        }
    }
    return false;
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

    _temperature = _humidity = std::numeric_limits<float>::quiet_NaN();

    if (readWithTransaction(rbuf.data(), rbuf.size()) ==
        m5::hal::error::error_t::OK) {
        utility::ReadDataWithCRC16 data(rbuf.data(), 2);
        bool valid[2] = {data.valid(0), data.valid(1)};
        if (valid[0]) {
            this->_temperature = Temperature::toFloat(data.value(0));
        }
        if (valid[1]) {
            this->_humidity = 100 * data.value(1) / 65536.f;
        }
        return valid[0] && valid[1];
    }
    return false;
}

}  // namespace unit
}  // namespace m5
