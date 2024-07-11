/*
 * SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
/*
  UnitTest for UnitBME688
*/

// Move to each libarry

#include <gtest/gtest.h>
#include <Wire.h>
#include <M5Unified.h>
#include <M5UnitUnified.hpp>
#include <googletest/test_template.hpp>
#include <unit/unit_BME688.hpp>
#include <chrono>
#include <random>
#include <set>

using namespace m5::unit::googletest;
using namespace m5::unit;
using namespace m5::unit::bme688;

const ::testing::Environment* global_fixture =
    ::testing::AddGlobalTestEnvironment(new GlobalFixture<400000U>());

class TestBME688 : public ComponentTestBase<UnitBME688, bool> {
   protected:
    virtual UnitBME688* get_instance() override {
        return new m5::unit::UnitBME688();
    }
    virtual bool is_using_hal() const override {
        return GetParam();
    };
};

// INSTANTIATE_TEST_SUITE_P(ParamValues, TestBME688,
//                         ::testing::Values(false, true));
// INSTANTIATE_TEST_SUITE_P(ParamValues, TestBME688, ::testing::Values(true));
INSTANTIATE_TEST_SUITE_P(ParamValues, TestBME688, ::testing::Values(false));

namespace {
constexpr Oversampling os_table[] = {
    Oversampling::None, Oversampling::x1, Oversampling::x1,  Oversampling::x2,
    Oversampling::x4,   Oversampling::x8, Oversampling::x16,
};
constexpr Filter filter_table[] = {
    Filter::Coeff_0,  Filter::Coeff_1,  Filter::Coeff_3,  Filter::Coeff_7,
    Filter::Coeff_15, Filter::Coeff_31, Filter::Coeff_63, Filter::Coeff_127,
};

#if defined(UNIT_BME688_USING_BSEC2)
// All outputs
constexpr bsec_virtual_sensor_t vs_table[] = {
    BSEC_OUTPUT_IAQ,
    BSEC_OUTPUT_STATIC_IAQ,
    BSEC_OUTPUT_CO2_EQUIVALENT,
    BSEC_OUTPUT_BREATH_VOC_EQUIVALENT,
    BSEC_OUTPUT_RAW_TEMPERATURE,
    BSEC_OUTPUT_RAW_PRESSURE,
    BSEC_OUTPUT_RAW_HUMIDITY,
    BSEC_OUTPUT_RAW_GAS,
    BSEC_OUTPUT_STABILIZATION_STATUS,
    BSEC_OUTPUT_RUN_IN_STATUS,
    BSEC_OUTPUT_SENSOR_HEAT_COMPENSATED_TEMPERATURE,
    BSEC_OUTPUT_SENSOR_HEAT_COMPENSATED_HUMIDITY,
    BSEC_OUTPUT_GAS_PERCENTAGE,
    BSEC_OUTPUT_GAS_ESTIMATE_1,
    BSEC_OUTPUT_GAS_ESTIMATE_2,
    BSEC_OUTPUT_GAS_ESTIMATE_3,
    BSEC_OUTPUT_GAS_ESTIMATE_4,
    BSEC_OUTPUT_RAW_GAS_INDEX,
    BSEC_OUTPUT_REGRESSION_ESTIMATE_1,
    BSEC_OUTPUT_REGRESSION_ESTIMATE_2,
    BSEC_OUTPUT_REGRESSION_ESTIMATE_3,
    BSEC_OUTPUT_REGRESSION_ESTIMATE_4,
};
#endif

std::random_device rng;

#if defined(UNIT_BME688_USING_BSEC2)
// Using BSEC2 library configuration files
constexpr uint8_t bsec_config[] = {
#include <config/bme688/bme688_sel_33v_3s_4d/bsec_selectivity.txt>
};
constexpr uint8_t bsec_config2[] = {
#include <config/bme688/bme688_sel_33v_300s_4d/bsec_selectivity.txt>
};
#endif

}  // namespace

