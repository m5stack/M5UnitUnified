/*!
  @file unit_SHT3x.cpp
  @brief SHT3x Unit for M5UnitUnified

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

}  // namespace

namespace m5 {
namespace unit {

using namespace sht3x::command;

const char UnitSHT3x::name[] = "UnitSHT3x";
const types::uid_t UnitSHT3x::uid{"UnitSHT3x"_mmh3};
const types::uid_t UnitSHT3x::attr{0};

bool UnitSHT3x::begin() {
    auto r = stopPeriodicMeasurement();
    if (!r) {
        M5_LIB_LOGD("Failed to stop");
        return false;
    }

#if 0
    UnitSHT3x::Status s{};
    r = readStatus(s);
    if (r) {
        M5_LIB_LOGI("Status at begin: %u/%u/%u/%u/%u/%u/%u", s.alertPending(),
                    s.heater(), s.trackingAlertRH(), s.trackingAlert(),
                    s.reset(), s.command(), s.checksum());
    } else {
        M5_LIB_LOGD("Failed to rad status");
    }

    startHeater();

    r = readStatus(s);
    if (r) {
        M5_LIB_LOGI("Status at begin: %u/%u/%u/%u/%u/%u/%u", s.alertPending(),
                    s.heater(), s.trackingAlertRH(), s.trackingAlert(),
                    s.reset(), s.command(), s.checksum());
    } else {
        M5_LIB_LOGD("Failed to rad status");
    }

    clearStatus();
    r = readStatus(s);
    if (r) {
        M5_LIB_LOGI("Status at begin: %u/%u/%u/%u/%u/%u/%u", s.alertPending(),
                    s.heater(), s.trackingAlertRH(), s.trackingAlert(),
                    s.reset(), s.command(), s.checksum());
    } else {
        M5_LIB_LOGD("Failed to rad status");
    }

    return startPeriodicMeasurement(MPS::Mps10, Repeatability::Medium) &&
           accelerateResponseTime();
#endif
    return startPeriodicMeasurement();
}

void UnitSHT3x::update() {
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

bool UnitSHT3x::measurementSingle(Repeatability rep, const bool stretch) {
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

    if (_periodic) {
        M5_LIB_LOGD("Periodic measurements are running");
        return false;
    }
    if (!sendCommand(cmd[m5::stl::to_underlying(rep) + (stretch ? 0 : 3)])) {
        return false;
    }
    if (!stretch) {
        m5::utility::delay(ms[m5::stl::to_underlying(rep)]);
    }
    return read_measurement();
}

bool UnitSHT3x::startPeriodicMeasurement(const MPS mps,
                                         const Repeatability rep) {
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

    if (_periodic) {
        M5_LIB_LOGD("Periodic measurements are running");
        return false;
    }

    _periodic = sendCommand(
        cmd[m5::stl::to_underlying(mps) * 3 + m5::stl::to_underlying(rep)]);
    if (_periodic) {
        _interval = ms[m5::stl::to_underlying(mps)];
    }
    return _periodic;
}

bool UnitSHT3x::stopPeriodicMeasurement() {
    if (sendCommand(STOP_PERIODIC_MEASUREMENT)) {
        _periodic = false;
        return true;
    }
    return false;
}

bool UnitSHT3x::readMeasurement() {
    if (!sendCommand(READ_MEASUREMENT)) return false;
    m5::utility::delay(1);
    return read_measurement();
}

bool UnitSHT3x::accelerateResponseTime() {
    if (sendCommand(ACCELERATED_RESPONSE_TIME)) {
        _interval = 1000 / 4;  // 4mps
        return true;
    }
    return false;
}

bool UnitSHT3x::readStatus(Status& s) {
    s.value = 0;
    return readRegister(READ_STATUS, s.value, 1);
}

bool UnitSHT3x::clearStatus() {
    return sendCommand(CLEAR_STATUS);
}

bool UnitSHT3x::softReset() {
    return sendCommand(SOFT_RESET);
}

bool UnitSHT3x::hardReset() {
    return sendCommand(GENERAL_RESET);
}

bool UnitSHT3x::startHeater() {
    return sendCommand(START_HEATER);
}

bool UnitSHT3x::stopHeater() {
    return sendCommand(STOPE_HEATER);
}

bool UnitSHT3x::read_measurement() {
    std::array<uint8_t, 6> rbuf{};
    return readWithTransaction(rbuf.data(), rbuf.size(), [this, &rbuf] {
        m5::utility::CRC8_Maxim crc;
        m5::types::big_uint16_t temp{}, humidity{};
        bool valid_temp{}, valid_humidity{};
        uint_fast8_t idx{};
        for (auto&& e : rbuf) {
            switch (idx) {
                case 0:
                case 1:
                    temp.u8[idx] = e;
                    break;
                case 2:
                    valid_temp = crc.get(temp.data(), temp.size()) == e;
                    break;
                case 3:
                case 4:
                    humidity.u8[idx - 3] = e;
                    break;
                case 5:
                    valid_humidity =
                        crc.get(humidity.data(), humidity.size()) == e;
                    break;
                default:
                    break;
            }
            ++idx;
        }
        if (valid_temp) {
            this->_temperature = Temperature::toFloat(temp.get());
        }
        if (valid_humidity) {
            this->_humidity = 100 * humidity.get() / 65536.f;
        }
        return valid_temp && valid_humidity;
    });
}

}  // namespace unit
}  // namespace m5
