/*!
  @file unit_SGP30.cpp
  @brief SGP30 Unit for M5UnitUnified

  SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD

  SPDX-License-Identifier: MIT
*/
#include "unit_SGP30.hpp"
#include <M5Utility.hpp>
#include <array>
#include <cmath>

using namespace m5::utility::mmh3;
using namespace m5::unit::types;
using namespace m5::unit::sgp30;
using namespace m5::unit::sgp30::command;

namespace {
// Supported lower limit version
constexpr uint8_t lower_limit_version{0x20};

// Max measurement duration (ms)
constexpr uint16_t IAQ_INIT_DURATION{10};
constexpr uint16_t MEASURE_IAQ_DURATION{12};
constexpr uint16_t GET_IAQ_BASELINE_DURATION{10};
constexpr uint16_t SET_IAQ_BASELINE_DURATION{10};
constexpr uint16_t SET_ABSOLUTE_HUMIDITY_DURATION{10};
constexpr uint16_t MEASURE_TEST_DURATION{220};
constexpr uint16_t GET_FEATURE_SET_DURATION{10};
constexpr uint16_t MEASURE_RAW_DURATION{25};
constexpr uint16_t GET_TVOC_INCEPTIVE_BASELINE_DURATION{10};
constexpr uint16_t SET_TVOC_INCEPTIVE_BASELINE_DURATION{10};
constexpr uint16_t GET_SERIAL_ID_DURATION{1};
constexpr elapsed_time_t BASELINE_INTERVAL{1000 * 60 * 60};  // 1 hour (ms)

bool delayMeasurementDuration(const uint16_t ms) {
    m5::utility::delay(ms);
    return true;
}
}  // namespace

