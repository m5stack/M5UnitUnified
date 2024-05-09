/*!
  UnitTest for M5UnitComponet

  copyright M5Stack. All rights reserved.
  Licensed under the MIT license. See LICENSE file in the project root for full
  license information.
*/
#include <gtest/gtest.h>
#include <M5UnitComponent.hpp>
#include "unit_dummy.hpp"

TEST(Component, basic) {
    UnitDummy u;
    EXPECT_FALSE(u.isRegistered());

    {
        auto cfg = u.config();
        EXPECT_FALSE(cfg.self_update);

        cfg.self_update = true;
        u.config(cfg);

        cfg = u.config();
        EXPECT_TRUE(cfg.self_update);
    }
}

TEST(Component, Children) {
    UnitDummy u0, u1, u2, u3;

    //
    EXPECT_FALSE(u0.hasParent());
    EXPECT_FALSE(u0.hasSiblings());
    EXPECT_FALSE(u0.hasChildren());

    EXPECT_EQ(0U, u0.childrenSize());
    EXPECT_FALSE(u0.add(u1, 0));

    // add 1
    auto cfg         = u0.config();
    cfg.max_children = 1;
    u0.config(cfg);
    EXPECT_LT(u1.channel(), 0);
    EXPECT_TRUE(u0.add(u1, 0));
    EXPECT_FALSE(u0.add(u1, 1));
    EXPECT_FALSE(u0.add(u2, 1));

    EXPECT_FALSE(u0.hasParent());
    EXPECT_FALSE(u0.hasSiblings());
    EXPECT_TRUE(u0.hasChildren());
    EXPECT_EQ(1U, u0.childrenSize());

    EXPECT_EQ(0, u1.channel());
    EXPECT_TRUE(u1.hasParent());
    EXPECT_FALSE(u1.hasSiblings());
    EXPECT_FALSE(u1.hasChildren());

    // add 2
    cfg              = u0.config();
    cfg.max_children = 2;
    u0.config(cfg);

    EXPECT_LT(u2.channel(), 0);
    EXPECT_FALSE(u0.add(u2, 0));  // same channel (failed)
    EXPECT_LT(u2.channel(), 0);
    EXPECT_TRUE(u0.add(u2, 3));

    EXPECT_FALSE(u0.hasParent());
    EXPECT_FALSE(u0.hasSiblings());
    EXPECT_TRUE(u0.hasChildren());
    EXPECT_EQ(2U, u0.childrenSize());

    EXPECT_TRUE(u1.hasParent());
    EXPECT_TRUE(u1.hasSiblings());
    EXPECT_FALSE(u1.hasChildren());

    EXPECT_TRUE(u2.hasParent());
    EXPECT_TRUE(u2.hasSiblings());
    EXPECT_FALSE(u2.hasChildren());
    EXPECT_EQ(3, u2.channel());

    EXPECT_LT(u3.channel(), 0);
    EXPECT_FALSE(u0.add(u3, 2));  // max = 2 (failed)
    EXPECT_LT(u3.channel(), 0);

    // iteration
    UnitDummy* ptr[] = {&u1, &u2};
    size_t i         = 0;
    for (auto it = u0.childBegin(); it != u0.childEnd(); ++it) {
        EXPECT_EQ(&(*it), ptr[i++]);
        //        printf("[%u]:%s\n", it->port(), (*it).deviceName());
    }
    EXPECT_EQ(i, u0.childrenSize());
}
