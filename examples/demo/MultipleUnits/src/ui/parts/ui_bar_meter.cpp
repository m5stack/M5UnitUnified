/*
 * SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
/*!
  @file ui_bar_meter.cpp
  @brief Bar meter
 */
#include "ui_bar_meter.hpp"
#include <M5Utility.h>

namespace {
constexpr float table0[] = {0.0f, 0.25f, 0.5f, 0.75f, 1.0f};
constexpr float table1[] = {0.125f, 0.125f * 3, 0.125f * 5, 0.125f * 7};

}  // namespace

namespace m5 {
namespace ui {

void BarMeterH::render(LovyanGFX* dst, const int32_t x, const int32_t y, const int32_t val)
{
    dst->setClipRect(x, y, width(), height());
    const auto t   = height() >> 3;
    const auto gy  = y + t * 6;
    const auto gw  = width() - 1;
    const auto gh0 = t;
    const auto gh1 = gh0 >> 1;

    dst->fillRect(x, y, width(), height(), TFT_BLUE);

    // gauge
    dst->drawFastHLine(x, gy, width(), gaugeColor());

    for (auto&& e : table0) {
        dst->drawFastVLine(x + gw * e, gy - gh0, gh0, gaugeColor());
    }
    for (auto&& e : table1) {
        dst->drawFastVLine(x + gw * e, gy - gh1, gh1, gaugeColor());
    }
    // needle
    dst->drawFastVLine(x + gw * ratio(val), y, height(), needleColor());

    dst->clearClipRect();
}

void BarMeterV::render(LovyanGFX* dst, const int32_t x, const int32_t y, const int32_t val)
{
    dst->setClipRect(x, y, width(), height());
    const auto t   = width() >> 3;
    const auto gx  = x + t * 6;
    const auto gh  = height() - 1;
    const auto gw0 = t;
    const auto gw1 = gw0 >> 1;

    // gauge
    dst->drawFastVLine(gx, y, height(), gaugeColor());

    for (auto&& e : table0) {
        dst->drawFastHLine(gx - gw0, y + gh * e, gw0, gaugeColor());
    }
    for (auto&& e : table1) {
        dst->drawFastHLine(gx - gw1, y + gh * e, gw1, gaugeColor());
    }
    // needle
    dst->drawFastHLine(x, y + gh * (1.0f - ratio(val)), width(), needleColor());

    dst->clearClipRect();
}

void ColorBarMeterH::render(LovyanGFX* dst, const int32_t x, const int32_t y, const int32_t val)
{
    const auto w = width();
    const auto t = height() >> 3;
    const auto h = t << 2;
    auto left    = x;
    auto top     = y + height() / 2 - height() / 4;
    auto gw      = width() - 1;

    dst->setClipRect(x, y, width(), height());

    // gauge
    if (!_crange.empty()) {
        dst->fillRect(left, top, w, h, _crange.back().clr);
        for (auto it = _crange.crbegin() + 1; it != _crange.crend(); ++it) {
            int32_t ww = w * ratio(it->lesseq);
            dst->fillRect(left, top, ww, h, it->clr);
        }
    } else {
        dst->fillRect(left, top, w, h, backgroundColor());
    }
    dst->drawRect(left, top, w, h, gaugeColor());
    // needle
    dst->drawFastVLine(left + gw * ratio(val), y, height(), needleColor());

    dst->clearClipRect();
}

void ColorBarMeterV::render(LovyanGFX* dst, const int32_t x, const int32_t y, const int32_t val)
{
    const auto h = height();
    const auto w = width() >> 1;
    auto top     = y;
    auto left    = x + width() / 2 - width() / 4;
    auto gh      = height() - 1;

    dst->setClipRect(x, y, width(), height());

    // gauge
    if (!_crange.empty()) {
        dst->fillRect(left, top, w, h, _crange.back().clr);
        for (auto it = _crange.crbegin() + 1; it != _crange.crend(); ++it) {
            int32_t hh = h * ratio(it->lesseq);
            dst->fillRect(left, top + height() - hh, w, hh, it->clr);
        }
    } else {
        dst->fillRect(left, top, w, h, backgroundColor());
    }
    dst->drawRect(left, top, w, h, gaugeColor());
    // needle
    dst->drawFastHLine(x, y + gh * (1.0f - ratio(val)), width(), needleColor());

    dst->clearClipRect();
}

}  // namespace ui
}  // namespace m5
