/*
 * SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
/*!
  @file adapter_base.hpp
  @brief Adapter base
*/
#ifndef M5_UNIT_COMPONENT_ADAPTER_BASE_HPP
#define M5_UNIT_COMPONENT_ADAPTER_BASE_HPP

#include <cstdint>
#include <cstddef>
#include <memory>
#include <M5HAL.hpp>
#include "types.hpp"

namespace m5 {
namespace unit {
/*!
  @class m5::unit::Adapter
  @brief Adapter base class  to treat M5HAL and TwoWire,GPIO,Serial,SPI... in the same way
 */
class Adapter {
public:
    enum class Type : uint8_t {
        Unknown,
        I2C,
        GPIO,
        UART,
        SPI,
    };

    class Impl {
    public:
        Impl() = default;

        virtual ~Impl()
        {
        }

        // I2C R/W
        virtual m5::hal::error::error_t readWithTransaction(uint8_t*, const size_t)
        {
            return m5::hal::error::error_t::UNKNOWN_ERROR;
        }
        virtual m5::hal::error::error_t writeWithTransaction(const uint8_t*, const size_t, const uint32_t)
        {
            return m5::hal::error::error_t::UNKNOWN_ERROR;
        }
        virtual m5::hal::error::error_t writeWithTransaction(const uint8_t, const uint8_t*, const size_t,
                                                             const uint32_t)
        {
            return m5::hal::error::error_t::UNKNOWN_ERROR;
        }
        virtual m5::hal::error::error_t writeWithTransaction(const uint16_t, const uint8_t*, const size_t,
                                                             const uint32_t)
        {
            return m5::hal::error::error_t::UNKNOWN_ERROR;
        }
        virtual m5::hal::error::error_t generalCall(const uint8_t*, const size_t)
        {
            return m5::hal::error::error_t::UNKNOWN_ERROR;
        }
        // GPIO R/W
        virtual m5::hal::error::error_t pinModeRX(const gpio::Mode)
        {
            return m5::hal::error::error_t::UNKNOWN_ERROR;
        }
        virtual m5::hal::error::error_t writeDigitalRX(const bool)
        {
            return m5::hal::error::error_t::UNKNOWN_ERROR;
        }
        virtual m5::hal::error::error_t readDigitalRX(bool&)
        {
            return m5::hal::error::error_t::UNKNOWN_ERROR;
        }
        virtual m5::hal::error::error_t writeAnalogRX(const uint16_t)
        {
            return m5::hal::error::error_t::UNKNOWN_ERROR;
        }
        virtual m5::hal::error::error_t readAnalogRX(uint16_t&)
        {
            return m5::hal::error::error_t::UNKNOWN_ERROR;
        }
        virtual m5::hal::error::error_t pulseInRX(uint32_t&, const int, const uint32_t)
        {
            return m5::hal::error::error_t::UNKNOWN_ERROR;
        }

        virtual m5::hal::error::error_t pinModeTX(const gpio::Mode)
        {
            return m5::hal::error::error_t::UNKNOWN_ERROR;
        }
        virtual m5::hal::error::error_t writeDigitalTX(const bool)
        {
            return m5::hal::error::error_t::UNKNOWN_ERROR;
        }
        virtual m5::hal::error::error_t readDigitalTX(bool&)
        {
            return m5::hal::error::error_t::UNKNOWN_ERROR;
        }
        virtual m5::hal::error::error_t writeAnalogTX(const uint16_t)
        {
            return m5::hal::error::error_t::UNKNOWN_ERROR;
        }
        virtual m5::hal::error::error_t readAnalogTX(uint16_t&)
        {
            return m5::hal::error::error_t::UNKNOWN_ERROR;
        }
        virtual m5::hal::error::error_t pulseInTX(uint32_t&, const int, const uint32_t)
        {
            return m5::hal::error::error_t::UNKNOWN_ERROR;
        }
    };

    explicit Adapter() : _impl{new Impl()}
    {
    }

protected:
    Adapter(const Type t, Impl* impl) : _type{t}, _impl{impl}
    {
    }

public:
    Adapter(const Adapter&) = delete;

    Adapter(Adapter&&) noexcept = default;

