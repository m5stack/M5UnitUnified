/*
 * SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
/*!
  @file sprite.cpp
  @brief Shared sprite
 */

#include "sprite.hpp"

struct NumberSprite {
    const lgfx::IFont* font;
    LGFX_Sprite* sprite;
    const int32_t width, height;
    const uint32_t base;
};

// For rotary counter
LGFX_Sprite number10_6x8;
LGFX_Sprite number10_8x16;
LGFX_Sprite number6_6x8;
LGFX_Sprite number6_8x16;

void make_shared_sprites()
{
    NumberSprite table[] = {
        {&fonts::Font0, &number10_6x8, 6, 8, 10},
        {&fonts::Font2, &number10_8x16, 8, 16, 10},
        {&fonts::Font0, &number6_6x8, 6, 8, 6},
        {&fonts::Font2, &number6_8x16, 8, 16, 6},
    };

    for (auto&& e : table) {
        e.sprite->setPsram(false);
        e.sprite->setColorDepth(1);
        e.sprite->createSprite(e.width + 2, e.height * (e.base + 1));
        e.sprite->setPaletteColor(0, TFT_BLACK);
        e.sprite->setPaletteColor(1, TFT_WHITE);
        e.sprite->setTextColor(1, 0);
        e.sprite->setFont(e.font);
        for (int i = 0; i <= e.base; ++i) {
            e.sprite->setCursor(1, i * e.height + 1);
            e.sprite->printf("%d", i % e.base);
        }
        //        e.sprite->drawFastVLine(0, 0, e.sprite->height(), 1);
        //        e.sprite->drawFastVLine(e.sprite->width() - 1, 0, e.sprite->height(), 1);
    }
}
