/*
 * SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
/*!
  @file ui_UnitHEART.cpp
  @brief UI for UnitHEART
 */
#include "ui_UnitHEART.hpp"
#include <M5Unified.h>
#include <M5Utility.h>
#include <cassert>

namespace {
constexpr int32_t GAP{2};
constexpr float COEFF{100.0f};
constexpr float COEFF_RECIPROCAL{1.0f / COEFF};

constexpr m5gfx::rgb565_t ir_gauge_color{161, 54, 54};
constexpr m5gfx::rgb565_t spo2_gauge_color{38, 41, 64};

constexpr int32_t min_spo2{90};
constexpr int32_t max_spo2{100};

constexpr char spO2ustr[] = "%";
}  // namespace

void UnitHEARTSmallUI::construct()
{
    auto& lcd = M5.Display;
    _wid      = lcd.width() >> 1;
    _hgt      = lcd.height() >> 1;

    auto gw = _wid - GAP * 2;
    auto gh = (_hgt >> 1) - (GAP * 2 + 16);

    _irPlotter.reset(new m5::ui::Plotter(_parent, gw, gw, gh));
    _irPlotter->setGaugeColor(ir_gauge_color);
    _spO2Plotter.reset(new m5::ui::Plotter(_parent, gw, min_spo2 * COEFF, max_spo2 * COEFF, gw, gh, COEFF));
    _spO2Plotter->setGaugeColor(spo2_gauge_color);
    _spO2Plotter->setUnitString(spO2ustr);
    _spO2Plotter->setGaugeTextDatum(textdatum_t::top_right);
}

void UnitHEARTSmallUI::push_back(const int32_t ir, const int32_t red)
{
    _intermediateBuffer.emplace_back(Data{ir, red});
}

void UnitHEARTSmallUI::update()
{
    if (_beatCounter > 0) {
        --_beatCounter;
    }

    lock();
    for (auto&& e : _intermediateBuffer) {
        _monitor.push_back(e.ir, e.red);
        _monitor.update();

        beat(_monitor.isBeat());
        _bpm = _monitor.bpm();

        //        _irPlotter->push_back(e.ir);
        _irPlotter->push_back(_monitor.latestIR());
        _spO2Plotter->push_back(_monitor.SpO2());
    }
    _intermediateBuffer.clear();
    unlock();

    _irPlotter->update();
    _spO2Plotter->update();
}

void UnitHEARTSmallUI::push(LovyanGFX* dst, const int32_t x, const int32_t y)
{
    auto f = dst->getFont();
    dst->setFont(&fonts::Font0);
    dst->setTextColor(TFT_WHITE);
    auto td = dst->getTextDatum();

    auto left   = x;
    auto right  = x + _wid - 1;
    auto top    = y;
    auto bottom = y + _hgt - 1;
    auto w      = right - left + 1;
    auto h      = bottom - top + 1;

    // BG
    dst->fillRoundRect(x, y, _wid, _hgt, GAP, TFT_YELLOW);
    dst->fillRoundRect(x + GAP, y + GAP, _wid - GAP * 2, _hgt - GAP * 2, GAP, TFT_BLACK);

    _irPlotter->push(dst, x + GAP, y + GAP);
    _spO2Plotter->push(dst, x + GAP, y + _hgt - GAP - _spO2Plotter->height());

    auto s = m5::utility::formatString("HR:%3dbpm", _bpm);
    dst->drawString(s.c_str(), left + GAP * 2, top + GAP * 2 + _irPlotter->height());

    dst->setTextDatum(textdatum_t::bottom_right);
    s = m5::utility::formatString("SpO2:%3.2f%%", _monitor.SpO2());
    dst->drawString(s.c_str(), right - GAP, bottom - _spO2Plotter->height() - GAP);

    constexpr int32_t radius{4};
    dst->fillCircle(right - radius * 2 - GAP, top + GAP * 2 + _irPlotter->height() + radius, radius,
                    _beatCounter > 0 ? TFT_RED : TFT_DARKGRAY);

    dst->setFont(&fonts::Font2);
    dst->setTextDatum(textdatum_t::middle_center);
    dst->setTextColor(TFT_YELLOW);
    dst->drawString("UnitHEART", left + w / 2, top + h / 2);

    dst->setTextDatum(td);
    dst->setFont(f);
}
