/*
 * SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
/*
  UnitTest for M5UnitComponent
*/
#include <gtest/gtest.h>
#include <M5UnitComponent.hpp>
#include <M5Utility.hpp>
#include "unit_dummy.hpp"
#include <type_traits>
#include <utility>

TEST(Component, Children)
{
    m5::unit::UnitDummy u0, u1, u2, u3;

    //
    EXPECT_FALSE(u0.hasParent());
    EXPECT_FALSE(u0.hasSiblings());
    EXPECT_FALSE(u0.hasChildren());

    EXPECT_EQ(0U, u0.childrenSize());
    EXPECT_FALSE(u0.add(u1, 0));

    // add 1
    auto cfg         = u0.component_config();
    cfg.max_children = 1;
    u0.component_config(cfg);
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
    cfg              = u0.component_config();
    cfg.max_children = 2;
    u0.component_config(cfg);

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

    // parent
    EXPECT_EQ(u1.parent(), &u0);
    EXPECT_EQ(u2.parent(), &u0);
    EXPECT_EQ(u0.parent(), nullptr);

    // existsChild / child
    EXPECT_TRUE(u0.existsChild(0));
    EXPECT_TRUE(u0.existsChild(3));
    EXPECT_FALSE(u0.existsChild(1));
    EXPECT_FALSE(u0.existsChild(2));
    EXPECT_EQ(u0.child(0), &u1);
    EXPECT_EQ(u0.child(3), &u2);
    EXPECT_EQ(u0.child(1), nullptr);

    // iteration
    m5::unit::UnitDummy* ptr[] = {&u1, &u2};
    size_t i                   = 0;
    for (auto it = u0.childBegin(); it != u0.childEnd(); ++it) {
        EXPECT_EQ(&(*it), ptr[i++]);
    }
    EXPECT_EQ(i, u0.childrenSize());
}

TEST(Component, DefaultProperties)
{
    m5::unit::UnitDummy u;

    // category defaults to None
    EXPECT_EQ(u.category(), m5::unit::types::category_t::None);

    // not in periodic measurement by default
    EXPECT_FALSE(u.inPeriodic());
    EXPECT_FALSE(u.updated());

    // order is 0 before registration
    EXPECT_EQ(u.order(), 0U);

    // adapter exists but Type::Unknown before assignment
    EXPECT_NE(u.adapter(), nullptr);
    EXPECT_EQ(u.adapter()->type(), m5::unit::Adapter::Type::Unknown);

    // address
    EXPECT_EQ(u.address(), m5::unit::DUMMY_I2C_ADDR);

    // deviceName
    EXPECT_STREQ(u.deviceName(), "UnitDummy");
}

namespace {
using namespace m5::unit;
using namespace m5::utility::mmh3;

// A unit that cannot be moved
class UnitNoMove : public Component {
    M5_UNIT_COMPONENT_HPP_BUILDER_WITHOUT_MOVE(UnitNoMove, 0x00);

public:
    UnitNoMove() : Component(DUMMY_I2C_ADDR)
    {
    }
    UnitNoMove(UnitNoMove&&) = delete;
    UnitNoMove& operator=(UnitNoMove&&) = delete;
};
const char UnitNoMove::name[] = "UnitNoMove";
const types::uid_t UnitNoMove::uid{"UnitNoMove"_mmh3};
const types::attr_t UnitNoMove::attr{types::attribute::AccessI2C};

// A unit that keeps a pointer to itself (as handed to an ISR) and refreshes it when moved
class UnitSelfMove : public Component {
    M5_UNIT_COMPONENT_HPP_BUILDER_WITHOUT_MOVE(UnitSelfMove, 0x00);

public:
    UnitSelfMove() : Component(DUMMY_I2C_ADDR), self{this}
    {
    }
    UnitSelfMove(UnitSelfMove&& o) noexcept : Component(std::move(o)), self{this}
    {
    }
    UnitSelfMove& operator=(UnitSelfMove&& o) noexcept
    {
        Component::operator=(std::move(o));
        self               = this;
        return *this;
    }
    UnitSelfMove* self{};
};
const char UnitSelfMove::name[] = "UnitSelfMove";
const types::uid_t UnitSelfMove::uid{"UnitSelfMove"_mmh3};
const types::attr_t UnitSelfMove::attr{types::attribute::AccessI2C};
}  // namespace

TEST(Component, BuilderWithoutMove)
{
    static_assert(std::is_move_constructible<UnitDummy>::value, "BUILDER keeps the default move");
    static_assert(!std::is_copy_constructible<UnitNoMove>::value, "Copy is deleted");
    static_assert(!std::is_move_constructible<UnitNoMove>::value, "The unit deleted its move");
    static_assert(!std::is_move_assignable<UnitNoMove>::value, "The unit deleted its move");

    UnitNoMove n;
    EXPECT_STREQ(n.deviceName(), "UnitNoMove");
    EXPECT_EQ(n.identifier(), UnitNoMove::uid);
    EXPECT_EQ(n.address(), DUMMY_I2C_ADDR);

    UnitSelfMove a;
    EXPECT_EQ(a.self, &a);
    UnitSelfMove b(std::move(a));
    EXPECT_EQ(b.self, &b);
    EXPECT_EQ(b.address(), DUMMY_I2C_ADDR);
    EXPECT_STREQ(b.deviceName(), "UnitSelfMove");
    UnitSelfMove c;
    c = std::move(b);
    EXPECT_EQ(c.self, &c);
}
