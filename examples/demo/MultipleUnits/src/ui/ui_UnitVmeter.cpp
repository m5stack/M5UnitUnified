/*
 * SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
/*!
  @file ui_UnitVmeter.cpp
  @brief UI for UnitVmeter
 */
#include "ui_UnitVmeter.hpp"
#include <M5Unified.h>
#include <M5Utility.h>
#include <cassert>

namespace {
constexpr int32_t GAP{2};

constexpr m5gfx::rgb565_t voltage_gauge_color{129, 134, 80};

constexpr char vustr[] = "mV";
}  // namespace

void UnitVmeterSmallUI::construct()
{
    auto& lcd = M5.Display;
    _wid      = lcd.width() >> 1;
    _hgt      = lcd.height() >> 1;

    auto pw = _wid - GAP * 2;
    //    auto ph = (_hgt >> 2) * 3 - GAP * 2;
    auto ph = (_hgt >> 1) - GAP * 2 - 4;

    _voltagePlotter.reset(new m5::ui::Plotter(_parent, pw, pw, ph));
    _voltagePlotter->setUnitString(vustr);
    _voltagePlotter->setGaugeColor(voltage_gauge_color);

    _intermediateBuffer.reserve(pw);
    _intermediateBuffer.clear();
}

void UnitVmeterSmallUI::push_back(const float mv)
{
    _intermediateBuffer.emplace_back(mv);
}

void UnitVmeterSmallUI::update()
{
    lock();
    for (auto&& e : _intermediateBuffer) {
        _voltagePlotter->push_back(e);
    }
    _intermediateBuffer.clear();
    unlock();
    _voltagePlotter->update();
}
void UnitVmeterSmallUI::push(LovyanGFX* dst, const int32_t x, const int32_t y)
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
    dst->fillRoundRect(x, y, _wid, _hgt, GAP, TFT_RED);
    dst->fillRoundRect(x + GAP, y + GAP, _wid - GAP * 2, _hgt - GAP * 2, GAP, TFT_BLACK);

    _voltagePlotter->push(dst, x + GAP, y + GAP);

    auto s = m5::utility::formatString("MIN:%5dmV", _voltagePlotter->minimum());
    dst->drawString(s.c_str(), x + GAP * 2, y + GAP * 2 + _voltagePlotter->height() + 16);
    s = m5::utility::formatString("MAX:%5dmV", _voltagePlotter->maximum());
    dst->drawString(s.c_str(), x + GAP * 2, y + GAP * 2 + _voltagePlotter->height() + 16 + 10 * 1);

    dst->setFont(&fonts::Font2);
    dst->setTextColor(TFT_RED);
    dst->setTextDatum(textdatum_t::middle_center);
    dst->drawString("UnitVmeter", left + w / 2, top + h / 2);

    dst->setTextDatum(td);
    dst->setFont(f);
}
