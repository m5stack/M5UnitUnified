/*
 * SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
/*!
  @file test_helper.hpp
  @brief Helper for testing UnitComponent periodic measurements
  @note Depends on GoogleTest
*/
#ifndef M5_UNIT_COMPONENT_GOOGLETEST_HELPER_HPP
#define M5_UNIT_COMPONENT_GOOGLETEST_HELPER_HPP

#include <M5Utility.hpp>
#include <algorithm>
#include <cstdint>
#include <numeric>
#include <vector>

namespace m5 {
namespace unit {
namespace googletest {

/*!
  @struct PeriodicMeasurementResult
  @brief Result of periodic measurement data collection with statistical analysis
 */
struct PeriodicMeasurementResult {
    std::vector<uint32_t> intervals;  //!< All inter-update intervals (ms)
    uint32_t update_count{};          //!< Number of successful updates
    uint32_t expected_interval{};     //!< Expected interval from unit->interval()
    bool timed_out{};                 //!< True if timeout before collecting all samples

    //! @brief Average of intervals (ms)
    uint32_t average() const
    {
        if (intervals.empty()) {
            return 0;
        }
        uint64_t sum = std::accumulate(intervals.begin(), intervals.end(), uint64_t{0});
        return static_cast<uint32_t>(sum / intervals.size());
    }

    //! @brief Median of intervals (ms), robust to outliers
    uint32_t median() const
    {
        if (intervals.empty()) {
            return 0;
        }
        auto sorted = intervals;
        std::sort(sorted.begin(), sorted.end());
        auto n = sorted.size();
        return (n % 2) ? sorted[n / 2] : (sorted[n / 2 - 1] + sorted[n / 2]) / 2;
    }

    //! @brief Minimum interval (ms)
    uint32_t min_interval() const
    {
        if (intervals.empty()) {
            return 0;
        }
        return *std::min_element(intervals.begin(), intervals.end());
    }

    //! @brief Maximum interval (ms)
    uint32_t max_interval() const
    {
        if (intervals.empty()) {
            return 0;
        }
        return *std::max_element(intervals.begin(), intervals.end());
    }
};

/*!
  @brief Collect periodic measurement data from a unit
  @tparam U m5::unit::Component-derived class
  @param unit Unit instance to collect measurements from
  @param times Number of update cycles to collect
  @param timeout_duration Timeout in ms (0 = auto-calculate as interval * (times + 1))
  @param callback Optional callback invoked after each successful update
  @return PeriodicMeasurementResult containing collected intervals and statistics
 */
template <class U>
PeriodicMeasurementResult collect_periodic_measurements(U* unit, const uint32_t times = 8,
                                                        const uint32_t timeout_duration = 0,
                                                        void (*callback)(U*)            = nullptr)
{
    static_assert(std::is_base_of<m5::unit::Component, U>::value, "U must be derived from Component");

    PeriodicMeasurementResult result;
    result.expected_interval = unit->interval();
    if (times > 1) {
        result.intervals.reserve(times - 1);
    }

    auto actual_timeout = timeout_duration ? timeout_duration : result.expected_interval * (times + 1);
    auto timeout_at     = m5::utility::millis() + actual_timeout;
    uint32_t remaining  = times;
    decltype(unit->updatedMillis()) prev{};
    bool first_update = true;

    while (remaining && m5::utility::millis() <= timeout_at) {
        unit->update();
        if (unit->updated()) {
            ++result.update_count;
            --remaining;
            auto um = unit->updatedMillis();
            if (!first_update) {
                result.intervals.push_back(um - prev);
            }
            first_update = false;
            prev         = um;
            if (callback) {
                callback(unit);
            }
        }
    }

    result.timed_out = (remaining > 0);
    return result;
}

}  // namespace googletest
}  // namespace unit
}  // namespace m5
#endif