TEST_P(TestBME688, Settings) {
    SCOPED_TRACE(ustr);

    Oversampling os{};
    Filter f{};

    uint32_t serial{};
    EXPECT_TRUE(unit->readUniqueID(serial));
    EXPECT_NE(serial, 0U);

    const TPHSetting prev = unit->tphSetting();

    for (auto&& e : os_table) {
        EXPECT_TRUE(unit->setOversamplingTemperature(e));
        EXPECT_EQ(unit->tphSetting().os_temp, m5::stl::to_underlying(e));
        EXPECT_TRUE(unit->readOversamplingTemperature(os));
        EXPECT_EQ(os, e);
    }
    for (auto&& e : os_table) {
        EXPECT_TRUE(unit->setOversamplingPressure(e));
        EXPECT_EQ(unit->tphSetting().os_pres, m5::stl::to_underlying(e));
        EXPECT_TRUE(unit->readOversamplingPressure(os));
        EXPECT_EQ(os, e);
    }
    for (auto&& e : os_table) {
        EXPECT_TRUE(unit->setOversamplingHumidity(e));
        EXPECT_EQ(unit->tphSetting().os_hum, m5::stl::to_underlying(e));
        EXPECT_TRUE(unit->readOversamplingHumidity(os));
        EXPECT_EQ(os, e);
    }
    for (auto&& e : filter_table) {
        EXPECT_TRUE(unit->setIIRFilter(e));
        EXPECT_EQ(unit->tphSetting().filter, m5::stl::to_underlying(e));
        EXPECT_TRUE(unit->readIIRFilter(f));
        EXPECT_EQ(f, e);
    }

    uint32_t cnt{10};
    while (cnt--) {
        TPHSetting tph = unit->tphSetting();
        tph.os_temp    = rng() % 0x06;
        tph.os_pres    = rng() % 0x06;
        tph.os_hum     = rng() % 0x06;
        tph.filter     = rng() % 0x07;

        // M5_LOGW("%u/%u/%u/%u", tph.os_temp, tph.os_pres, tph.os_hum,
        //         tph.filter);

        EXPECT_TRUE(unit->setTPHSetting(tph));
        EXPECT_EQ(unit->tphSetting().os_temp, tph.os_temp);
        EXPECT_EQ(unit->tphSetting().os_pres, tph.os_pres);
        EXPECT_EQ(unit->tphSetting().os_hum, tph.os_hum);

        TPHSetting after{};
        EXPECT_TRUE(unit->readTPHSetting(after));
        EXPECT_TRUE(memcmp(&tph, &after, sizeof(tph)) == 0)
            << tph.os_temp << "/" << tph.os_pres << "/" << tph.os_hum << "/"
            << tph.filter;

        EXPECT_TRUE(unit->setOversampling((Oversampling)tph.os_temp,
                                          (Oversampling)tph.os_pres,
                                          (Oversampling)tph.os_hum));
        EXPECT_EQ(unit->tphSetting().os_temp, tph.os_temp);
        EXPECT_EQ(unit->tphSetting().os_pres, tph.os_pres);
        EXPECT_EQ(unit->tphSetting().os_hum, tph.os_hum);

        EXPECT_TRUE(unit->readTPHSetting(after));
        EXPECT_TRUE(memcmp(&tph, &after, sizeof(tph)) == 0)
            << tph.os_temp << "/" << tph.os_pres << "/" << tph.os_hum << "/"
            << tph.filter;
    }

    // softReset rewinds settings
    EXPECT_TRUE(unit->softReset());

    EXPECT_TRUE(unit->readOversamplingTemperature(os));
    EXPECT_EQ(m5::stl::to_underlying(os), prev.os_temp);
    EXPECT_TRUE(unit->readOversamplingPressure(os));
    EXPECT_EQ(m5::stl::to_underlying(os), prev.os_pres);
    EXPECT_TRUE(unit->readOversamplingHumidity(os));
    EXPECT_EQ(m5::stl::to_underlying(os), prev.os_hum);
    EXPECT_TRUE(unit->readIIRFilter(f));
    EXPECT_EQ(m5::stl::to_underlying(f), prev.filter);
}

