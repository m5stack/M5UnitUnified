/*
 * SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
/*!
  @file ui_UnitTVOC.hpp
  @brief UI for UnitTVOC
 */
#ifndef UI_UNIT_TVOC_HPP
#define UI_UNIT_TVOC_HPP

#include "parts/ui_bar_meter.hpp"
#include "parts/ui_plotter.hpp"
#include "ui_UnitBase.hpp"
#include <memory>

class UnitTVOCSmallUI : public UnitUIBase {
public:
    explicit UnitTVOCSmallUI(LovyanGFX* parent) : UnitUIBase(parent)
    {
    }
    void push_back(const int32_t co2, const int32_t tvoc);

    void construct() override;

    void update() override;
    void push(LovyanGFX* dst, const int32_t x, const int32_t y) override;

private:
    std::unique_ptr<m5::ui::Plotter> _co2Plotter{};
    std::unique_ptr<m5::ui::BarMeterV> _co2Bar{};
    std::unique_ptr<m5::ui::Plotter> _tvocPlotter{};
    std::unique_ptr<m5::ui::ColorBarMeterV> _tvocBar{};
    struct Data {
        int32_t co2, tvoc;
    };
    std::vector<Data> _intermediateBuffer{};
};

#endif
