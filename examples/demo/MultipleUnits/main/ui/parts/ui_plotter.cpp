/*
 * SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
/*!
  @file ui_plotter.cpp
  @brief Plotter
 */

#include "ui_plotter.hpp"
#include <algorithm>

namespace m5 {
namespace ui {

Plotter::Plotter(LovyanGFX* parent, const size_t maxPlot, const int32_t wid, const int32_t hgt,
                 const int32_t coefficient)
    : _parent(parent), _wid{wid}, _hgt{hgt}, _coefficient(coefficient), _data(maxPlot), _autoScale{true}
{
}

Plotter::Plotter(LovyanGFX* parent, const size_t maxPlot, const int32_t minimum, const int32_t maximum,
                 const int32_t wid, const int32_t hgt, const int32_t coefficient)
    : _parent(parent),
      _min{minimum},
      _max{maximum},
      _wid{wid},
      _hgt{hgt},
      _coefficient(coefficient),
      _data(maxPlot),
      _autoScale{false}
{
}

void Plotter::update()
{
    if (_cb && _autoScale && _cb->size() >= 2) {
        auto it = std::minmax_element(_cb->cbegin(), _cb->cend());
        _min    = *(it.first);
        _max    = *(it.second);
        if (_min == _max) {
            ++_max;
        }
    }
}

void Plotter::push_back(const float val)
{
    push_back((int32_t)(val * _coefficient));
}

void Plotter::push_back(const int32_t val)
{
    auto v = _autoScale ? val : std::min(std::max(val, _min), _max);
    _data.push_back(v);

    if (_autoScale && _data.size() >= 2) {
#if 0
        if (_min == _max) {
            auto it = std::minmax_element(_data.cbegin(), _data.cend());
            _min    = *(it.first);
            _max    = *(it.second);
        } else {
            if (v < _min) {
                _min = v;
            }
            if (v > _max) {
                _max = v;
            }
        }
#else
        auto it = std::minmax_element(_data.cbegin(), _data.cend());
        _min    = *(it.first);
        _max    = *(it.second);
        if (_min == _max) {
            ++_max;
        }
#endif
    }
}

void Plotter::assign(m5::container::CircularBuffer<int32_t>& cb)
{
    _cb = &cb;
    if (_autoScale && _cb->size() >= 2) {
        auto it = std::minmax_element(_cb->cbegin(), _cb->cend());
        _min    = *(it.first);
        _max    = *(it.second);
    }
}

void Plotter::push(LovyanGFX* dst, const int32_t x, const int32_t y)
{
    dst->setClipRect(x, y, width(), height());

    // gauge
    dst->drawFastHLine(x, y, _wid, _gaugeClr);
    dst->drawFastHLine(x, y + (_hgt >> 1), _wid, _gaugeClr);
    dst->drawFastHLine(x, y + (_hgt >> 2), _wid, _gaugeClr);
    dst->drawFastHLine(x, y + (_hgt >> 2) * 3, _wid, _gaugeClr);
    dst->drawFastHLine(x, y + _hgt - 1, _wid, _gaugeClr);

    if (_data.size() >= 2) {
        auto it    = _cb ? _cb->cbegin() : _data.cbegin();
        auto itend = _cb ? --_cb->cend() : --_data.cend();
        auto sz    = _cb ? _cb->size() : _data.size();
        const float range{(float)_max - _min};
        const int32_t hh{_hgt - 1};
        int32_t left{x};

        // plot latest
        if (sz > _wid) {
            auto cnt{sz - _wid};
            while (cnt--) {
                ++it;  // Bidirectional iterator, so only ++/-- is available.
            }
        }
        if (sz < _wid) {
            left += _wid - sz;
        }

        while (it != itend) {
            int32_t s{*it}, e{*(++it)};
            dst->drawLine(left, y + hh - hh * (s - _min) / range, left + 1, y + hh - hh * (e - _min) / range,
                          _needleClr);
            ++left;
        }
    }

    //
    auto f  = dst->getFont();
    auto td = dst->getTextDatum();
    dst->setFont(&fonts::Font0);
    dst->setTextColor(TFT_WHITE);
    dst->setTextDatum(_tdatum);

    int32_t tx{x};  // left
    switch (_tdatum & 0x03) {
        case 1:  // center
            tx = x + (_wid >> 1);
            break;
        case 2:  // right:
            tx = x + _wid;
            break;
        default:
            break;
    }
    auto s = m5::utility::formatString("%d%s", _min / _coefficient, _ustr ? _ustr : "");
    dst->drawString(s.c_str(), tx, y + _hgt - 8);
    if (_min != _max) {
        auto s = m5::utility::formatString("%d%s", _max / _coefficient, _ustr ? _ustr : "");
        dst->drawString(s.c_str(), tx, y);
        if (_max - _min > 1) {
            s = m5::utility::formatString("%d%s", (_min + ((_max - _min) >> 1)) / _coefficient, _ustr ? _ustr : "");
            dst->drawString(s.c_str(), tx, y + _hgt / 2 - 4);
        }
    }

    dst->setTextDatum(td);
    dst->setFont(f);
    dst->clearClipRect();
}

}  // namespace ui
}  // namespace m5
