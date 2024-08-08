/*
 * SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
/*!
  @file test_helper.hpp
  @brief Helper for testing m5::unit::Component-derived classes
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

/*!
  @brief Tests for periodic measurement
  @tparam U Classes derived from m5::unit::Component
  @param unit Instance pointer of the U
  @param times Number of times to measure
  @param callback Function pointer called when the measurement is updated
  @param skip_after_test Skip check avg etc if true
  @return Average of the measurement interval
  @pre startPeriodicMeasurement must already have been called and _interval has
  been set and _latest was cleared
 */
template <class U>
uint32_t test_periodic_measurement(U* unit, const uint32_t times = 8,
                                   void (*callback)(U*)       = nullptr,
                                   const bool skip_after_test = false) {
    static_assert(std::is_base_of<m5::unit::Component, U>::value,
                  "U must be derived from Component");

    auto interval = unit->interval();
    decltype(interval) avg{}, avgCnt{};
    uint32_t cnt{times};
    auto prev       = unit->updatedMillis();
    auto timeout_at = m5::utility::millis() + (interval * 2) * times;
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
                // EXPECT_LE(duration, interval + 1);
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
            // There is room for consideration on the tolerance
            // EXPECT_LE(avg, decltype(interval)(interval * 1.1f));
            EXPECT_LE(avg, decltype(interval)(interval + 1));
        }
        return avg;
    }
    return 0U;
}

}  // namespace googletest
}  // namespace unit
}  // namespace m5
#endif
