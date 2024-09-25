/*
 * SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
/*!
  @file ui_scale_meter.hpp
  @brief Bar meter
 */
#ifndef UI_PARTS_BAR_METER_HPP
#define UI_PARTS_BAR_METER_HPP

#include "ui_base.hpp"
#include <initializer_list>

namespace m5 {
namespace ui {

/*!
  @class BarMeterH
  @brief Horizontal bar meter
 */
class BarMeterH : public Base {
public:
    BarMeterH(LovyanGFX* parent, const int32_t minimum, const int32_t maximum, const int32_t wid, const int32_t hgt)
        : Base(parent, minimum, maximum, wid, hgt)
    {
    }
    virtual ~BarMeterH()
    {
    }

protected:
    virtual void render(LovyanGFX* dst, const int32_t x, const int32_t y, const int32_t val) override;
};

/*!
  @class BarMeterH
  @brief Vertical bar meter
 */
class BarMeterV : public Base {
public:
    BarMeterV(LovyanGFX* parent, const int32_t minimum, const int32_t maximum, const int32_t wid, const int32_t hgt)
        : Base(parent, minimum, maximum, wid, hgt)
    {
    }
    virtual ~BarMeterV()
    {
    }

protected:
    virtual void render(LovyanGFX* dst, const int32_t x, const int32_t y, const int32_t val) override;
};

struct ColorRange {
    int32_t lesseq;
    m5gfx::rgb565_t clr;
};

class ColorBarMeterH : public Base {
public:
    ColorBarMeterH(LovyanGFX* parent, const int32_t minimum, const int32_t maximum, const int32_t wid,
                   const int32_t hgt, std::initializer_list<ColorRange> init = {})
        : Base(parent, minimum, maximum, wid, hgt), _crange(init.begin(), init.end())
    {
    }
    virtual ~ColorBarMeterH()
    {
    }

    void setColorRange(std::initializer_list<ColorRange> init)
    {
        _crange = std::vector<ColorRange>(init);
    }

protected:
    virtual void render(LovyanGFX* dst, const int32_t x, const int32_t y, const int32_t val) override;

private:
    std::vector<ColorRange> _crange{};
};

class ColorBarMeterV : public Base {
public:
    ColorBarMeterV(LovyanGFX* parent, const int32_t minimum, const int32_t maximum, const int32_t wid,
                   const int32_t hgt, std::initializer_list<ColorRange> init = {})
        : Base(parent, minimum, maximum, wid, hgt), _crange(init.begin(), init.end())
    {
    }
    virtual ~ColorBarMeterV()
    {
    }

    void setColorRange(std::initializer_list<ColorRange> init)
    {
        _crange = std::vector<ColorRange>(init);
    }

protected:
    virtual void render(LovyanGFX* dst, const int32_t x, const int32_t y, const int32_t val) override;

private:
    std::vector<ColorRange> _crange{};
};

}  // namespace ui
}  // namespace m5

#endif
