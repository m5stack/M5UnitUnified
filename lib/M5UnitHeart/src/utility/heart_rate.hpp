/*
 * SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
/*!
  @file heart_rate.hpp
  @brief Calculate heart rate and SpO2
*/
#ifndef M5_UNIT_HEART_UTILITY_HEART_RATE_HPP
#define M5_UNIT_HEART_UTILITY_HEART_RATE_HPP

#include <deque>
#include <cstddef>
#include <m5_unit_component/types.hpp>

namespace m5 {
namespace max30100 {

struct dcFilter_t {
    float w;
    float result;
};

struct meanDiffFilter_t {
    static constexpr uint32_t MEAN_FILTER_SIZE{15};
    float values[MEAN_FILTER_SIZE];
    uint8_t index;
    float sum;
    uint8_t count;
};

struct butterworthFilter_t {
    float v[2];
    float result;
    float a0, a1, b1;
};

/*!
  @class HeartRate
  @brief Utility class for calcurate heart beat rate and SpO2
 */
class HeartRate {
   public:
    /*!
      @param sr Sampling rate (e.g. 167 if max30100::Sampling::Rate167)
      @param threshold Threshold for detect beat (depends on ir/redCUrrent)
      @param store_size Stored data size(0U means auto)
     */
    HeartRate(const uint32_t srate, const float threshold = 125.f,
              const size_t max_data_size = 0U);

    ///@name Settings
    ///@{
    //! @brief Set sampling rate
    void setSamplingRate(const uint32_t sr);
    //! @brief Set threshold
    void setThreshold(const float t) {
        _threshold = t;
    }
    //! @brief Set coefficients for SpO2 calculations
    void setSpO2Coefficients(const float coeff) {
        _coeffSpO2 = coeff;
    }
    ///@}

    //!  @brief Stock sample value
    bool push_back(const float ir, const float red);
    /*!
      @brief Clear inner buffer
      @note When starting measurement again, etc
    */
    void clear();

    ///@name Gets the values
    ///@{
    /*!
      @brief Calculate heart rate from data stocked at the time of call
      @return heart beat rate
     */
    float calculate() const;
    //! @brief Gets the latest SpO2
    float SpO2() const {
        return _SpO2;
    }
    ///@}

   private:
    bool detect_beat();

    float _samplingRate{}, _threshold{};
    size_t _maxDataSize{};
    std::deque<float> _dataIR{};
    std::deque<m5::unit::types::elapsed_time_t> _peakDowns{};
    bool _incrasing{}, _beat{};
    uint32_t _count{};

    dcFilter_t _dcIR{}, _dcRED{};
    meanDiffFilter_t _mdIR{};
    butterworthFilter_t _bwfIR{};

    float _acSqIR{}, _acSqRED{};
    float _SpO2{}, _coeffSpO2{18.0f};

    //    float _calibrationSpO2{};
};

}  // namespace max30100
}  // namespace m5
#endif
