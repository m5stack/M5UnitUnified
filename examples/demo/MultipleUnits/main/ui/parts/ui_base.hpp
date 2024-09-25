/*
 * SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
/*!
  @file ui_base.hpp
  @brief Base class for UI
 */
#ifndef UI_BASE_HPP
#define UI_BASE_HPP

#include <M5GFX.h>
#include <algorithm>
#include <cmath>

namespace m5 {
namespace ui {

class Base {
public:
    using elapsed_time_t = unsigned long;

    Base(LovyanGFX* parent, const int32_t minimum, const int32_t maximum, const int32_t wid, const int32_t hgt)
        : _parent(parent),
          _min{minimum},
          _max{maximum},
          _value{minimum},
          _from{minimum},
          _to{minimum},
          _wid{wid},
          _hgt{hgt}
    {
    }
    virtual ~Base()
    {
    }

    inline int32_t value() const
    {
        return _value;
    }
    inline int32_t valueTo() const
    {
        return _to;
    }
    inline int32_t width() const
    {
        return _wid;
    }
    inline int32_t height() const
    {
        return _hgt;
    }
    inline int32_t range() const
    {
        return _max - _min;
    }

    inline m5gfx::rgb565_t needleColor() const
    {
        return _needleClr;
    }
    inline m5gfx::rgb565_t gaugeColor() const
    {
        return _gaugeClr;
    }
    inline m5gfx::rgb565_t backgroundColor() const
    {
        return _bgClr;
    }
    template <typename T>
    void setNeedleColor(const T& clr)
    {
        _needleClr = clr;
    }
    template <typename T>
    void setGaugeColor(const T& clr)
    {
        _gaugeClr = clr;
    }
    template <typename T>
    void setBackgroundColor(const T& clr)
    {
        _bgClr = clr;
    }

    virtual bool update();

    ///@name Control
    ///@{
    virtual void animate(const int32_t val, const elapsed_time_t dur);
    inline void set(const int32_t val)
    {
        animate(val, 0U);
    }
    ///@}

    ///@name Push
    ///@{
    inline void push(const int32_t x, const int32_t y)
    {
        push(_parent, x, y);
    }
    virtual void push(LovyanGFX* dst, const int32_t x, const int32_t y)
    {
        render(dst, x, y, _value);
    }
    ///@}

protected:
    inline float ratio(const int32_t val)
    {
        return range() > 0 ? (std::min(std::max(val, _min), _max) - _min) / (float)range() : 0.0f;
    }
    virtual void render(LovyanGFX* dst, const int32_t x, const int32_t y, const int32_t val)
    {
    }

private:
    LovyanGFX* _parent{};
    int32_t _min{}, _max{}, _value{}, _from{}, _to{}, _wid{}, _hgt{};
    elapsed_time_t _start_at{}, _duration{};
    m5gfx::rgb565_t _needleClr{TFT_WHITE}, _gaugeClr{TFT_DARKGRAY}, _bgClr{TFT_BLACK};
};

}  // namespace ui
}  // namespace m5
#endif
