/*
 * SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
/*!
  @file ui_UnitVmeter.hpp
  @brief UI for UnitVmeter
 */
#ifndef UI_UNIT_VMETER_HPP
#define UI_UNIT_VMETER_HPP

#include "parts/ui_plotter.hpp"
#include "ui_UnitBase.hpp"
#include <memory>

class UnitVmeterSmallUI : public UnitUIBase {
public:
    explicit UnitVmeterSmallUI(LovyanGFX* parent) : UnitUIBase(parent)
    {
    }

    void push_back(const float mv);

    void construct() override;
    void update() override;
    void push(LovyanGFX* dst, const int32_t x, const int32_t y) override;

private:
    std::unique_ptr<m5::ui::Plotter> _voltagePlotter{};
    std::vector<float> _intermediateBuffer{};
};

#endif
