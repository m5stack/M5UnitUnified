/*
 * SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
/*!
  @file ui_lgfx_extesion.hpp
  @brief M5GFX lgfx extensions
 */
#ifndef UI_LGFX_EXTENSION_HPP
#define UI_LGFX_EXTENSION_HPP

#include <M5GFX.h>

namespace m5 {
namespace lgfx {
/*!
  @brief Push sprite partialty
  @param dst Push target
  @param dx Destination X coordinate for push
  @param dy Destination Y coordinate for push
  @param width Width of partial rectangle
  @param height Height of partial rectangle
  @param src Source sprite
  @param sx Source X coordinate for push
  @param sy Source Y coordinate for push
  @warning If you have already set clip rect to dst, save and set it again on your own.
  @warning After this function call, the clip rectangle in dst is cleared.
*/
inline void pushPartial(LovyanGFX* dst, const int32_t dx, const int32_t dy, const int32_t width, const int32_t height,
                        LGFX_Sprite* src, const int32_t sx, const int32_t sy)
{
    dst->setClipRect(dx, dy, width, height);
    src->pushSprite(dst, dx - sx, dy - sy);
    dst->clearClipRect();
}

/*!
  @copybrief pushPartial(LovyanGFX* dst, const int32_t dx, const int32_t dy, const int32_t width, const int32_t height,
  LGFX_Sprite* src, const int32_t sx, const int32_t sy)
  @copydoc pushPartial(LovyanGFX* dst, const int32_t dx, const int32_t dy, const int32_t width, const int32_t height,
  LGFX_Sprite* src, const int32_t sx, const int32_t sy)
  @param transp Color/Palette for transparent
  @tparam T Color/Palettetype
 */
template <typename T>
void pushPartial(LovyanGFX* dst, const int32_t dx, const int32_t dy, const int32_t width, const int32_t height,
                 LGFX_Sprite* src, const int32_t sx, const int32_t sy, const T& transp)
{
    dst->setClipRect(dx, dy, width, height);
    src->pushSprite(dst, dx - sx, dy - sy, transp);
    dst->clearClipRect();
}

}  // namespace lgfx
}  // namespace m5
#endif