    Adapter& operator=(const Adapter&) = delete;

    Adapter& operator=(Adapter&&) noexcept = default;

    virtual ~Adapter() = default;

    inline Type type() const
    {
        return _type;
    }

    virtual Adapter* duplicate(const uint8_t /*addr*/)
    {
        return new Adapter();
    }

    // I2C R/W
    inline m5::hal::error::error_t readWithTransaction(uint8_t* data, const size_t len)
    {
        return _impl->readWithTransaction(data, len);
    }
    inline m5::hal::error::error_t writeWithTransaction(const uint8_t* data, const size_t len,
                                                        const uint32_t exparam = 1)
    {
        return _impl->writeWithTransaction(data, len, exparam);
    }
    inline m5::hal::error::error_t writeWithTransaction(const uint8_t reg, const uint8_t* data, const size_t len,
                                                        const uint32_t exparam = 1)
    {
        return _impl->writeWithTransaction(reg, data, len, exparam);
    }
    inline m5::hal::error::error_t writeWithTransaction(const uint16_t reg, const uint8_t* data, const size_t len,
                                                        const uint32_t exparam = 1)
    {
        return _impl->writeWithTransaction(reg, data, len, exparam);
    }
    inline m5::hal::error::error_t generalCall(const uint8_t* data, const size_t len)
    {
        return _impl->generalCall(data, len);
    }

    // GPIO R/W
    inline m5::hal::error::error_t pinModeRX(const gpio::Mode m)
    {
        return _impl->pinModeRX(m);
    }
    inline m5::hal::error::error_t writeDigitalRX(const bool high)
    {
        return _impl->writeDigitalRX(high);
    }
    inline m5::hal::error::error_t readDigitalRX(bool& high)
    {
        return _impl->readDigitalRX(high);
    }
    inline m5::hal::error::error_t writeAnalogRX(const uint16_t v)
    {
        return _impl->writeAnalogRX(v);
    }
    inline m5::hal::error::error_t readAnalogRX(uint16_t& v)
    {
        return _impl->readAnalogRX(v);
    }
    inline m5::hal::error::error_t pulseInRX(uint32_t& duration, const int state, const uint32_t timeout_us)
    {
        return _impl->pulseInRX(duration, state, timeout_us);
    }

    inline m5::hal::error::error_t pinModeTX(const gpio::Mode m)
    {
        return _impl->pinModeTX(m);
    }
    inline m5::hal::error::error_t writeDigitalTX(const bool high)
    {
        return _impl->writeDigitalTX(high);
    }
    inline m5::hal::error::error_t readDigitalTX(bool& high)
    {
        return _impl->readDigitalTX(high);
    }
    inline m5::hal::error::error_t writeAnalogTX(const uint16_t v)
    {
        return _impl->writeAnalogTX(v);
    }
    inline m5::hal::error::error_t readAnalogTX(uint16_t& v)
    {
        return _impl->readAnalogTX(v);
    }
    inline m5::hal::error::error_t pulseInTX(uint32_t& duration, const int state, const uint32_t timeout_us)
    {
        return _impl->pulseInTX(duration, state, timeout_us);
    }

    // clang-format off

    // WARNING
    // Patches for old units for minor version upgrades (will be removed as soon as no longer needed)
    // For M5Unit-CRYPTO
    [[deprecated("Patches for old units")]]
    uint32_t clock() const;
    [[deprecated("Patches for old units")]]
    void setClock(const uint32_t);
    // For M5Unit-THERMO
    [[deprecated("Patches for old units")]]
    int16_t sda() const;
    [[deprecated("Patches for old units")]]
    int16_t scl() const;
    [[deprecated("Patches for old units")]]
    void pinMode(const uint8_t pin, const uint8_t mode);
    [[deprecated("Patches for old units")]]
    void digitalWrite(const uint8_t pin, const uint8_t high);
    [[deprecated("Patches for old units")]]
    bool begin();
    [[deprecated("Patches for old units")]]
    bool end();

    // clang-format on

private:
    Type _type{Type::Unknown};

protected:
    std::unique_ptr<Impl> _impl{};
};

}  // namespace unit
}  // namespace m5
#endif
