/*
 * SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
/*!
  @file adapter.hpp
  @brief Adapters to treat M5HAL and TwoWire in the same way
  @note  Currently handles TwoWire directly, but will handle via M5HAL in the future
*/
#ifndef M5_UNIT_COMPONENT_ADAPTER_HPP
#define M5_UNIT_COMPONENT_ADAPTER_HPP

#include <cstdint>
#include <cstddef>
#include <memory>
#include <M5HAL.hpp>
#include "pin.hpp"

class TwoWire;

namespace m5 {
namespace unit {
/*!
  @class Adapter
  @brief Adapters to treat M5HAL and TwoWire in the same way
 */
class Adapter {
public:
    ///@name Constructor
    ///@{
    explicit Adapter(const uint8_t addr);
    Adapter(TwoWire& wire, const uint8_t addr);
    Adapter(m5::hal::bus::Bus* bus, const uint8_t addr);
    Adapter(m5::hal::bus::Bus& bus, const uint8_t addr) : Adapter(&bus, addr)
    {
    }
    ///@}
    Adapter(const Adapter&) = delete;

    Adapter(Adapter&&) noexcept = default;

    Adapter& operator=(const Adapter&) = delete;

    Adapter& operator=(Adapter&&) noexcept = default;

    virtual ~Adapter() = default;

    inline uint8_t address() const
    {
        return _impl->address();
    }

    inline uint32_t clock() const
    {
        return _impl->clock();
    }
    inline void setClock(const uint32_t clock)
    {
        return _impl->setClock(clock);
    }

    int16_t scl() const
    {
        return _impl->scl();
    }
    int16_t sda() const
    {
        return _impl->sda();
    }

    //! @brief Dupicate adapter
    Adapter* duplicate(const uint8_t addr);

    //! @brief write to address zero (general call)
    m5::hal::error::error_t generalCall(const uint8_t* data, const size_t len);

    ///@name R/W
    ///@{
    /*! @brief Reading data with transactions */
    inline m5::hal::error::error_t readWithTransaction(uint8_t* data, const size_t len)
    {
        return _impl->readWithTransaction(data, len);
    }
    //! @brief Writeing data with transactions */
    inline m5::hal::error::error_t writeWithTransaction(const uint8_t* data, const size_t len, const bool stop = true)
    {
        return _impl->writeWithTransaction(data, len, stop);
    }
    //! @brief Writeing data with transactions (reg8)*/
    inline m5::hal::error::error_t writeWithTransaction(const uint8_t reg, const uint8_t* data, const size_t len,
                                                        const bool stop = true)
    {
        return _impl->writeWithTransaction(reg, data, len, stop);
    }
    //! @brief Writeing data with transactions (reg16) */
    inline m5::hal::error::error_t writeWithTransaction(const uint16_t reg, const uint8_t* data, const size_t len,
                                                        const bool stop = true)
    {
        return _impl->writeWithTransaction(reg, data, len, stop);
    }
    ///@}

    /// @warning Functionality required for a specific unit
    /// @warning Will be improved when integrated with M5HAL
    /// @name Temporary API
    ///@{
    bool begin()
    {
        return _impl->begin();
    }
    bool end()
    {
        return _impl->end();
    }
    void pinMode(uint8_t pin, uint8_t mode)
    {
        _impl->pinMode(pin, mode);
    }
    void digitalWrite(uint8_t pin, uint8_t val)
    {
        _impl->digitalWrite(pin, val);
    }
    bool pushPin();
    bool popPin();
    ///@}

    ///@cond
    class Impl {
    public:
        Impl() = default;
        explicit Impl(const uint8_t addr) : _addr(addr)
        {
        }
        Impl(const uint8_t addr, const uint32_t clock) : _addr(addr), _clock(clock)
        {
        }

        virtual ~Impl() = default;

        inline uint8_t address() const
        {
            return _addr;
        }
        inline uint32_t clock() const
        {
            return _clock;
        }
        inline void setClock(const uint32_t clock)
        {
            _clock = clock;
        }
        virtual int16_t scl() const
        {
            return -1;
        }
        virtual int16_t sda() const
        {
            return -1;
        }

        virtual bool begin()
        {
            return false;
        }
        virtual bool end()
        {
            return false;
        }
        virtual void pinMode(uint8_t pin, uint8_t mode)
        {
        }
        virtual void digitalWrite(uint8_t pin, uint8_t val)
        {
        }

        virtual Impl* duplicate(const uint8_t addr)
        {
            return new Impl(addr, _clock);
        }

        virtual m5::hal::error::error_t readWithTransaction(uint8_t*, const size_t)
        {
            return m5::hal::error::error_t::UNKNOWN_ERROR;
        }
        virtual m5::hal::error::error_t writeWithTransaction(const uint8_t*, const size_t, const bool)
        {
            return m5::hal::error::error_t::UNKNOWN_ERROR;
        }
        virtual m5::hal::error::error_t writeWithTransaction(const uint8_t, const uint8_t*, const size_t, const bool)
        {
            return m5::hal::error::error_t::UNKNOWN_ERROR;
        }
        virtual m5::hal::error::error_t writeWithTransaction(const uint16_t, const uint8_t*, const size_t, const bool)
        {
            return m5::hal::error::error_t::UNKNOWN_ERROR;
        }

        virtual m5::hal::error::error_t generalCall(const uint8_t*, const size_t)
        {
            return m5::hal::error::error_t::UNKNOWN_ERROR;
        }

    protected:
        uint8_t _addr{};
        uint32_t _clock{100000};
    };
    ///@endcond

protected:
    std::unique_ptr<Impl> _impl{};
    gpio::pin_backup_t _backupSCL{-1}, _backupSDA{-1};

    //    Adapter* _parent{};
};

}  // namespace unit
}  // namespace m5
#endif
