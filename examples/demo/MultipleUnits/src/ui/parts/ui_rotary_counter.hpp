/*
 * SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
/*!
  @file ui_rotary_couter.hpp
  @brief Rotary counter
 */
#ifndef UI_ROTARY_COUNTER_HPP
#define UI_ROTARY_COUNTER_HPP

#include <M5GFX.h>
#include <vector>
#include "ui_lgfx_extension.hpp"

namespace m5 {
namespace ui {

/*!
  @class RotaryCounter
  @brief Rotary counter with any digits
*/
class RotaryCounter {
public:
    using elapsed_time_t = unsigned long;

    // For each number
    class Number {
    public:
        Number()
        {
        }
        Number(LGFX_Sprite* src, const uint8_t base = 10);
        inline LGFX_Sprite* sprite()
        {
            return _src;
        }
        inline int32_t sourceY() const
        {
            return _y;
        }
        inline uint32_t width() const
        {
            return _src ? _src->width() : 0U;
        }
        inline uint32_t height() const
        {
            return _height;
        }
        inline void set(const uint8_t num)
        {
            animate(num, 0);
        }
        void animate(const uint8_t num, const uint32_t dur);
        bool update(const elapsed_time_t now);

    private:
        LGFX_Sprite* _src{};
        int32_t _height{}, _fy{}, _ty{}, _y{};
        elapsed_time_t _start_at{}, _duration{};
        uint8_t _base{};
        uint8_t _to{};
    };

    using vector_type_t = std::vector<Number>;

    /*!
      @brief Constructor
      @param parent Push target
      @param src Source sprite
      @param digits Number of digits
      @param base How many decimal digits?
      @note The source sprite should consist of the following
      [0...9] [0...5] [0...2]
      +---+   +---+   +---+
      | 0 |   | 0 |   | 0 |
      | 1 |   | 1 |   | 1 |
      | 2 |   | 2 |   | 2 |
      | 3 |   | 3 |   | 0 |
      | 4 |   | 4 |   +---+
      | 5 |   | 5 |
      | 6 |   | 0 |
      | 7 |   +---*
      | 8 |
      | 9 |
      | 0 |
      +---+
     */
    RotaryCounter(LovyanGFX* parent, const size_t digits, LGFX_Sprite* src = nullptr, const uint8_t base = 10);
    virtual ~RotaryCounter()
    {
    }

    //! @brief Construct with source sprite
    void construct(LGFX_Sprite* src);

    //!@brief Update all numbers
    virtual bool update();

    ///@Properties
    ///@{
    const vector_type_t& numbers() const
    {
        return _numbers;
    }
    vector_type_t& numbers()
    {
        return _numbers;
    }
    ///@}

    ///@name Control
    ///@{
    /*!@brief Pause/Resume */
    inline void pause(const bool paused)
    {
        _pause = paused;
    }
    //! @brief Pause
    inline void pause()
    {
        pause(true);
    }
    //! @brief Resume
    inline void resume()
    {
        pause(false);
    }
    //! @brief Animate and change values (all)
    void animate(const uint32_t val, const elapsed_time_t dur);
    //! @brief Set value (all)
    inline void set(const uint32_t val)
    {
        animate(val, 0U);
    }
    //! Animate and change values (partial)
    void animate(const size_t digit, const uint8_t val, const elapsed_time_t dur);
    //! @brief Set value (partial)
    inline void set(const size_t digit, const uint8_t val)
    {
        animate(digit, val, 0U);
    }
    ///@}

    ///@warning If you have already set clip rect to dst, save and set it again on your own.
    ///@warning After this function call, the clip rectangle in dst is cleared.
    ///@name Push
    ///@{
    inline void push(const int32_t x, const int32_t y)
    {
        push(_parent, x, y);
    }
    void push(LovyanGFX* dst, const int32_t x, const int32_t y)
    {
        int32_t left{x};
        for (auto&& n : _numbers) {
            m5::lgfx::pushPartial(dst, left, y, n.width(), n.height(), n.sprite(), 0, n.sourceY());
            left += n.width();
        }
    }
    template <typename T>
    inline void push(const int32_t x, const int32_t y, const T& transp)
    {
        push(_parent, x, y, transp);
    }
    template <typename T>
    void push(LovyanGFX* dst, const int32_t x, const int32_t y, const T& transp)
    {
        int32_t left{x};
        if (!_fit) {
            fit();
            _fit = true;
        }
        for (auto&& n : _numbers) {
            m5::lgfx::pushPartial(dst, left, y, n.width(), n.height(), n.sprite(), 0, n.sourceY(), transp);
            left += n.width();
        }
    }
    ///@}

protected:
    void fit();

    LovyanGFX* _parent{};
    int32_t _height{};
    vector_type_t _numbers{};
    uint8_t _base{};
    bool _fit{}, _pause{};
};

}  // namespace ui
}  // namespace m5
#endif
