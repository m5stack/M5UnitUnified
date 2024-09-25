/*
 * SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
/*!
  @file ui_UnitENV3.hpp
  @brief UI for UnitENV3
 */
#ifndef UI_UNIT_ENV3_HPP
#define UI_UNIT_ENV3_HPP

#include "parts/ui_scale_meter.hpp"
#include "parts/ui_gauge_meter.hpp"
#include "ui_UnitBase.hpp"
#include <memory>

class UnitENV3SmallUI : public UnitUIBase {
public:
    explicit UnitENV3SmallUI(LovyanGFX* parent) : UnitUIBase(parent)
    {
    }

    void construct() override;

    void sht30_push_back(const float tmp, const float hum);     // SHT30
    void qmp6988_push_back(const float tmp, const float pres);  // QMP6988

    void update() override;
    void push(LovyanGFX* dst, const int32_t x, const int32_t y) override;

private:
    std::unique_ptr<m5::ui::GaugeMeter> _tempMeterSHT{};
    std::unique_ptr<m5::ui::GaugeMeter> _tempMeterQMP{};
    std::unique_ptr<m5::ui::ScaleMeter> _humMeter;
    std::unique_ptr<m5::ui::ScaleMeter> _presMeter;
};

#endif
