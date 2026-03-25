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

//! Dummy I2C address for testing (any valid non-zero value)
constexpr uint8_t DUMMY_I2C_ADDR{0x01};

// DummyComponent for UnitTest (I2C accessible)
class UnitDummy : public m5::unit::Component {
    M5_UNIT_COMPONENT_HPP_BUILDER(UnitDummy, 0x00);

public:
    UnitDummy() : Component(DUMMY_I2C_ADDR)
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

// DummyComponent for GPIO access
class UnitDummyGPIO : public m5::unit::Component {
    M5_UNIT_COMPONENT_HPP_BUILDER(UnitDummyGPIO, 0x00);

public:
    UnitDummyGPIO() : Component(0x00)
    {
    }
    virtual ~UnitDummyGPIO()
    {
    }
    virtual bool begin() override
    {
        return true;
    }
    virtual void update(const bool force = false) override
    {
    }
};

// DummyComponent for UART access
class UnitDummyUART : public m5::unit::Component {
    M5_UNIT_COMPONENT_HPP_BUILDER(UnitDummyUART, 0x00);

public:
    UnitDummyUART() : Component(0x00)
    {
    }
    virtual ~UnitDummyUART()
    {
    }
    virtual bool begin() override
    {
        return true;
    }
    virtual void update(const bool force = false) override
    {
    }
};

// DummyComponent for SPI access
class UnitDummySPI : public m5::unit::Component {
    M5_UNIT_COMPONENT_HPP_BUILDER(UnitDummySPI, 0x00);

public:
    UnitDummySPI() : Component(0x00)
    {
    }
    virtual ~UnitDummySPI()
    {
    }
    virtual bool begin() override
    {
        return true;
    }
    virtual void update(const bool force = false) override
    {
    }
};

// DummyComponent with I2C + SPI access
class UnitDummyI2CSPI : public m5::unit::Component {
    M5_UNIT_COMPONENT_HPP_BUILDER(UnitDummyI2CSPI, 0x00);

public:
    UnitDummyI2CSPI() : Component(DUMMY_I2C_ADDR)
    {
    }
    virtual ~UnitDummyI2CSPI()
    {
    }
    virtual bool begin() override
    {
        return true;
    }
    virtual void update(const bool force = false) override
    {
    }
};

// DummyComponent with all access (I2C + GPIO + UART + SPI)
class UnitDummyAll : public m5::unit::Component {
    M5_UNIT_COMPONENT_HPP_BUILDER(UnitDummyAll, 0x00);

public:
    UnitDummyAll() : Component(DUMMY_I2C_ADDR)
    {
    }
    virtual ~UnitDummyAll()
    {
    }
    virtual bool begin() override
    {
        return true;
    }
    virtual void update(const bool force = false) override
    {
    }
};

// DummyComponent with no access (attr=0)
class UnitDummyNone : public m5::unit::Component {
    M5_UNIT_COMPONENT_HPP_BUILDER(UnitDummyNone, 0x00);

public:
    UnitDummyNone() : Component(DUMMY_I2C_ADDR)
    {
    }
    virtual ~UnitDummyNone()
    {
    }
    virtual bool begin() override
    {
        return true;
    }
    virtual void update(const bool force = false) override
    {
    }
};

}  // namespace unit
}  // namespace m5
#endif
