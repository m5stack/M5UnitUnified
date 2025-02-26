/*
 * SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
/*
  UnitTest for M5UnitComponet
*/
#include <gtest/gtest.h>
#include <M5UnitComponent.hpp>
#include <M5UnitUnified.hpp>
#include "unit_dummy.hpp"
#include <Wire.h>

using namespace m5::unit;

TEST(Component, Update)
{
    UnitUnified units;
    UnitDummy u;
    EXPECT_FALSE(u.isRegistered());

    {
        auto cfg = u.component_config();
        EXPECT_FALSE(cfg.self_update);  // false as default

        EXPECT_EQ(u.count, 0U);
        EXPECT_TRUE(units.add(u, Wire));

        units.update();  // Dont call u.update() because unit was not begun.
        EXPECT_EQ(u.count, 0U);

        EXPECT_TRUE(units.begin());
        units.update();  // Call u.update()
        EXPECT_EQ(u.count, 1U);

        cfg.self_update = true;
        u.component_config(cfg);
        cfg = u.component_config();
        EXPECT_TRUE(cfg.self_update);

        units.update();  // Don't call u.update()
        EXPECT_EQ(u.count, 1U);

        u.update();  // If component_config.self_update is true, you have to call it yourself
        EXPECT_EQ(u.count, 2U);
    }
}