namespace m5 {
namespace unit {
// class UnitSGP30
const char UnitSGP30::name[] = "UnitSGP30";
const types::uid_t UnitSGP30::uid{"UnitSGP30"_mmh3};
const types::uid_t UnitSGP30::attr{0};

bool UnitSGP30::begin() {
    Feature f{};

    if (!getFeatureSet(f)) {
        M5_LIB_LOGE("Failed to read feature");
        return false;
    }
    if (f.productType() != 0) {
        // May be SGPC3 gas sensor if value is 1
        M5_LIB_LOGE("This unit is NOT SGP30");
        return false;
    }
    _version = f.productVersion();
    if (_version < lower_limit_version) {
        M5_LIB_LOGE("Not enough the product version %x", _version);
        return false;
    }
    return _cfg.start_periodic
               ? startPeriodicMeasurement(_cfg.baseline_co2eq,
                                          _cfg.baseline_tvoc, _cfg.humidity)
               : true;
}

void UnitSGP30::update(const bool force) {
    if (_periodic) {
        elapsed_time_t at{m5::utility::millis()};
        if (force || !_latest || at >= _latest + _interval) {
            _interval = 1000;  // 1sec
            _updated  = readMeasurement(_CO2eq, _TVOC);
            if (_updated) {
                _latest = at;
            }
        } else {
            _updated = false;
        }

        // Store baseline values every hour
        if (_interval == 1000 &&
            (!_latestBaseline || at >= _latestBaseline + BASELINE_INTERVAL)) {
            _updatedBaseline = getIaqBaseline(_baselineCO2eq, _baselineTVOC);
            if (_updatedBaseline) {
                _latestBaseline = at;
            }
        } else {
            _updatedBaseline = false;
        }
    }
}

bool UnitSGP30::startPeriodicMeasurement(const uint16_t co2eq,
                                         const uint16_t tvoc,
                                         const uint16_t humidity) {
    if (inPeriodic()) {
        return false;
    }

    if (start_periodic_measurement() && set_iaq_baseline(co2eq, tvoc) &&
        setAbsoluteHumidity(humidity)) {
        return true;
    }
    _periodic = false;
    _latest   = 0;
    return false;
}

bool UnitSGP30::stopPeriodicMeasurement() {
    _periodic = false;
    return true;
}

bool UnitSGP30::readMeasurement(uint16_t& co2eq, uint16_t& tvoc) {
    std::array<uint8_t, 6> rbuf{};
    if (readRegister(MEASURE_IAQ, rbuf.data(), rbuf.size(),
                     MEASURE_IAQ_DURATION)) {
        utility::ReadDataWithCRC16 data(rbuf.data(), 2);
        bool valid[2] = {data.valid(0), data.valid(1)};
        if (valid[0] && valid[1]) {
            co2eq = data.value(0);
            tvoc  = data.value(1);
            return true;
        }
    }
    return false;
}

bool UnitSGP30::readRaw(uint16_t& h2, uint16_t& etoh) {
    std::array<uint8_t, 6> rbuf{};
    if (readRegister(MEASURE_RAW, rbuf.data(), rbuf.size(),
                     MEASURE_RAW_DURATION)) {
        utility::ReadDataWithCRC16 data(rbuf.data(), 2);
        bool valid[2] = {data.valid(0), data.valid(1)};
        if (valid[0] && valid[1]) {
            h2   = data.value(0);
            etoh = data.value(1);
            return true;
        }
    }
    return false;
}

bool UnitSGP30::getIaqBaseline(uint16_t& co2eq, uint16_t& tvoc) {
    std::array<uint8_t, 6> rbuf{};
    if (readRegister(GET_IAQ_BASELINE, rbuf.data(), rbuf.size(),
                     GET_IAQ_BASELINE_DURATION)) {
        utility::ReadDataWithCRC16 data(rbuf.data(), 2);
        bool valid[2] = {data.valid(0), data.valid(1)};
        if (valid[0] && valid[1]) {
            co2eq = data.value(0);
            tvoc  = data.value(1);
            return true;
        }
    }
    return false;
}

bool UnitSGP30::setAbsoluteHumidity(const uint16_t raw) {
    m5::utility::CRC8_Checksum crc8;
    std::array<uint8_t, 3> buf{};
    m5::types::big_uint16_t rr(raw);
    std::memcpy(buf.data(), rr.data(), 2);
    buf[2] = crc8.update(rr.data(), 2);
    return writeRegister(SET_ABSOLUTE_HUMIDITY, buf.data(), buf.size()) &&
           delayMeasurementDuration(SET_ABSOLUTE_HUMIDITY_DURATION);
}

bool UnitSGP30::setAbsoluteHumidity(const float gm3) {
    int32_t tmp = static_cast<int32_t>(std::round(gm3 * 256.f));
    if (tmp > 32767 || tmp < -32768) {
        M5_LIB_LOGE("Over/underflow: %f / %d", gm3, tmp);
        return false;
    }
    return setAbsoluteHumidity(
        static_cast<uint16_t>(static_cast<int16_t>(tmp)));
}

bool UnitSGP30::measureTest(uint16_t& result) {
    if (inPeriodic()) {
        M5_LIB_LOGD("Periodic measurements are running");
        return false;
    }

    std::array<uint8_t, 3> rbuf{};
    if (readRegister(MEASURE_TEST, rbuf.data(), rbuf.size(),
                     MEASURE_TEST_DURATION)) {
        utility::ReadDataWithCRC16 data(rbuf.data(), 1);
        if (data.valid(0)) {
            result = data.value(0);
            return true;
        }
    }
    return false;
}

bool UnitSGP30::getTvocInceptiveBaseline(uint16_t& tvoc) {
    if (_version < 0x21) {
        M5_LIB_LOGE("Not enough the product version %x", _version);
        return false;
    }
    std::array<uint8_t, 3> rbuf{};
    if (readRegister(GET_TVOC_INCEPTIVE_BASELINE, rbuf.data(), rbuf.size(),
                     GET_TVOC_INCEPTIVE_BASELINE_DURATION)) {
        utility::ReadDataWithCRC16 data(rbuf.data(), 1);
        if (data.valid(0)) {
            tvoc = data.value(0);
            return true;
        }
    }
    return false;
}

bool UnitSGP30::setTvocInceptiveBaseline(const uint16_t tvoc) {
    if (_version < 0x21) {
        M5_LIB_LOGE("Not enough the product version %x", _version);
        return false;
    }
    m5::utility::CRC8_Checksum crc8;
    std::array<uint8_t, 3> buf{};
    m5::types::big_uint16_t tt(tvoc);
    std::memcpy(buf.data(), tt.data(), 2);
    buf[2] = crc8.update(tt.data(), 2);
    return writeRegister(SET_TVOC_INCEPTIVE_BASELINE, buf.data(), buf.size()) &&
           delayMeasurementDuration(SET_TVOC_INCEPTIVE_BASELINE_DURATION);
}

bool UnitSGP30::generalReset() {
    uint8_t cmd{0x06};
    if (generalCall(&cmd, 1)) {
        _periodic = false;
        _latest = _latestBaseline = 0;
        m5::utility::delay(10);
        return true;
    }
    return false;
}

bool UnitSGP30::getFeatureSet(sgp30::Feature& feature) {
    std::array<uint8_t, 3> rbuf{};
    if (readRegister(GET_FEATURE_SET, rbuf.data(), rbuf.size(),
                     GET_FEATURE_SET_DURATION)) {
        utility::ReadDataWithCRC16 data(rbuf.data(), 1);
        if (data.valid(0)) {
            feature.value = data.value(0);
            return true;
        }
    }
    return false;
}

bool UnitSGP30::getSerialNumber(uint64_t& number) {
    std::array<uint8_t, 9> rbuf{};
    if (readRegister(GET_SERIAL_ID, rbuf.data(), rbuf.size(),
                     GET_SERIAL_ID_DURATION)) {
        utility::ReadDataWithCRC16 data(rbuf.data(), 3);
        bool valid[3] = {data.valid(0), data.valid(1), data.valid(2)};
        if (valid[0] && valid[1] && valid[2]) {
            for (uint_fast8_t i = 0; i < 3; ++i) {
                number |= ((uint64_t)data.value(i)) << (16U * (2 - i));
            }
            return true;
        }
    }
    return false;
}

bool UnitSGP30::getSerialNumber(char* number) {
    if (!number) {
        return false;
    }

    *number = '\0';
    uint64_t sno{};
    if (getSerialNumber(sno)) {
        uint_fast8_t i{12};
        while (i--) {
            *number++ = m5::utility::uintToHexChar((sno >> (i * 4)) & 0x0F);
        }
        *number = '\0';
        return true;
    }
    return false;
}

//

bool UnitSGP30::start_periodic_measurement() {
    if (writeRegister(IAQ_INIT)) {
        m5::utility::delay(IAQ_INIT_DURATION);
        // For the first 15s after the “sgp30_iaq_init” command the sensor
        // is an initialization phase during which a “sgp30_measure_iaq”
        // command returns fixed values of 400 ppm CO2eq and 0 ppb TVOC.A
        // new “sgp30_iaq_init” command has to be sent after every power-up
        // or soft reset.

        // Baseline and absolute humidity restoration must take place during
        // this 15-second period
        _periodic = true;
        _latest   = m5::utility::millis();
        _interval = 15 * 1000;  // 15sec for first measurement

        return true;
    }
    return false;
}

bool UnitSGP30::set_iaq_baseline(const uint16_t co2eq, const uint16_t tvoc) {
    m5::utility::CRC8_Checksum crc8;
    m5::types::big_uint16_t cc(co2eq);
    m5::types::big_uint16_t tt(tvoc);

    std::array<uint8_t, (2 + 1) * 2> buf{};
    // Note that the order is different for get and set
    std::memcpy(buf.data() + 0, tt.data(), 2);
    buf[2] = crc8.update(tt.data(), 2);
    std::memcpy(buf.data() + 3, cc.data(), 2);
    buf[5] = crc8.update(cc.data(), 2);

    return writeRegister(SET_IAQ_BASELINE, buf.data(), buf.size()) &&
           delayMeasurementDuration(SET_IAQ_BASELINE_DURATION);
}

}  // namespace unit
}  // namespace m5
