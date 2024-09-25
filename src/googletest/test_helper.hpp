/*
 * SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
/*!
  @file test_helper.hpp
  @brief Helper for testing UnitComponent
  @note Depends on GoogleTest
*/
#ifndef M5_UNIT_COMPONENT_GOOGLETEST_HELPER_HPP
#define M5_UNIT_COMPONENT_GOOGLETEST_HELPER_HPP

#include <M5Utility.hpp>
#include <thread>
#include <cassert>

namespace m5 {
namespace unit {
namespace googletest {

template <class U>
uint32_t test_periodic_measurement(U* unit, const uint32_t times, const uint32_t tolerance,
                                   const uint32_t timeout_duration, void (*callback)(U*), const bool skip_after_test)
{
    static_assert(std::is_base_of<m5::unit::Component, U>::value, "U must be derived from Component");

    auto interval = unit->interval();
    decltype(interval) avg{}, avgCnt{};
    uint32_t cnt{times};
    auto prev       = unit->updatedMillis();
    auto timeout_at = m5::utility::millis() + timeout_duration;
    while (cnt && m5::utility::millis() <= timeout_at) {
        unit->update();
        if (unit->updated()) {
            --cnt;
            auto um = unit->updatedMillis();
            if (prev) {
                auto duration = um - prev;
                ++avgCnt;
                avg += duration;
                // M5_LOGI("dur:%ld", duration);
                //  EXPECT_LE(duration, interval + 1);
            }
            prev = um;
            if (callback) {
                callback(unit);
            }
        }
        std::this_thread::yield();
    }

    if (!skip_after_test) {
        EXPECT_EQ(cnt, 0U);
        EXPECT_EQ(avgCnt, times - 1);
        if (avgCnt) {
            avg /= avgCnt;
            EXPECT_LE(avg, decltype(interval)(interval + tolerance));
        }
        return avg;
    }
    return 0U;
}

template <class U>
uint32_t test_periodic_measurement(U* unit, const uint32_t times = 8, const uint32_t tolerance = 1,
                                   void (*callback)(U*) = nullptr, const bool skip_after_test = false)
{
    static_assert(std::is_base_of<m5::unit::Component, U>::value, "U must be derived from Component");
    auto timeout_duration = (unit->interval() * 2) * times;
    return test_periodic_measurement(unit, times, tolerance, timeout_duration, callback, skip_after_test);
}

template <class U>
uint32_t test_periodic_measurement(U* unit, const uint32_t times = 8, void (*callback)(U*) = nullptr,
                                   const bool skip_after_test = false)
{
    static_assert(std::is_base_of<m5::unit::Component, U>::value, "U must be derived from Component");
    auto timeout_duration = (unit->interval() * 2) * times;
    return test_periodic_measurement(unit, times, 1, timeout_duration, callback, skip_after_test);
}

}  // namespace googletest
}  // namespace unit
}  // namespace m5
#endif
