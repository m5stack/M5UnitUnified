/*
 * SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
/*!
  @file ui_UnitENV3.cpp
  @brief UI for UnitENV3
 */
#include "ui_UnitENV3.hpp"
#include <M5Unified.h>
#include <M5Utility.h>
#include <cassert>

namespace {
constexpr int32_t GAP{2};
constexpr float COEFF{100.0f};
constexpr float COEFF_RECIPROCAL{1.0f / COEFF};

constexpr m5gfx::rgb565_t hum_needle_color{32, 147, 223};
constexpr m5gfx::rgb565_t pres_needle_color{161, 54, 64};

constexpr int32_t min_temp{-10};
constexpr int32_t max_temp{40};
constexpr int32_t min_hum{0};
constexpr int32_t max_hum{100};
constexpr int32_t min_pres{0};
constexpr int32_t max_pres{1500};

m5gfx::rgb565_t temp_chooseColor(const int32_t val)
{
    return val > 0 ? m5gfx::rgb565_t(0xfe, 0xcb, 0xf2) : m5gfx::rgb565_t(0xb8, 0xc2, 0xf2);
}

}  // namespace

void UnitENV3SmallUI::construct()
{
    auto& lcd = M5.Display;
    _wid      = lcd.width() >> 1;
    _hgt      = lcd.height() >> 1;

    //    auto left   = 0 + GAP;
    //    auto right  = _wid - GAP - 1;
    //    auto top    = 0;
    //    auto bottom = _hgt - 1;
    auto w   = _wid / 5;
    auto h   = _hgt / 2 - GAP * 2;
    auto rad = std::min(w, h / 2);
    auto wh  = (std::min(_wid, _hgt) >> 1) - GAP * 2;

    _tempMeterSHT.reset(
        new m5::ui::GaugeMeter(_parent, min_temp * COEFF, max_temp * COEFF, 25.0f, -25.0f + 360.f, wh, wh, 10));
    _tempMeterQMP.reset(
        new m5::ui::GaugeMeter(_parent, min_temp * COEFF, max_temp * COEFF, 25.0f, -25.0f + 360.f, wh, wh, 10));
    _humMeter.reset(
        new m5::ui::ScaleMeter(_parent, min_hum * COEFF, max_hum * COEFF, 360.0f + 90.0f, 270.0f, w, h, 0, h / 2, rad));
    _presMeter.reset(new m5::ui::ScaleMeter(_parent, min_pres * COEFF, max_pres * COEFF, 360.0f + 90.0f, 270.0f, w, h,
                                            0, h / 2, rad));

    _humMeter->setNeedleColor(hum_needle_color);
    _presMeter->setNeedleColor(pres_needle_color);
}

void UnitENV3SmallUI::sht30_push_back(const float tmp, const float hum)
{
    _tempMeterSHT->animate(tmp * COEFF, 10);
    _humMeter->animate(hum * COEFF, 10);
}

void UnitENV3SmallUI::qmp6988_push_back(const float tmp, const float pa)
{
    _tempMeterQMP->animate(tmp * COEFF, 10);
    _presMeter->animate(pa * COEFF * 0.01f, 10);  // pa to hPa
}

void UnitENV3SmallUI::update()
{
    lock();
    _tempMeterSHT->update();
    _tempMeterQMP->update();
    _tempMeterSHT->setNeedleColor(temp_chooseColor(_tempMeterSHT->value()));
    _tempMeterQMP->setNeedleColor(temp_chooseColor(_tempMeterQMP->value()));
    _humMeter->update();
    _presMeter->update();
    unlock();
}

