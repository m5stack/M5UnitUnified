/*
 * SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
/*!
  @file ui_scale_meter.hpp
  @brief Scale meter
 */
#ifndef UI_SCALE_METER_HPP
#define UI_SCALE_METER_HPP

#include "ui_base.hpp"

namespace m5 {
namespace ui {

class ScaleMeter : public Base {
public:
    ScaleMeter(LovyanGFX* parent, const int32_t minimum, const int32_t maximum, const float minDeg, const float maxDeg,
               const int32_t wid, const int32_t hgt, const int32_t cx, const int32_t cy, const uint32_t radius)
        : Base(parent, minimum, maximum, wid, hgt), _cx(cx), _cy(cy), _radius(radius), _minDeg{minDeg}, _maxDeg{maxDeg}
    {
    }

    virtual ~ScaleMeter()
    {
    }

protected:
    virtual void render(LovyanGFX* dst, const int32_t x, const int32_t y, const int32_t val) override;

private:
    int32_t _cx{}, _cy{}, _radius{};
    float _minDeg{}, _maxDeg{};
};

}  // namespace ui
}  // namespace m5

#endif
