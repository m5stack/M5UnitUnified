/*
 * SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
/*!
  @file ui_UnitBase.hpp
  @brief UI for UnitBase
 */
#ifndef UI_UNIT_BASE_HPP
#define UI_UNIT_BASE_HPP

#include <freertos/FreeRTOS.h>
#include <M5GFX.h>
#include <vector>

class UnitUIBase {
public:
    explicit UnitUIBase(LovyanGFX* parent);
    virtual ~UnitUIBase();

    bool lock(portTickType bt = portMAX_DELAY);
    // TickType_t
    void unlock();

    virtual void construct() = 0;
    virtual void update()    = 0;

    virtual void push(LovyanGFX* dst, const int32_t x, const int32_t y) = 0;
    inline void push(const int32_t x, const int32_t y)
    {
        push(_parent, x, y);
    }

protected:
    LovyanGFX* _parent{};
    int32_t _wid{}, _hgt{};

private:
    volatile SemaphoreHandle_t _sem{};
};

#endif
