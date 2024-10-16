/*
 * SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
/*!
  @file ui_UnitHEART.hpp
  @brief UI for UnitHEART
 */
#ifndef UI_UNIT_HEART_HPP
#define UI_UNIT_HEART_HPP

#include "parts/ui_plotter.hpp"
#include "ui_unitBase.hpp"
#include <M5UnitUnifiedHEART.h>
#include <memory>

class UnitHEARTSmallUI : public UnitUIBase {
public:
    explicit UnitHEARTSmallUI(LovyanGFX* parent = nullptr) : UnitUIBase(parent)
    {
    }

    inline m5::heart::PulseMonitor& monitor()
    {
        return _monitor;
    }

    inline void beat(bool beated)
    {
        if (beated) {
            _beatCounter = 4;
        }
    }
    void push_back(const int32_t ir, const int32_t red);

    void construct() override;

    void update() override;
    void push(LovyanGFX* dst, const int32_t x, const int32_t y) override;

private:
    std::unique_ptr<m5::ui::Plotter> _irPlotter{};
    std::unique_ptr<m5::ui::Plotter> _spO2Plotter{};
    m5::heart::PulseMonitor _monitor{100.0f};
    int32_t _beatCounter{}, _bpm{};

    struct Data {
        int32_t ir, red;
    };
    std::vector<Data> _intermediateBuffer{};
};

#endif
