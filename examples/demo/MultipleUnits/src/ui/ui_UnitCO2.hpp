/*
 * SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
/*!
  @file ui_UnitCO2.hpp
  @brief UI for UnitCO2
 */
#ifndef UI_UNIT_CO2_HPP
#define UI_UNIT_CO2_HPP

#include "parts/ui_rotary_counter.hpp"
#include "parts/ui_scale_meter.hpp"
#include "parts/ui_gauge_meter.hpp"
#include "parts/ui_bar_meter.hpp"
#include "ui_UnitBase.hpp"
#include <memory>

class UnitCO2SmallUI : public UnitUIBase {
public:
    explicit UnitCO2SmallUI(LovyanGFX* parent = nullptr) : UnitUIBase(parent)
    {
    }

    void push_back(const int32_t co2, const float temp);

    virtual void construct() override;
    virtual void update() override;
    virtual void push(LovyanGFX* dst, const int32_t x, const int32_t y) override;

private:
    LovyanGFX* _parent{};
    int32_t _wid{}, _hgt{};
    std::unique_ptr<m5::ui::GaugeMeter> _tempMeter{};
    std::unique_ptr<m5::ui::ColorBarMeterH> _co2Meter{};
};
#endif
