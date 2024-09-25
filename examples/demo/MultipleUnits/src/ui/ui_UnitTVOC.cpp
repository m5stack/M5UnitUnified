/*
 * SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
/*!
  @file ui_UnitTVOC.cpp
  @brief UI for UnitTVOC
 */
#include "ui_UnitTVOC.hpp"
#include <M5Unified.h>
#include <cassert>

namespace {
const int32_t GAP{2};

constexpr m5gfx::rgb565_t co2_gauge_color{38, 41, 64};
constexpr m5gfx::rgb565_t tvoc_gauge_color{64, 48, 26};
constexpr char co2ustr[]  = "ppm";
constexpr char tvocustr[] = "ppb";

constexpr std::initializer_list<m5::ui::ColorRange> tvocGauge = {
    {220, m5gfx::rgb565_t(TFT_GREEN)}, {660, m5gfx::rgb565_t(TFT_GOLD)},    {1430, m5gfx::rgb565_t(TFT_ORANGE)},
    {2000, m5gfx::rgb565_t(TFT_RED)},  {3300, m5gfx::rgb565_t(TFT_VIOLET)}, {5500, m5gfx::rgb565_t(TFT_PURPLE)},
};

}  // namespace

void UnitTVOCSmallUI::construct()
{
    auto& lcd = M5.Display;
    _wid      = lcd.width() >> 1;
    _hgt      = lcd.height() >> 1;

    auto bw = _wid / 6 - GAP * 2;
    auto bh = (_hgt >> 1) - GAP * 2 - 8;

    auto pw = _wid / 6 * 5 - GAP * 2;
    auto ph = (_hgt >> 1) - GAP * 2 - 8;

    _co2Bar.reset(new m5::ui::BarMeterV(_parent, 400, 6000, bw, bh));
    _tvocBar.reset(new m5::ui::ColorBarMeterV(_parent, 0, 5500, bw, bh, tvocGauge));

    _co2Plotter.reset(new m5::ui::Plotter(_parent, pw, pw, ph));
    _co2Plotter->setGaugeColor(co2_gauge_color);
    _co2Plotter->setUnitString(co2ustr);
    _co2Plotter->setGaugeTextDatum(textdatum_t::top_right);

    _tvocPlotter.reset(new m5::ui::Plotter(_parent, pw, pw, ph));
    _tvocPlotter->setGaugeColor(tvoc_gauge_color);
    _tvocPlotter->setUnitString(tvocustr);

    _intermediateBuffer.reserve(pw);
    _intermediateBuffer.clear();
}

void UnitTVOCSmallUI::push_back(const int32_t co2, const int32_t tvoc)
{
    _co2Bar->animate(co2, 10);
    _tvocBar->animate(tvoc, 10);
    _intermediateBuffer.emplace_back(Data{co2, tvoc});
}

void UnitTVOCSmallUI::update()
{
    lock();
    for (auto&& e : _intermediateBuffer) {
        _co2Plotter->push_back(e.co2);
        _tvocPlotter->push_back(e.tvoc);
    }
    _intermediateBuffer.clear();
    unlock();

    _co2Bar->update();
    _tvocBar->update();
    _co2Plotter->update();
    _tvocPlotter->update();
}

void UnitTVOCSmallUI::push(LovyanGFX* dst, const int32_t x, const int32_t y)
{
    auto left   = x;
    auto right  = x + _wid - 1;
    auto top    = y;
    auto bottom = y + _hgt - 1;
    auto w      = right - left + 1;
    auto h      = bottom - top + 1;

    auto f = dst->getFont();
    dst->setFont(&fonts::Font0);
    dst->setTextColor(TFT_WHITE);
    auto td = dst->getTextDatum();

    // BG
    dst->fillRoundRect(x, y, _wid, _hgt, GAP, TFT_BLUE);
    dst->fillRoundRect(x + GAP, y + GAP, _wid - GAP * 2, _hgt - GAP * 2, GAP, TFT_BLACK);

    _co2Bar->push(dst, left + GAP, y + GAP);
    _co2Plotter->push(dst, right - _co2Plotter->width() - GAP, y + GAP);

    _tvocPlotter->push(dst, left + GAP, bottom - _tvocPlotter->height() - GAP);
    _tvocBar->push(dst, right - _tvocBar->width() - GAP, bottom - _tvocPlotter->height() - GAP);

    dst->drawString("CO2eq", left + GAP * 3 + _co2Bar->width(), y + GAP);
    dst->setTextDatum(textdatum_t::bottom_right);
    dst->drawString("TVOC", right - (GAP * 2 + _tvocBar->width()), bottom);

    dst->setFont(&fonts::Font2);
    dst->setTextColor(TFT_BLUE);
    dst->setTextDatum(textdatum_t::middle_center);
    dst->drawString("UnitTVOC", x + w / 2, y + h / 2);

    dst->setTextDatum(td);
    dst->setFont(f);
}
// current valiue
