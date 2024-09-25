/*
 * SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
/*!
  @file ui_base.cpp
  @brief Base class for UI
 */
#include "ui_base.hpp"
#include <M5Utility.h>

namespace m5 {
namespace ui {

void Base::animate(const int32_t val, const elapsed_time_t dur)
{
    if (_to != val && _min != _max) {
        _from     = _value;
        _to       = std::min(std::max(val, _min), _max);
        _start_at = m5::utility::millis();
        _duration = dur;
    }
}

bool Base::update()
{
    if (_start_at) {
        auto now = m5::utility::millis();
        if (now >= _start_at + _duration) {
            _start_at = 0;
            _value    = _to;
        } else {
            float t = (now - _start_at) / (float)_duration;
            _value  = _from + (_to - _from) * t;
        }
        return true;
    }
    return false;
}

}  // namespace ui
}  // namespace m5
