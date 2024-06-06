/*
  UnitTest for M5Utility

  SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD

  SPDX-License-Identifier: MIT
*/
#include <gtest/gtest.h>
#include <M5Utility.hpp>
#include <random>
#include <chrono>
#include <thread>

using namespace m5::utility::log;

namespace {
using pf_t = std::pair<const char* /* full path */, const char* /* filename */>;

pf_t table[] = {
    {"", ""},         {nullptr, ""},
    {"aaa", "aaa"},   {"ソソソソ", "ソソソソ"},
    {"a/b.c", "b.c"}, {"c:/aaa/bbb/ccc/ddd.eee", "ddd.eee"},
};

}  // namespace

TEST(Utility, pathToFilename) {
    {
        constexpr auto fn0 = pathToFilename("");
        EXPECT_STREQ(fn0, "");
        constexpr auto fn1 = pathToFilename("ABC");
        EXPECT_STREQ(fn1, "ABC");
        constexpr auto fn2 = pathToFilename("a:/bb/ccc/dddd/eee.f");
        EXPECT_STREQ(fn2, "eee.f");
    }

    for (auto&& e : table) {
        EXPECT_STREQ(pathToFilename(e.first), e.second) << e.first;
    }
}

#if 0
TEST(Utility, log) {
    M5_LIB_LOGE("Error");
    M5_LIB_LOGW("Warn");
    M5_LIB_LOGI("Info");
    M5_LIB_LOGD("Debug");
    M5_LIB_LOGV("Verbose");

    constexpr uint8_t test[] = {
        0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF, 0x01, 0x23,
        0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF, 0x01, 0x23, 0x45, 0x67,
        0x89, 0xAB, 0xCD, 0xEF, 0x01, 0x23, 0x45, 0x67, 0x89, 0xAB,
        0xCD, 0xEF, 0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF,
    };
    M5_DUMPE(test + 0, sizeof(test) - 0);
    M5_DUMPE(test + 1, sizeof(test) - 1);
    M5_DUMPE(test + 7, sizeof(test) - 07;
    M5_DUMPE(test + 11, sizeof(test) - 11);
    M5_DUMPE(test + 19, sizeof(test) - 19);
}
#endif

namespace {
auto rng = std::default_random_engine{};
}

TEST(Utility, BitSegment) {
    // static_assert
#if 0
    {
        m5::utility::BitSegment<11, int8_t> compile_error1; // 11 > 8
        m5::utility::BitSegment<0, int8_t> copile_error2;  // 0
    }
#endif

    // Constructor/Assignemt
    {
        using bs_t = m5::utility::BitSegment<6, uint8_t>;
        bs_t v0{0x84};  // base_type constructor
        bs_t v1 = v0;   // same type constructor

        EXPECT_EQ(v0.raw(), 0x84);
        EXPECT_EQ(v0.upper(), v1.upper());
        EXPECT_EQ(v0.lower(), v1.lower());
        EXPECT_EQ(v0.raw(), v1.raw());

        bs_t v2, v3;
        EXPECT_EQ(v2.raw(), 0);
        EXPECT_EQ(v2.raw(), v3.raw());

        v2 = -1;  // base_type assignment
        EXPECT_NE(v2.raw(), v3.raw());

        v3 = v2;  // same type assignment
        EXPECT_EQ(v2.raw(), v3.raw());

        bs_t v4 = 0xC0;  // base_type constructor
        EXPECT_EQ(v4.raw(), 0xC0);
    }

    // Getter/Setter using signed
    {
        using bs1_t = m5::utility::BitSegment<12, int16_t>;

        auto t = std::is_same<int16_t, bs1_t::base_type>::value;
        EXPECT_TRUE(t);
        t = std::is_same<uint16_t, bs1_t::unsigned_type>::value;
        EXPECT_TRUE(t);

        EXPECT_TRUE(bs1_t::SIGNED);
        EXPECT_EQ(+bs1_t::UPPER_BITS, 3);
        EXPECT_EQ(+bs1_t::LOWER_BITS, 12);
        EXPECT_EQ(+bs1_t::UPPER_SHIFT, 12);
        EXPECT_EQ(+bs1_t::UPPER_MASK, 0x07);
        EXPECT_EQ(+bs1_t::LOWER_MASK, 0xFFF);

        bs1_t value0;
        EXPECT_EQ(value0.upper(), 0U);
        EXPECT_EQ(value0.lower(), 0U);
        EXPECT_EQ(value0.raw(), 0);

        value0.lower(123);
        EXPECT_EQ(value0.upper(), 0U);
        EXPECT_EQ(value0.lower(), 123U);
        EXPECT_EQ(value0.raw(), 123);

        value0.lower(0x7FFF);
        EXPECT_EQ(value0.upper(), 0U);
        EXPECT_EQ(value0.lower(), 0x0FFFU);
        EXPECT_EQ(value0.raw(), 0x0FFF);

        value0.upper(5);
        EXPECT_EQ(value0.upper(), 5U);
        EXPECT_EQ(value0.lower(), 0xFFFU);
        EXPECT_EQ(value0.raw(), 0x5FFF);

        value0.upper(0xFF);
        EXPECT_EQ(value0.upper(), 0x0007U);
        EXPECT_EQ(value0.lower(), 0x0FFFU);
        EXPECT_EQ(value0.raw(), 0x7FFF);

        value0.raw(-1);
        EXPECT_EQ(value0.upper(), 0x0007U);
        EXPECT_EQ(value0.lower(), 0x0FFFU);
        EXPECT_EQ(value0.raw(), -1);

        // Keep signed bit
        value0.lower(234);
        EXPECT_EQ(value0.upper(), 0x0007U);
        EXPECT_EQ(value0.lower(), 234U);
        EXPECT_EQ(value0.raw(), -3862);  // 0xF0EA
    }

    // Getter/Setter using unsigned
    {
        using bs2_t = m5::utility::BitSegment<12, const uint32_t&>;
        auto t      = std::is_same<uint32_t, bs2_t::base_type>::value;
        EXPECT_TRUE(t);
        t = std::is_same<uint32_t, bs2_t::unsigned_type>::value;
        EXPECT_TRUE(t);

        EXPECT_FALSE(bs2_t::SIGNED);
        EXPECT_EQ(+bs2_t::UPPER_BITS, 20U);
        EXPECT_EQ(+bs2_t::LOWER_BITS, 12U);
        EXPECT_EQ(+bs2_t::UPPER_SHIFT, 12U);
        EXPECT_EQ(+bs2_t::UPPER_MASK, 0xFFFFFU);
        EXPECT_EQ(+bs2_t::LOWER_MASK, 0xFFFU);

        bs2_t value0;
        EXPECT_EQ(value0.upper(), 0U);
        EXPECT_EQ(value0.lower(), 0U);
        EXPECT_EQ(value0.raw(), 0U);

        value0.lower(123);
        EXPECT_EQ(value0.upper(), 0U);
        EXPECT_EQ(value0.lower(), 123U);
        EXPECT_EQ(value0.raw(), 123U);

        value0.lower(0x00003FFF);
        EXPECT_EQ(value0.upper(), 0U);
        EXPECT_EQ(value0.lower(), 0x00000FFFU);
        EXPECT_EQ(value0.raw(), 0x00000FFFU);

        value0.upper(5);
        EXPECT_EQ(value0.upper(), 5U);
        EXPECT_EQ(value0.lower(), 0x00000FFFU);
        EXPECT_EQ(value0.raw(), 0x00005FFFU);

        value0.upper(0x84218421);
        EXPECT_EQ(value0.upper(), 0x00018421U);
        EXPECT_EQ(value0.lower(), 0x00000FFFU);
        EXPECT_EQ(value0.raw(), 0x18421FFFU);

        value0.raw(-1);
        EXPECT_EQ(value0.upper(), 0x000FFFFFU);
        EXPECT_EQ(value0.lower(), 0x00000FFFU);
        EXPECT_EQ(value0.raw(), 0xFFFFFFFFU);

        // Not include signed bit
        value0.lower(234);
        EXPECT_EQ(value0.upper(), 0x000FFFFFU);
        EXPECT_EQ(value0.lower(), 234U);
        EXPECT_EQ(value0.raw(), 4294963434U);  // 0xFFFFF0EA
    }

    // Compare
    {
        using bs_t = m5::utility::BitSegment<14, int64_t>;

        bs_t v0, v1;
        EXPECT_TRUE(v0 == v1);
        EXPECT_FALSE(v0 != v1);
        EXPECT_FALSE(v0 < v1);
        EXPECT_FALSE(v0 > v1);
        EXPECT_TRUE(v0 <= v1);
        EXPECT_TRUE(v0 >= v1);

        EXPECT_TRUE(v0 == 0);
        EXPECT_FALSE(v0 != 0);
        EXPECT_FALSE(v0 < 0);
        EXPECT_FALSE(v0 > 0);
        EXPECT_TRUE(v0 <= 0);
        EXPECT_TRUE(v0 >= 0);

        EXPECT_TRUE(0 == v0);
        EXPECT_FALSE(0 != v0);
        EXPECT_FALSE(0 < v0);
        EXPECT_FALSE(0 > v0);
        EXPECT_TRUE(0 <= v0);

        v1.lower(123);
        EXPECT_FALSE(v0 == v1);
        EXPECT_TRUE(v0 != v1);
        EXPECT_TRUE(v0 < v1);
        EXPECT_FALSE(v0 > v1);
        EXPECT_TRUE(v0 <= v1);
        EXPECT_FALSE(v0 >= v1);

        v0.lower(987);
        EXPECT_FALSE(v0 == v1);
        EXPECT_TRUE(v0 != v1);
        EXPECT_FALSE(v0 < v1);
        EXPECT_TRUE(v0 > v1);
        EXPECT_FALSE(v0 <= v1);
        EXPECT_TRUE(v0 >= v1);

        v0 = v1;
        v1.upper(654321);
        EXPECT_FALSE(v0 == v1);
        EXPECT_TRUE(v0 != v1);
        EXPECT_TRUE(v0 < v1);
        EXPECT_FALSE(v0 > v1);
        EXPECT_TRUE(v0 <= v1);
        EXPECT_FALSE(v0 >= v1);

        v0.upper(99999999);
        EXPECT_FALSE(v0 == v1);
        EXPECT_TRUE(v0 != v1);
        EXPECT_FALSE(v0 < v1);
        EXPECT_TRUE(v0 > v1);
        EXPECT_FALSE(v0 <= v1);
        EXPECT_TRUE(v0 >= v1);

        v0 = v1 = 0;

        v0 = -123;
        v1 = -12;
        EXPECT_FALSE(v0 == v1);
        EXPECT_TRUE(v0 != v1);
        EXPECT_TRUE(v0 < v1);
        EXPECT_FALSE(v0 > v1);
        EXPECT_TRUE(v0 <= v1);
        EXPECT_FALSE(v0 >= v1);

        v1 = -876543;
        EXPECT_FALSE(v0 == v1);
        EXPECT_TRUE(v0 != v1);
        EXPECT_FALSE(v0 < v1);
        EXPECT_TRUE(v0 > v1);
        EXPECT_FALSE(v0 <= v1);
        EXPECT_TRUE(v0 >= v1);
    }
}

namespace {
using clock = std::chrono::high_resolution_clock;
}

TEST(Utility, comatibility) {
    // millis
    {
        auto ms  = m5::utility::millis();
        auto ams = clock::now() + std::chrono::milliseconds(1);
        std::this_thread::sleep_until(ams);
        EXPECT_GT(m5::utility::millis(), ms);
    }
    // micros
    {
        auto us  = m5::utility::micros();
        auto aus = clock::now() + std::chrono::microseconds(1);
        std::this_thread::sleep_until(aus);
        EXPECT_GT(m5::utility::micros(), us);
    }

    // delay
    {
        constexpr unsigned long wait{20};
        auto start = clock::now();
        m5::utility::delay(wait);
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                           clock::now() - start)
                           .count();
        auto elapsed2 = std::chrono::duration_cast<std::chrono::microseconds>(
                            clock::now() - start)
                            .count();
        // printf("-----> %lld %lld\n", elapsed, elapsed2);
#if defined(ARDUINO)
        EXPECT_GE(
            elapsed,
            wait - 1);  //  Arduino delay may return before the specified time.
#else
        EXPECT_GE(elapsed, wait);
#endif
    }

    // delayMicroseconds
    {
        auto start = clock::now();
        m5::utility::delayMicroseconds(1);
        auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(
                           clock::now() - start)
                           .count();
        EXPECT_GE(elapsed, 1);
    }
}
