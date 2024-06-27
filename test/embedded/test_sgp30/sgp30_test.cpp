/*
 * SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
/*
  UnitTest for UnitSGP30
*/

// Move to each libarry

#include <gtest/gtest.h>
#include <Wire.h>
#include <M5Unified.h>
#include <M5UnitUnified.hpp>
#include <googletest/test_template.hpp>
#include <unit/unit_SGP30.hpp>

using namespace m5::unit::googletest;
using namespace m5::unit;
using namespace m5::unit::sgp30;

const ::testing::Environment* global_fixture =
    ::testing::AddGlobalTestEnvironment(new GlobalFixture<400000U>());

class TestSGP30 : public ComponentTestBase<UnitSGP30, bool> {
   protected:
    virtual UnitSGP30* get_instance() override {
        auto* ptr = new m5::unit::UnitSGP30();
        if (ptr) {
            // *1
            auto cfg           = ptr->config();
            cfg.start_periodic = false;
            ptr->config(cfg);
        }
        return ptr;
    }
    virtual bool is_using_hal() const override {
        return GetParam();
    };

    bool wait_start_measurement() {
        auto timeout_at = m5::utility::millis() + (15 * 1000);
        bool done{};
        do {
            unit->update();
            done = unit->updated();
            if (done) {
                break;
            }
            m5::utility::delay(1);
        } while (!done && m5::utility::millis() <= timeout_at);
        return done;
    }
};

// INSTANTIATE_TEST_SUITE_P(ParamValues, TestSGP30,
//                          ::testing::Values(false, true));
//  INSTANTIATE_TEST_SUITE_P(ParamValues, TestSGP30, ::testing::Values(true));
INSTANTIATE_TEST_SUITE_P(ParamValues, TestSGP30, ::testing::Values(false));

TEST_P(TestSGP30, selfTest) {
    SCOPED_TRACE(ustr);

    uint16_t result{};
    EXPECT_TRUE(unit->measureTest(result));
    EXPECT_EQ(result, 0xD400);
}

TEST_P(TestSGP30, serialNumber) {
    SCOPED_TRACE(ustr);
    // Read direct [MSB] SNB_3, SNB_2, CRC, SNB_1, SNB_0, CRC [LSB]
    std::array<uint8_t, 9> rbuf{};
    EXPECT_TRUE(unit->readRegister(command::GET_SERIAL_ID, rbuf.data(),
                                   rbuf.size(), 1));

    // M5_LOGI("%02x%02x%02x%02x%02x%02x", rbuf[0], rbuf[1], rbuf[3],
    // rbuf[4],
    //         rbuf[6], rbuf[7]);

    m5::types::big_uint16_t w0(rbuf[0], rbuf[1]);
    m5::types::big_uint16_t w1(rbuf[3], rbuf[4]);
    m5::types::big_uint16_t w2(rbuf[6], rbuf[7]);
    uint64_t d_sno = (((uint64_t)w0.get()) << 32) |
                     (((uint64_t)w1.get()) << 16) | ((uint64_t)w2.get());

    // M5_LOGI("d_sno[%llX]", d_sno);

    //
    uint64_t sno{};
    char ssno[13]{};
    EXPECT_TRUE(unit->getSerialNumber(sno));
    EXPECT_TRUE(unit->getSerialNumber(ssno));

    // M5_LOGI("s:[%s] uint64:[%x]", ssno, sno);

    EXPECT_EQ(sno, d_sno);

    std::stringstream stream;
    stream << std::uppercase << std::setw(12) << std::hex << std::setfill('0')
           << sno;
    std::string s(stream.str());

    EXPECT_STREQ(s.c_str(), ssno);
}

TEST_P(TestSGP30, generalReset) {
    SCOPED_TRACE(ustr);

    EXPECT_TRUE(unit->startPeriodicMeasurement(0x1234, 0x5678, 0x9ABC));
    EXPECT_TRUE(wait_start_measurement());

    uint16_t co2eq{}, tvoc{};
    EXPECT_TRUE(unit->getIaqBaseline(co2eq, tvoc));
    EXPECT_EQ(co2eq, 0x1234);
    EXPECT_EQ(tvoc, 0x5678);

    EXPECT_TRUE(unit->generalReset());

    EXPECT_TRUE(unit->getIaqBaseline(co2eq, tvoc));
    EXPECT_EQ(co2eq, 0x0000);
    EXPECT_EQ(tvoc, 0x0000);
}

TEST_P(TestSGP30, Measurement) {
    SCOPED_TRACE(ustr);

    EXPECT_TRUE(unit->startPeriodicMeasurement(0x1234, 0x5678, 0x9ABC));
    EXPECT_TRUE(wait_start_measurement());

    auto now                        = m5::utility::millis();
    constexpr unsigned long timeout = 1000;  // 1sec
    auto timeout_at                 = now + timeout;
    uint8_t count{0};
    // Between first and second mesured
    do {
        unit->update();
        bool upd = unit->updated();
        count += upd ? 1 : 0;
        now = m5::utility::millis();
        if (upd && count == 1) {  // First?
            timeout_at = now + timeout;
        }
        m5::utility::delay(1);
    } while (count < 2 && now <= timeout_at);

    EXPECT_EQ(count, 2);
    EXPECT_LE(now, timeout_at) << now << " : " << timeout_at;

    auto update_at = unit->updatedMillis();
    EXPECT_LE(update_at, timeout_at);

    auto co2eq = unit->co2eq();
    auto tvoc  = unit->tvoc();
    M5_LOGI("%u %u", co2eq, tvoc);

    uint16_t h2{}, etoh{};
    EXPECT_TRUE(unit->readRaw(h2, etoh));

    M5_LOGI("%u/%u", h2, etoh);
}
