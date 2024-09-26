/*
 * SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
/*!
  @file ui_gauge_meter.hpp
  @brief Gauge meter
 */
#ifndef UI_GAUGE_METER_HPP
#define UI_GAUGE_METER_HPP

#include "ui_base.hpp"

namespace m5 {
namespace ui {

class GaugeMeter : public Base {
public:
    GaugeMeter(LovyanGFX* parent, const int32_t minimum, const int32_t maximum, const float minDeg, const float maxDeg,
               const int32_t wid, const int32_t hgt, const int32_t thickness);
    GaugeMeter(LovyanGFX* parent, const int32_t minimum, const int32_t maximum, const float minDeg, const float maxDeg,
               const int32_t wid, const int32_t hgt, const int32_t cx, const int32_t cy, const int32_t radius,
               const int32_t thickness);

    virtual ~GaugeMeter()
    {
    }

protected:
    virtual void render(LovyanGFX* dst, const int32_t x, const int32_t y, const int32_t val) override;

private:
    int32_t _cx{}, _cy{}, _radius{}, _thickness{};
    float _minDeg{}, _maxDeg{};
};

}  // namespace ui
}  // namespace m5

#endif
