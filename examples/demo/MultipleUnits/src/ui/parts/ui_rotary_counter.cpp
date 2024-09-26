/*
 * SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
/*!
  @file ui_rotary_couter.cpp
  @brief Rotary counter
 */
#include "ui_rotary_counter.hpp"
#include <M5Utility.h>
#include <cassert>

namespace m5 {
namespace ui {

RotaryCounter::Number::Number(LGFX_Sprite* src, const uint8_t base) : _src{src}, _base{base}
{
    assert(_src && "Source must be NOT nullptr");
    _height = _src->height() / (_base + 1);
}

void RotaryCounter::Number::animate(const uint8_t num, const uint32_t dur)
{
    _duration = dur;

    auto n = num % _base;
    const int32_t shgt{_height * _base};

    if (_to != n) {
        //        _y = _fy  = _ty % (_height * _base);
        _fy = _y % shgt;
        _ty = n * _height;
        if (_ty < _fy) {
            _ty += shgt;
        }
        _start_at = m5::utility::millis();
        _to       = n;
        // printf("==> %d >Y ;%d -> %d\n", _to, _fy, _ty);
    }
}

bool RotaryCounter::Number::update(const unsigned long now)
{
    const int32_t shgt{_height * _base};
    if (_start_at) {
        if (now >= _start_at + _duration) {
            _start_at = 0;
            _ty %= shgt;
            _y = _fy = _ty;
        } else {
            float t = (now - _start_at) / (float)_duration;
            _y      = (int16_t)(_fy + (_ty - _fy) * t) % shgt;
            // printf(">>> [%d] %d: (%d - %d) %f\n", _to, _y, _fy, _ty, t);
        }
        return true;
    }
    return false;
}

RotaryCounter::RotaryCounter(LovyanGFX* parent, const size_t maxDigits, LGFX_Sprite* src, const uint8_t base)
    : _parent(parent), _base(base)
{
    _numbers.resize(maxDigits);
    if (src) {
        construct(src);
    }
}

void RotaryCounter::construct(LGFX_Sprite* src)
{
    assert(src != nullptr && "src must be NOT nullptr");
    for (auto& n : _numbers) {
        n = Number(src, _base);
    }
}

bool RotaryCounter::update()
{
    bool updated{};
    if (!_pause) {
        auto now = m5::utility::millis();
        for (auto&& n : _numbers) {
            updated |= n.update(now);
        }
    }
    return updated;
}

void RotaryCounter::animate(const uint32_t val, const unsigned long dur)
{
    uint32_t v{val};
    for (auto it = _numbers.rbegin(); it != _numbers.rend(); ++it) {
        it->animate(v % 10, dur);
        v /= 10;
    }
}

void RotaryCounter::animate(const size_t digit, const uint8_t val, const unsigned long dur)
{
    if (digit >= _numbers.size()) {
        M5_LIB_LOGE("Illegal digit %zu/%zu", digit, _numbers.size());
        return;
    }
    _numbers[digit].animate(val, dur);
}

}  // namespace ui
}  // namespace m5
