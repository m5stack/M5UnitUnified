/*
 * SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
/*!
  @file unit_SGP30.cpp
  @brief SGP30 Unit for M5UnitUnified
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
constexpr elapsed_time_t BASELINE_INTERVAL{1000 * 60 * 60};  // 1 hour (ms)

inline bool delayMeasurementDuration(const uint16_t ms) {
    m5::utility::delay(ms);
    return true;
}
}  // namespace

namespace m5 {
namespace unit {

namespace sgp30 {
uint16_t Data::co2eq() const {
    return m5::types::big_uint16_t(raw[0], raw[1]).get();
}

uint16_t Data::tvoc() const {
    return m5::types::big_uint16_t(raw[3], raw[4]).get();
}

}  // namespace sgp30

// class UnitSGP30
const char UnitSGP30::name[] = "UnitSGP30";
const types::uid_t UnitSGP30::uid{"UnitSGP30"_mmh3};
const types::uid_t UnitSGP30::attr{0};

bool UnitSGP30::begin() {
    assert(_cfg.stored_size && "stored_size must be greater than zero");
    if (_cfg.stored_size != _data->capacity()) {
        _data.reset(new m5::container::CircularBuffer<Data>(_cfg.stored_size));
        if (!_data) {
            M5_LIB_LOGE("Failed to allocate");
            return false;
        }
    }

    Feature f{};
    if (!readFeatureSet(f)) {
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
    _updated         = false;
    if (_periodic) {
        elapsed_time_t at{m5::utility::millis()};
        if (at < _can_measure_time) {
            return;
        }
        if (force || !_latest || at >= _latest + _interval) {
            Data d{};
            _updated = read_measurement(d);
            if (_updated) {
                _latest = at;
                _data->push_back(d);
            }
#if 0
            uint16_t h2{}, etoh{};
            if (readRaw(h2, etoh)) {
                float hh   = 0.5f * std::exp( (13119 - h2)/ 512.f);
                float ee   = 0.4f * std::exp( (18472 - etoh) / 512.f);
                M5_LIB_LOGW("H:%f E:%f", hh, ee);
            }
#endif
        }
    }
}

bool UnitSGP30::startPeriodicMeasurement(const uint16_t co2eq,
                                         const uint16_t tvoc,
                                         const uint16_t humidity,
                                         const uint32_t duration) {
    if (inPeriodic()) {
        return false;
    }

    // Baseline and absolute humidity restoration must take place during
    // this 15-second period
    return start_periodic_measurement(duration) &&
           set_iaq_baseline(co2eq, tvoc) && setAbsoluteHumidity(humidity, 0U);
}

bool UnitSGP30::startPeriodicMeasurement(const uint32_t duration) {
    return start_periodic_measurement(duration);
}

bool UnitSGP30::stopPeriodicMeasurement() {
    _periodic = false;
    return true;
}

bool UnitSGP30::readRaw(uint16_t& h2, uint16_t& etoh) {
    std::array<uint8_t, 6> rbuf{};
    if (readRegister(MEASURE_RAW, rbuf.data(), rbuf.size(),
                     MEASURE_RAW_DURATION)) {
        m5::utility::CRC8_Checksum crc{};
        if (crc.range(rbuf.data(), 2) == rbuf[2] &&
            crc.range(rbuf.data() + 3, 2) == rbuf[5]) {
            h2   = m5::types::big_uint16_t(rbuf[0], rbuf[1]).get();
            etoh = m5::types::big_uint16_t(rbuf[3], rbuf[4]).get();
        }
        return true;
    }
    return false;
}

bool UnitSGP30::readIaqBaseline(uint16_t& co2eq, uint16_t& tvoc) {
    std::array<uint8_t, 6> rbuf{};
    if (readRegister(GET_IAQ_BASELINE, rbuf.data(), rbuf.size(),
                     GET_IAQ_BASELINE_DURATION)) {
        m5::utility::CRC8_Checksum crc{};
        if (crc.range(rbuf.data(), 2) == rbuf[2] &&
            crc.range(rbuf.data() + 3, 2) == rbuf[5]) {
            co2eq = m5::types::big_uint16_t(rbuf[0], rbuf[1]).get();
            tvoc  = m5::types::big_uint16_t(rbuf[3], rbuf[4]).get();
            return true;
        }
    }
    return false;
}

bool UnitSGP30::setAbsoluteHumidity(const uint16_t raw,
                                    const uint32_t duration) {
    m5::utility::CRC8_Checksum crc;
    std::array<uint8_t, 3> buf{};
    m5::types::big_uint16_t rr(raw);
    std::memcpy(buf.data(), rr.data(), 2);
    buf[2] = crc.range(rr.data(), 2);
    return writeRegister(SET_ABSOLUTE_HUMIDITY, buf.data(), buf.size()) &&
           delayMeasurementDuration(duration);
}

bool UnitSGP30::setAbsoluteHumidity(const float gm3, const uint32_t duration) {
    int32_t tmp = static_cast<int32_t>(std::round(gm3 * 256.f));
    if (tmp > 32767 || tmp < -32768) {
        M5_LIB_LOGE("Over/underflow: %f / %d", gm3, tmp);
        return false;
    }
    return setAbsoluteHumidity(static_cast<uint16_t>(static_cast<int16_t>(tmp)),
                               duration);
}

bool UnitSGP30::measureTest(uint16_t& result) {
    if (inPeriodic()) {
        M5_LIB_LOGD("Periodic measurements are running");
        return false;
    }

    std::array<uint8_t, 3> rbuf{};
    if (readRegister(MEASURE_TEST, rbuf.data(), rbuf.size(),
                     MEASURE_TEST_DURATION)) {
        m5::utility::CRC8_Checksum crc;
        if (crc.range(rbuf.data(), 2) == rbuf[2]) {
            result = m5::types::big_uint16_t(rbuf[0], rbuf[1]).get();
            return true;
        }
    }
    return false;
}

bool UnitSGP30::readTvocInceptiveBaseline(uint16_t& tvoc) {
    if (_version < 0x21) {
        M5_LIB_LOGE("Not enough the product version %x", _version);
        return false;
    }
    std::array<uint8_t, 3> rbuf{};
    if (readRegister(GET_TVOC_INCEPTIVE_BASELINE, rbuf.data(), rbuf.size(),
                     GET_TVOC_INCEPTIVE_BASELINE_DURATION)) {
        m5::utility::CRC8_Checksum crc;
        if (crc.range(rbuf.data(), 2) == rbuf[2]) {
            tvoc = m5::types::big_uint16_t(rbuf[0], rbuf[1]).get();
            return true;
        }
    }
    return false;
}

bool UnitSGP30::setTvocInceptiveBaseline(const uint16_t tvoc,
                                         const uint32_t duration) {
    if (_version < 0x21) {
        M5_LIB_LOGE("Not enough the product version %x", _version);
        return false;
    }
    m5::utility::CRC8_Checksum crc;
    std::array<uint8_t, 3> buf{};
    m5::types::big_uint16_t tt(tvoc);
    std::memcpy(buf.data(), tt.data(), 2);
    buf[2] = crc.range(tt.data(), 2);
    return writeRegister(SET_TVOC_INCEPTIVE_BASELINE, buf.data(), buf.size()) &&
           delayMeasurementDuration(duration);
}

bool UnitSGP30::generalReset() {
    uint8_t cmd{0x06};
    if (generalCall(&cmd, 1)) {
        _periodic = false;
        m5::utility::delay(10);
        return true;
    }
    return false;
}

bool UnitSGP30::readFeatureSet(sgp30::Feature& feature) {
    std::array<uint8_t, 3> rbuf{};
    if (readRegister(GET_FEATURE_SET, rbuf.data(), rbuf.size(),
                     GET_FEATURE_SET_DURATION)) {
        m5::utility::CRC8_Checksum crc;
        if (crc.range(rbuf.data(), 2) == rbuf[2]) {
            feature.value = m5::types::big_uint16_t(rbuf[0], rbuf[1]).get();
            return true;
        }
    }
    return false;
}

bool UnitSGP30::readSerialNumber(uint64_t& number) {
    std::array<uint8_t, 9> rbuf{};
    number = 0;
    if (readRegister(GET_SERIAL_ID, rbuf.data(), rbuf.size(),
                     GET_SERIAL_ID_DURATION)) {
        m5::utility::CRC8_Checksum crc;
        for (uint_fast8_t i = 0; i < 3; i++) {
            if (crc.range(rbuf.data() + i * 3, 2) != rbuf[i * 3 + 2]) {
                return false;
            }
        }
        for (uint_fast8_t i = 0; i < 3; ++i) {
            number |= ((uint64_t)(m5::types::big_uint16_t(rbuf[i * 3],
                                                          rbuf[i * 3 + 1])
                                      .get()))
                      << (16U * (2 - i));
        }
        return true;
    }
    return false;
}

bool UnitSGP30::readSerialNumber(char* number) {
    if (!number) {
        return false;
    }

    *number = '\0';
    uint64_t sno{};
    if (readSerialNumber(sno)) {
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
bool UnitSGP30::start_periodic_measurement(const uint32_t duration) {
    if (inPeriodic()) {
        return false;
    }

    if (writeRegister(IAQ_INIT)) {
        // For the first 15s after the “sgp30_iaq_init” command the sensor
        // is an initialization phase during which a “sgp30_measure_iaq”
        // command returns fixed values of 400 ppm CO2eq and 0 ppb TVOC.A
        // new “sgp30_iaq_init” command has to be sent after every power-up
        // or soft reset.

        // Baseline and absolute humidity restoration must take place during
        // this 15-second period
        _can_measure_time = m5::utility::millis() + 15 * 1000;
        _periodic         = true;
        _latest           = 0;
        _interval         = 1000;
        assert(_interval >= std::max(sgp30::MEASURE_RAW_DURATION,
                                     sgp30::MEASURE_IAQ_DURATION) &&
               "Illegal interval");

        m5::utility::delay(duration);
    }
    return _periodic;
}

bool UnitSGP30::set_iaq_baseline(const uint16_t co2eq, const uint16_t tvoc) {
    m5::utility::CRC8_Checksum crc{};
    m5::types::big_uint16_t cc(co2eq);
    m5::types::big_uint16_t tt(tvoc);

    std::array<uint8_t, (2 + 1) * 2> buf{};
    // Note that the order is different for get and set
    std::memcpy(buf.data() + 0, tt.data(), 2);
    buf[2] = crc.range(tt.data(), 2);
    std::memcpy(buf.data() + 3, cc.data(), 2);
    buf[5] = crc.range(cc.data(), 2);

    return writeRegister(SET_IAQ_BASELINE, buf.data(), buf.size());
}

bool UnitSGP30::read_measurement(Data& d) {
    if (readRegister(MEASURE_IAQ, d.raw.data(), d.raw.size(),
                     MEASURE_IAQ_DURATION)) {
        m5::utility::CRC8_Checksum crc{};
        return crc.range(d.raw.data(), 2) == d.raw[2] &&
               crc.range(d.raw.data() + 3, 2) == d.raw[5];
    }
    return false;

#if 0
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
#endif
}

}  // namespace unit
}  // namespace m5