TEST_P(TestBME688, SelfTest) {
    SCOPED_TRACE(ustr);
    EXPECT_TRUE(unit->selfTest());
}

#if defined(UNIT_BME688_USING_BSEC2)
TEST_P(TestBME688, BSEC2) {
    SCOPED_TRACE(ustr);

    uint8_t cfg[BSEC_MAX_PROPERTY_BLOB_SIZE]{};
    uint8_t state[BSEC_MAX_STATE_BLOB_SIZE]{};
    uint8_t state2[BSEC_MAX_STATE_BLOB_SIZE]{};
    uint32_t actual{};

    // Version
    auto& ver = unit->bsec2Version();
    EXPECT_NE(ver.major, 0);
    EXPECT_NE(ver.minor, 0);
    M5_LOGI("bsec2 ver:%u.%u.%u.%u", ver.major, ver.minor, ver.major_bugfix,
            ver.minor_bugfix);

    // Config
    EXPECT_EQ(sizeof(bsec_config), BSEC_MAX_PROPERTY_BLOB_SIZE);
    EXPECT_TRUE(unit->bsec2SetConfig(bsec_config));
    EXPECT_TRUE(unit->bsec2GetConfig(cfg, actual));
    auto cmp = memcmp(cfg, bsec_config, actual) == 0;
    EXPECT_TRUE(cmp);
    if (!cmp) {
        M5_DUMPI(cfg, actual);
        M5_DUMPI(bsec_config, actual);
    }

    // State
    EXPECT_TRUE(unit->bsec2GetState(state, actual));
    EXPECT_TRUE(unit->bsec2SetState(state));
    EXPECT_TRUE(unit->bsec2GetState(state2, actual));
    cmp = memcmp(state2, state, actual) == 0;
    if (!cmp) {
        M5_DUMPI(state, actual);
        M5_DUMPI(state2, actual);
    }

    // Subscribe
    constexpr bsec_virtual_sensor_t sensorList[] = {
        BSEC_OUTPUT_IAQ,          BSEC_OUTPUT_RAW_TEMPERATURE,
        BSEC_OUTPUT_RAW_PRESSURE, BSEC_OUTPUT_RAW_HUMIDITY,
        BSEC_OUTPUT_RAW_GAS,      BSEC_OUTPUT_STABILIZATION_STATUS,
        BSEC_OUTPUT_RUN_IN_STATUS};
    std::vector<bsec_virtual_sensor_t> nosubscribed(
        vs_table, vs_table + m5::stl::size(vs_table));
    auto it = std::remove_if(
        nosubscribed.begin(), nosubscribed.end(),
        [&sensorList](bsec_virtual_sensor_t vs) {
            auto e = std::begin(sensorList) + m5::stl::size(sensorList);
            return std::find(std::begin(sensorList), e, vs) != e;
        });
    nosubscribed.erase(it, nosubscribed.end());

    EXPECT_TRUE(unit->bsec2UpdateSubscription(
        sensorList, m5::stl::size(sensorList), bsec2::SampleRate::LowPower));
    for (auto&& e : sensorList) {
        EXPECT_TRUE(unit->bsec2IsSubscribed(e)) << e;
    }
    for (auto&& e : nosubscribed) {
        EXPECT_FALSE(unit->bsec2IsSubscribed(e)) << e;
    }

    for (auto&& e : sensorList) {
        EXPECT_TRUE(unit->bsec2Unsubscribe(e)) << e;
        EXPECT_FALSE(unit->bsec2IsSubscribed(e)) << e;
    }
    for (auto&& e : nosubscribed) {
        EXPECT_FALSE(unit->bsec2IsSubscribed(e)) << e;
    }

    for (auto&& e : sensorList) {
        EXPECT_TRUE(unit->bsec2Subscribe(e)) << e;
        EXPECT_TRUE(unit->bsec2IsSubscribed(e)) << e;
    }
    for (auto&& e : nosubscribed) {
        EXPECT_FALSE(unit->bsec2IsSubscribed(e)) << e;
    }

    EXPECT_TRUE(unit->bsec2UnsubscribeAll());
    for (auto&& e : vs_table) {
        EXPECT_FALSE(unit->bsec2IsSubscribed(e)) << e;
    }

    // Measurement
    EXPECT_TRUE(unit->bsec2UpdateSubscription(
        sensorList, m5::stl::size(sensorList), bsec2::SampleRate::LowPower));

    uint32_t cnt{3};
    while (cnt--) {
        auto now{m5::utility::millis()};
        auto timeout_at = now + 3 * 1000;
        do {
            unit->update();
            now = m5::utility::millis();
            if (unit->updated()) {
                break;
            }
            m5::utility::delay(1);
        } while (now <= timeout_at);
        EXPECT_TRUE(unit->updated());
        if (cnt < 2) {
            EXPECT_LE(now, timeout_at);
        }

        for (auto&& vs : sensorList) {
            auto f = unit->latestData(vs);
            // M5_LOGI("[%u]:%.2f", vs, f);
            EXPECT_TRUE(std::isfinite(f)) << vs;
        }
        for (auto&& vs : nosubscribed) {
            EXPECT_FALSE(std::isfinite(unit->latestData(vs))) << vs;
        }
    }
}
#endif

