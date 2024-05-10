/*!
  UnitTest for M5UnitUnified

  copyright M5Stack. All rights reserved.
  Licensed under the MIT license. See LICENSE file in the project root for full
  license information.
*/
#include <gtest/gtest.h>
#include <M5UnitComponent.hpp>
#include <M5UnitUnified.hpp>
#include <algorithm>

#include <unit/unit_SCD4x.hpp>
#include <unit/unit_SHT3x.hpp>

namespace {

#if 0
// Get the equivalent of a unique type name without RTTI
template <typename U>
const char* TypeName() {
    return __PRETTY_FUNCTION__;
}
#endif

std::vector<m5::unit::Component*> vec;

template <class U>
void each_unit_test() {
    SCOPED_TRACE(U::name);

    U* u = new U();

    // Are the values the same via class and via instance?
    EXPECT_STREQ(U::name, u->deviceName());
    EXPECT_EQ(U::uid, u->identifier());
    EXPECT_EQ(U::attr, u->attribute());

    // Identical IDs exist?
    for (auto&& e : vec) {
        EXPECT_NE(u->identifier(), e->identifier())
            << u->deviceName() << " / " << e->deviceName();
    }

    // Other test......

    
    vec.push_back(u);
}

}  // namespace

TEST(UnitUnified, EachUnit) {
    each_unit_test<m5::unit::UnitSCD40>();
    each_unit_test<m5::unit::UnitSCD41>();
    each_unit_test<m5::unit::UnitSHT3x>();

    for (auto&& e : vec) {
        delete e;
    }
    vec.clear();
}
