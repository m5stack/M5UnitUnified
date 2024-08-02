/*
 * SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
/*!
  @file heart_rate.cpp
  @brief Calculate heart rate
*/
#include "heart_rate.hpp"
#include <M5Unified.hpp>
#include <M5Utility.hpp>
#include <cmath>
#include <numeric>

using namespace m5::max30100;

namespace {

constexpr float ALPHA{0.95f};  // for removeDC

void calculateButterworthCoefficients(float Fs, float Fc, float& a0, float& a1,
                                      float& b1) {
    float tanWc = std::tan(M_PI * Fc / Fs);
    float sqrt2 = std::sqrt(2.0f);

    a0 = tanWc / (tanWc + sqrt2);
    a1 = a0;
    b1 = (tanWc - sqrt2) / (tanWc + sqrt2);
}

inline dcFilter_t removeDC(const float x, const float prev_w,
                           const float alpha) {
    dcFilter_t filtered;
    filtered.w      = x + alpha * prev_w;
    filtered.result = filtered.w - prev_w;
    return filtered;
}

float meanDiff(const float M, meanDiffFilter_t& filterValues) {
    float avg{};

    filterValues.sum -= filterValues.values[filterValues.index];
    filterValues.values[filterValues.index] = M;
    filterValues.sum += filterValues.values[filterValues.index];

    filterValues.index++;
    filterValues.index =
        filterValues.index % meanDiffFilter_t::MEAN_FILTER_SIZE;

    if (filterValues.count < meanDiffFilter_t::MEAN_FILTER_SIZE) {
        filterValues.count++;
    }

    avg = filterValues.sum / filterValues.count;
    return avg - M;
}

void lowPassButterworthFilter(const float x, butterworthFilter_t& fr) {
    fr.v[0]   = fr.v[1];
    fr.v[1]   = (fr.a0 * x) + (fr.a1 * fr.v[0]) - (fr.b1 * fr.v[1]);
    fr.result = fr.v[0] + fr.v[1];
}

#if 0
std::pair<float /*AC*/, float /*DC*/> calculateACDC(
    const std::deque<float>& data) {
    float maxVal = *std::max_element(data.begin(), data.end());
    float minVal = *std::min_element(data.begin(), data.end());
    return {maxVal - minVal,
            std::accumulate(data.begin(), data.end(), 0.0f) / data.size()};
}
#endif

}  // namespace

namespace m5 {
namespace max30100 {

HeartRate::HeartRate(const uint32_t srate, const float threshold,
                     const size_t max_data_size)
    : _samplingRate{srate}, _threshold(threshold), _maxDataSize{max_data_size} {
    assert(srate && "SamplingRate must not be zero");
    if (!max_data_size) {
        _maxDataSize = (size_t)srate * 30U;
    }
    calculateButterworthCoefficients(_samplingRate, 10.0f, _bwfIR.a0, _bwfIR.a1,
                                     _bwfIR.b1);
}

void HeartRate::setSamplingRate(const uint32_t sr) {
    _samplingRate = sr;
    calculateButterworthCoefficients(_samplingRate, 10.0f, _bwfIR.a0, _bwfIR.a1,
                                     _bwfIR.b1);
    clear();
}

void HeartRate::clear() {
    _dataIR.clear();
    _peakDowns.clear();
    _incrasing = false;
    _dcIR      = dcFilter_t{};
    _mdIR      = meanDiffFilter_t{};
    _bwfIR     = butterworthFilter_t{};
    calculateButterworthCoefficients(_samplingRate, 10.0f, _bwfIR.a0, _bwfIR.a1,
                                     _bwfIR.b1);
}

bool HeartRate::push_back(const float ir, const float red) {
    // Filtering (IR)
    _dcIR = removeDC(ir, _dcIR.w, ALPHA);
    // M5_LIB_LOGI("\n>ACIR:%f", _dcIR.result);
    auto md = meanDiff(_dcIR.result, _mdIR);
    // M5_LIB_LOGI("\n>MD:%f", md);
    lowPassButterworthFilter(md, _bwfIR);
    // M5_LIB_LOGI("\n>LP:%f", _bwfIR.result);

    // Filtering (RED)
    _dcRED = removeDC(red, _dcRED.w, ALPHA);
    // M5_LIB_LOGI("\n>ACRED:%f", _dcRED.result);

    // Store
    _dataIR.push_back(_bwfIR.result);
    if (_dataIR.size() > _maxDataSize) {
        _dataIR.pop_front();
    }

    // Heart beat
    _beat = detect_beat();

    // SpO2
    _acSqIR += _dcIR.result * _dcIR.result;
    _acSqRED = _dcRED.result * _dcRED.result;
    ++_count;
    if (_beat) {
        float rr = std::log(std::sqrt(_acSqRED / _count)) /
                   std::log(std::sqrt(_acSqIR / _count));
        auto s  = 110.0f - _coeffSpO2 * rr;
        _SpO2   = std::fmin(std::fmax(s, 80.f), 100.f);
        _acSqIR = _acSqRED = 0.0f;
        _count             = 0;
    }
    return _beat;
}

float HeartRate::calculate() const {
    if (_peakDowns.size() < 2) {
        return 0.0f;
    }

    float total{};
    for (size_t i = 1; i < _peakDowns.size(); ++i) {
        total += _peakDowns[i] - _peakDowns[i - 1];
    }
    float avg = total / (_peakDowns.size() - 1);
    return 1.0f / avg * 60000.0f;
}

bool HeartRate::detect_beat() {
    auto now = m5::utility::millis();
    bool beat{};

    if (_dataIR.size() >= 4) {
        auto d3 = _dataIR.back();
        auto d2 = _dataIR[_dataIR.size() - 2];
        auto d1 = _dataIR[_dataIR.size() - 3];
        auto d0 = _dataIR[_dataIR.size() - 4];
        if (!_incrasing && (d0 > _threshold) && (d1 > d0) && (d2 > d1) &&
            (d3 > d2)) {
            _incrasing = true;
        } else if (_incrasing && (d1 < d0) && (d2 < d1) && (d3 < d2)) {
            _incrasing = false;
            beat       = true;
            _peakDowns.push_back(now);
        }
    }
    // Remove old peaks from the window
    while (!_peakDowns.empty() && now - _peakDowns.front() > 1000 * 10) {
        _peakDowns.pop_front();
    }
    return beat;
}

}  // namespace max30100
}  // namespace m5