#if 0
TEST_P(TestBME688, BSEC2_2) {
    SCOPED_TRACE(ustr);

    EXPECT_EQ(sizeof(bsec_config2), BSEC_MAX_PROPERTY_BLOB_SIZE);
    EXPECT_TRUE(unit->bsec2SetConfig(bsec_config2));

    constexpr bsec_virtual_sensor_t sensorList[] = {
        BSEC_OUTPUT_RAW_TEMPERATURE,       BSEC_OUTPUT_RAW_PRESSURE,
        BSEC_OUTPUT_RAW_HUMIDITY,          BSEC_OUTPUT_RAW_GAS,
        BSEC_OUTPUT_RAW_GAS_INDEX,         BSEC_OUTPUT_REGRESSION_ESTIMATE_1,
        BSEC_OUTPUT_REGRESSION_ESTIMATE_2, BSEC_OUTPUT_REGRESSION_ESTIMATE_3,
        BSEC_OUTPUT_REGRESSION_ESTIMATE_4};

    EXPECT_TRUE(unit->bsec2UpdateSubscription(
        sensorList, m5::stl::size(sensorList), bsec2::SampleRate::Scan));

    auto now{m5::utility::millis()};
    auto timeout_at = now + 3 * 1000;
    do {
        unit->update();
        now = m5::utility::millis();
        if (unit->updated()) {
            break;
        }
        m5::utility::delay(1);
    } while (now <= timeout_at);
    EXPECT_TRUE(unit->updated());

}
#endif

#if 0
TEST_P(TestBME688, Forced) {
    SCOPED_TRACE(ustr);
    TPHSetting tph{};
    tph.os_temp = m5::stl::to_underlying(Oversampling::x2);
    tph.os_pres = m5::stl::to_underlying(Oversampling::x1);
    tph.os_temp = m5::stl::to_underlying(Oversampling::x16);
    tph.filter  = m5::stl::to_underlying(Filter::Coeff_0);
    EXPECT_TRUE(unit->setTPHSetting(tph));

    HeaterSetting hs{};
    hs.enable     = true;
    hs.heatr_temp = 300;
    hs.heatr_dur  = 100;
    EXPECT_TRUE(unit->setHeaterSetting(Mode::Forced, hs));

    MeasurementData data{};
    EXPECT_TRUE(unit->measureSingleShot(data));

    M5_LOGI(
        "Status:%u\n"
        "Gidx:%u Midx:%u\n"
        "Resistance:%u IDAC:%u GasWait:%u\n"
        "T:%f P:%f H:%f R:%f",
        data.status, data.gas_index, data.meas_index, data.res_heat, data.idac,
        data.gas_wait, data.temperature, data.pressure, data.humidity,
        data.gas_resistance);
}

#endif
