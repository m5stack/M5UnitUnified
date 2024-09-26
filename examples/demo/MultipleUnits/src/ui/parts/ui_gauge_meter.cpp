/*
 * SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
/*!
  @file ui_gauge_meter.cpp
  @brief Gauge meter
 */
#include "ui_gauge_meter.hpp"
#include <M5Utility.h>
#include <cassert>

namespace m5 {
namespace ui {

GaugeMeter::GaugeMeter(LovyanGFX* parent, const int32_t minimum, const int32_t maximum, const float minDeg,
                       const float maxDeg, const int32_t wid, const int32_t hgt, const int32_t thickness)
    : GaugeMeter(parent, minimum, maximum, minDeg, maxDeg, wid, hgt, (wid >> 1) - 1, (hgt >> 1) - 1,
                 std::min(wid >> 1, hgt >> 1) - 1, thickness)
{
}

GaugeMeter::GaugeMeter(LovyanGFX* parent, const int32_t minimum, const int32_t maximum, const float minDeg,
                       const float maxDeg, const int32_t wid, const int32_t hgt, const int32_t cx, const int32_t cy,
                       const int32_t radius, const int32_t thickness)
    : Base(parent, minimum, maximum, wid, hgt),
      _cx(cx),
      _cy(cy),
      _radius(radius),
      _thickness{thickness},
      _minDeg{minDeg},
      _maxDeg{maxDeg}
{
}

void GaugeMeter::render(LovyanGFX* dst, const int32_t x, const int32_t y, const int32_t val)
{
    dst->setClipRect(x, y, width(), height());

    int32_t r0{_radius}, r1{_radius - _thickness};
    float sdeg{std::fmin(_minDeg, _maxDeg)};
    float edeg{std::fmax(_minDeg, _maxDeg)};
    // float sdeg{_minDeg};
    // float edeg{_maxDeg};

    dst->fillArc(x + _cx, y + _cy, r0, r1, sdeg, edeg, backgroundColor());
    float deg = _minDeg + (_maxDeg - _minDeg) * ratio(val);
    dst->fillArc(x + _cx, y + _cy, r0, r1, sdeg, deg, needleColor());

    dst->drawArc(x + _cx, y + _cy, r0, r1, sdeg, edeg, gaugeColor());

    dst->clearClipRect();
}

}  // namespace ui
}  // namespace m5
