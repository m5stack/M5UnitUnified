/*
 * SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
/*!
  @file ui_UnitCO2.cpp
  @brief UI for UnitCO2
 */
#include "ui_UnitCO2.hpp"
#include <M5Unified.h>
#include <M5Utility.h>
#include <iterator>

namespace {
constexpr int32_t GAP{2};
constexpr float COEFF{100.0f};
constexpr float COEFF_RECIPROCAL{1.0f / COEFF};

constexpr int32_t min_co2{0};
constexpr int32_t max_co2{6000};
constexpr int32_t min_temp{-10};
constexpr int32_t max_temp{40};

m5gfx::rgb565_t temp_chooseColor(const int32_t val)
{
    return val > 0 ? m5gfx::rgb565_t(0xfe, 0xcb, 0xf2) : m5gfx::rgb565_t(0xb8, 0xc2, 0xf2);
}

constexpr std::initializer_list<m5::ui::ColorRange> co2_color_table = {
    {1000, m5gfx::rgb565_t(TFT_GREEN)},
    {1500, m5gfx::rgb565_t(TFT_GOLD)},
    {2500, m5gfx::rgb565_t(TFT_ORANGE)},
    {6000, m5gfx::rgb565_t(TFT_RED)},
};

m5gfx::rgb565_t co2_chooseColor(const int32_t val)
{
    for (auto&& e : co2_color_table) {
        if (val <= e.lesseq) {
            return e.clr;
        }
    }
    return (std::end(co2_color_table) - 1)->clr;
}

}  // namespace

void UnitCO2SmallUI::construct()
{
    auto& lcd  = M5.Display;
    _wid       = lcd.width() >> 1;
    _hgt       = lcd.height() >> 1;
    int32_t wh = std::max(_wid / 2, _hgt / 2) - GAP * 2;

    _tempMeter.reset(new m5::ui::GaugeMeter(_parent, min_temp * COEFF, max_temp * COEFF, 90.0f + 45.0f,
                                            90.0f - 45.0f + 360.f, wh, wh, 10));
    _tempMeter->set(0);

    _co2Meter.reset(
        new m5::ui::ColorBarMeterH(_parent, min_co2, max_co2, _wid - GAP * 2, _hgt - wh - GAP * 2, co2_color_table));
}

void UnitCO2SmallUI::push_back(const int32_t co2, const float temp)
{
    _tempMeter->animate(temp * COEFF, 1000);
    _co2Meter->animate(co2, 1000);
}

void UnitCO2SmallUI::update()
{
    _tempMeter->update();
    _tempMeter->setNeedleColor(temp_chooseColor(_tempMeter->value()));
    _co2Meter->update();
}

void UnitCO2SmallUI::push(LovyanGFX* dst, const int32_t x, const int32_t y)
{
    auto left   = x;
    auto right  = x + _wid - 1;
    auto top    = y;
    auto bottom = y + _hgt - 1;
    auto w      = right - left + 1;
    auto h      = bottom - top + 1;

    // BG
    dst->fillRoundRect(x, y, _wid, _hgt, GAP * 2, TFT_MAGENTA);
    dst->fillRoundRect(x + GAP, y + GAP, _wid - GAP * 2, _hgt - GAP * 2, GAP, TFT_BLACK);

    auto tcx = left + GAP;
    auto tcy = top + GAP;
    _tempMeter->push(dst, x + GAP, y + GAP);
    _co2Meter->push(dst, x + GAP, tcy + _tempMeter->height() + GAP * 2);

    //
    auto f = dst->getFont();
    dst->setFont(&fonts::Font0);
    dst->setTextColor(TFT_WHITE);
    auto td = dst->getTextDatum();

    dst->setTextDatum(textdatum_t::middle_center);
    auto s = m5::utility::formatString("%3.2fC", _tempMeter->value() * COEFF_RECIPROCAL);
    dst->drawString(s.c_str(), tcx + _tempMeter->width() / 2, tcy + _tempMeter->height() / 2);
    dst->drawString("TEMP", tcx + _tempMeter->width() / 2, tcy + _tempMeter->height() / 2 + 10);

    dst->setTextDatum(textdatum_t::top_right);
    dst->drawString("CO2", right - GAP, top + _tempMeter->height() + GAP - 10);
    auto sw = dst->drawString("ppm", right - GAP, top + _tempMeter->height() + GAP);
    dst->setTextColor((uint16_t)co2_chooseColor(_co2Meter->value()));
    s = m5::utility::formatString("%d", _co2Meter->value());
    dst->drawString(s.c_str(), right - GAP - sw, top + _tempMeter->height() + GAP);

    dst->setFont(&fonts::Font2);
    dst->setTextColor(TFT_WHITE);
    dst->setTextDatum(textdatum_t::middle_center);
    dst->drawString("UnitCO2", left + w / 4 * 3, top + h / 4);

    dst->setTextDatum(td);
    dst->setFont(f);
}