void UnitENV3SmallUI::push(LovyanGFX* dst, const int32_t x, const int32_t y)
{
    auto left   = x;
    auto right  = x + _wid - 1;
    auto top    = y;
    auto bottom = y + _hgt - 1;
    auto w      = right - left + 1;
    auto h      = bottom - top + 1;

    auto sx = left + GAP;
    auto sy = top + GAP;
    auto qx = left + GAP;
    auto qy = top + h / 2 + GAP;
    auto hx = right - (_humMeter->width() + GAP);
    auto hy = top + GAP;
    auto px = right - (_presMeter->width() + GAP);
    auto py = top + h / 2 + GAP;

    auto f = dst->getFont();
    dst->setFont(&fonts::Font0);
    dst->setTextColor(TFT_WHITE);
    auto td = dst->getTextDatum();

    // BG
    dst->fillRoundRect(x, y, _wid, _hgt, GAP, TFT_GREEN);
    dst->fillRoundRect(x + GAP, y + GAP, _wid - GAP * 2, _hgt - GAP * 2, GAP, TFT_BLACK);

    // meters
    _tempMeterSHT->push(dst, sx, sy);
    _humMeter->push(dst, hx, hy);
    _tempMeterQMP->push(dst, qx, qy);
    _presMeter->push(dst, px, py);

    // text
    dst->setTextDatum(textdatum_t::middle_left);
    auto s = m5::utility::formatString("T:%3.2fC", _tempMeterSHT->value() * COEFF_RECIPROCAL);
    dst->drawString(s.c_str(), sx + 16, sy + _tempMeterSHT->height() / 2);
    s = m5::utility::formatString("T:%3.2fC", _tempMeterQMP->value() * COEFF_RECIPROCAL);
    dst->drawString(s.c_str(), qx + 16, qy + _tempMeterQMP->height() / 2);
    dst->setTextDatum(textdatum_t::middle_right);
    s = m5::utility::formatString("H:%3.2f%%", _humMeter->value() * COEFF_RECIPROCAL);
    dst->drawString(s.c_str(), hx, hy + _humMeter->height() / 2);
    s = m5::utility::formatString("P:%4.0f", _presMeter->value() * COEFF_RECIPROCAL);
    dst->drawString(s.c_str(), px, py + _presMeter->height() / 2);

#if 0
    auto s = m5::utility::formatString("%dC", max_temp);
    dst->drawString(s.c_str(), sx, sy);
    dst->drawString(s.c_str(), qx, qy);

    s = m5::utility::formatString("%dC", (max_temp - min_temp) / 2);
    dst->setTextDatum(textdatum_t::middle_left);
    dst->drawString(s.c_str(), sx, sy + _tempMeterSHT->height() / 2);
    dst->drawString(s.c_str(), qx, qy + _tempMeterSHT->height() / 2);

    dst->setTextDatum(textdatum_t::bottom_left);
    s = m5::utility::formatString("%dC", min_temp);
    dst->drawString(s.c_str(), sx, sy + _tempMeterSHT->height());
    dst->drawString(s.c_str(), qx, qy + _tempMeterSHT->height());
#endif

    dst->setTextDatum(textdatum_t::top_right);
    s = m5::utility::formatString("%d%%", max_hum);
    dst->drawString(s.c_str(), right - GAP, hy);
    dst->setTextDatum(textdatum_t::bottom_right);
    s = m5::utility::formatString("%d%%", min_hum);
    dst->drawString(s.c_str(), right - GAP, hy + _humMeter->height());

    dst->setTextDatum(textdatum_t::top_right);
    s = m5::utility::formatString("%dhPa", max_pres);
    dst->drawString(s.c_str(), right - GAP, py);
    dst->setTextDatum(textdatum_t::bottom_right);
    s = m5::utility::formatString("%dhPa", min_pres);
    dst->drawString(s.c_str(), right - GAP, py + _presMeter->height());

    //
#if 0
    dst->setTextDatum(textdatum_t::top_left);
    dst->drawString("SHT30", sx + _tempMeterSHT->width() + GAP, sy + 10);
    s = m5::utility::formatString(" T: %3.2f C", _tempMeterSHT->valueTo() * COEFF_RECIPROCAL);
    dst->drawString(s.c_str(), sx + _tempMeterSHT->width() + GAP, sy + 10 * 2);
    s = m5::utility::formatString(" H: %3.2f RH", _humMeter->valueTo() * COEFF_RECIPROCAL);
    dst->drawString(s.c_str(), sx + _tempMeterSHT->width() + GAP, sy + 10 * 3);
    dst->setTextDatum(textdatum_t::bottom_right);
    dst->drawString("QMP6988", right - (_tempMeterSHT->width() + GAP), bottom - GAP - 10 + 1);
    s = m5::utility::formatString(" T: %4.2f C", _tempMeterQMP->valueTo() * COEFF_RECIPROCAL);
    dst->drawString(s.c_str(), right - (_tempMeterSHT->width() + GAP), bottom - GAP - 10 * 2);
    s = m5::utility::formatString(" P: %4.2f hPa", _presMeter->valueTo() * COEFF_RECIPROCAL);
    dst->drawString(s.c_str(), right - (_tempMeterSHT->width() + GAP), bottom - GAP - 10 * 3);
#endif

    dst->setTextDatum(textdatum_t::top_center);
    dst->drawString("SHT30", x + w / 2, top + GAP);
    dst->setTextDatum(textdatum_t::bottom_center);
    dst->drawString("QMP6988", x + w / 2, bottom - GAP);

    dst->setFont(&fonts::Font2);
    dst->setTextColor(TFT_GREEN);
    dst->setTextDatum(textdatum_t::middle_center);
    dst->drawString("UnitENVIII", left + w / 2, top + h / 2);

    dst->setTextDatum(td);
    dst->setFont(f);
}
