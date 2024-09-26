/*
 * SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
/*!
  @file ui_plotter.hpp
  @brief Plotter
 */
#ifndef UI_PLOTTER_HPP
#define UI_PLOTTER_HPP

#include <M5GFX.h>
#include <M5Utility.h>

namespace m5 {
namespace ui {

class Plotter {
public:
    Plotter(LovyanGFX* parent, const size_t maxPlot, const int32_t wid, const int32_t hgt,
            const int32_t coefficient = 1);
    Plotter(LovyanGFX* parent, const size_t maxPlot, const int32_t minimum, const int32_t maximum, const int32_t wid,
            const int32_t hgt, const int32_t coefficient = 1);

    void update();

    inline int32_t width() const
    {
        return _wid;
    }
    inline int32_t height() const
    {
        return _hgt;
    }
    inline int32_t minimum() const
    {
        return _min;
    }
    inline int32_t maximum() const
    {
        return _max;
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

    inline void setUnitString(const char* s)
    {
        _ustr = s;
    }
    inline void setGaugeTextDatum(const textdatum_t datum)
    {
        _tdatum = datum;
    }

    void push_back(const float val);
    void push_back(const int32_t val);
    void assign(m5::container::CircularBuffer<int32_t>& cb);

    inline void push(const int32_t x, const int32_t y)
    {
        push(_parent, x, y);
    }
    virtual void push(LovyanGFX* dst, const int32_t x, const int32_t y);

protected:
    m5gfx::rgb565_t needleColor() const
    {
        return _needleClr;
    }
    m5gfx::rgb565_t gaugeColor() const
    {
        return _gaugeClr;
    }
    m5gfx::rgb565_t backgroundColor() const
    {
        return _bgClr;
    }

protected:
private:
    LovyanGFX* _parent{};
    int32_t _min{}, _max{}, _wid{}, _hgt{}, _coefficient{};
    m5::container::CircularBuffer<int32_t> _data;
    m5::container::CircularBuffer<int32_t>* _cb{};

    m5gfx::rgb565_t _needleClr{TFT_WHITE}, _gaugeClr{TFT_DARKGRAY}, _bgClr{TFT_BLACK};
    textdatum_t _tdatum{textdatum_t::top_left};
    const char* _ustr{};
    bool _autoScale{};
};

}  // namespace ui
}  // namespace m5
#endif
