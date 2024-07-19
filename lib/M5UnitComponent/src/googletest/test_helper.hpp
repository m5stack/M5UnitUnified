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

namespace m5 {
namespace unit {
namespace googletest {

/*!
  @brief Tests for periodic measurement
  @tparam U Classes derived from m5::unit::Component
  @param unit Instance pointer of the U
  @param interval Periodic measurement intervals
  @param times Number of times to measure
  @param callback Function pointer called when the measurement is updated
  @pre startPeriodicMeasurement must already have been called
 */
template <class U>
inline void test_periodic_measurement(U* unit, const uint32_t interval,
                                      const uint32_t times = 8,
                                      void (*callback)(U*) = nullptr) {
    static_assert(std::is_base_of<m5::unit::Component, U>::value,
                  "U must be derived from Component");

    uint32_t cnt{times};
    auto prev       = unit->updatedMillis();
    auto timeout_at = m5::utility::millis() + (interval + 1) * times;
    while (cnt && m5::utility::millis() <= timeout_at) {
        unit->update();
        if (unit->updated()) {
            --cnt;
            auto um = unit->updatedMillis();
            if (prev) {
                auto duration = um - prev;
                EXPECT_LE(duration, interval + 1); /* (*1) */
            }
            prev = um;
            if (callback) {
                callback(unit);
            }
        }
        m5::utility::delay(1); // *1
    }
    EXPECT_EQ(cnt, 0U);
}

}  // namespace googletest
}  // namespace unit
}  // namespace m5
#endif
