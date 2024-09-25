/*
 * SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
/*!
  @file ui_scale_meter.cpp
  @brief Scale meter
 */
#include "ui_scale_meter.hpp"
#include <M5Utility.h>
#include <cassert>

namespace {
constexpr float table0[] = {0.0f, 0.25f, 0.5f, 0.75f, 1.0f};
constexpr float table1[] = {0.125f, 0.125f * 3, 0.125f * 5, 0.125f * 7};
}  // namespace

namespace m5 {
namespace ui {

void ScaleMeter::render(LovyanGFX* dst, const int32_t x, const int32_t y, const int32_t val)
{
    dst->setClipRect(x, y, width(), height());

    auto rad = _radius - 1;

    // gauge
    int32_t r0{rad}, r1{rad - 1};
    float sdeg{std::fmin(_minDeg, _maxDeg)};
    float edeg{std::fmax(_minDeg, _maxDeg)};
    dst->fillArc(x + _cx, y + _cy, r0, r1, sdeg, edeg, gaugeColor());

    const auto w               = _maxDeg - _minDeg;
    constexpr float deg_to_rad = 0.017453292519943295769236907684886f;
    for (auto&& e : table0) {
        const float f  = _minDeg + w * e;
        const float cf = std::cos(f * deg_to_rad);
        const float sf = std::sin(f * deg_to_rad);
        int32_t sx     = rad * cf;
        int32_t sy     = rad * sf;
        int32_t ex     = (rad - 4) * cf;
        int32_t ey     = (rad - 4) * sf;
        dst->drawLine(x + _cx + sx, y + _cy + sy, x + _cx + ex, y + _cy + ey, gaugeColor());
    }
    for (auto&& e : table1) {
        const float f  = _minDeg + w * e;
        const float cf = std::cos(f * deg_to_rad);
        const float sf = std::sin(f * deg_to_rad);
        int32_t sx     = rad * cf;
        int32_t sy     = rad * sf;
        int32_t ex     = (rad - 2) * cf;
        int32_t ey     = (rad - 2) * sf;
        dst->drawLine(x + _cx + sx, y + _cy + sy, x + _cx + ex, y + _cy + ey, gaugeColor());
    }

    // needle
    float deg  = _minDeg + (_maxDeg - _minDeg) * ratio(val);
    int32_t tx = rad * std::cos(deg * deg_to_rad);
    int32_t ty = rad * std::sin(deg * deg_to_rad);
    dst->drawLine(x + _cx, y + _cy, x + _cx + tx, y + _cy + ty, needleColor());

    dst->clearClipRect();
}

}  // namespace ui
}  // namespace m5
