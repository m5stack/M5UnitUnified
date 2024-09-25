/*
 * SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
/*!
  @file sprite.hpp
  @brief Shared sprite
 */
#ifndef SPRITE_HPP
#define SPRITE_HPP

#include <M5GFX.h>

void make_shared_sprites();

// For rotary counter
extern LGFX_Sprite number10_6x8;
extern LGFX_Sprite number10_8x16;
extern LGFX_Sprite number6_6x8;
extern LGFX_Sprite number6_8x16;

#endif
