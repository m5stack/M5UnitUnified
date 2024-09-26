/*
 * SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
/*
  UnitTest for M5UnitComponent
*/
#ifndef M5_UNIT_COMPONENT_TEST_UNIT_DUMMY_HPP
#define M5_UNIT_COMPONENT_TEST_UNIT_DUMMY_HPP

#include <M5UnitComponent.hpp>

namespace m5 {
namespace unit {
// DummyComponent for UnitTest
class UnitDummy : public m5::unit::Component {
    M5_UNIT_COMPONENT_HPP_BUILDER(UnitDummy, 0x00);

public:
    UnitDummy() : Component(0x00)
    {
    }
    virtual ~UnitDummy()
    {
    }

    virtual bool begin() override
    {
        return true;
    }
    virtual void update(const bool force = false) override
    {
        ++count;
    }
    uint32_t count{};
};
}  // namespace unit
}  // namespace m5
#endif
