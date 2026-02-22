/*
 * SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
/*!
  @file adapter_i2c.hpp
  @brief Adapter for I2C to treat M5HAL and TwoWire in the same way
*/
#ifndef M5_UNIT_COMPONENT_ADAPTER_I2C_HPP
#define M5_UNIT_COMPONENT_ADAPTER_I2C_HPP

#include "adapter_base.hpp"
#include "pin.hpp"

class TwoWire;

namespace m5 {
namespace unit {

/*!
  @class m5::unit::AdapterI2C
  @brief I2C access adapter
 */
class AdapterI2C : public Adapter {
public:
    class I2CImpl : public Adapter::Impl {
    public:
        I2CImpl() = default;
        I2CImpl(const uint8_t addr, const uint32_t clock) : Adapter::Impl(), _addr(addr), _clock(clock)
        {
        }

        virtual ~I2CImpl() = default;

        inline uint8_t address() const
        {
            return _addr;
        }
        inline void setAddress(const uint8_t addr)
        {
            _addr = addr;
        }

        inline uint32_t clock() const
        {
            return _clock;
        }

        inline virtual void setClock(const uint32_t clock)
        {
            _clock = clock;
        }

        //
        virtual int16_t scl() const
        {
            return -1;
        }
        virtual int16_t sda() const
        {
            return -1;
        }

        //
        virtual bool begin()
        {
            return false;
        }
        virtual bool end()
        {
            return false;
        }
        virtual m5::hal::error::error_t wakeup()
        {
            return m5::hal::error::error_t::UNKNOWN_ERROR;
        }

        //
        virtual I2CImpl* duplicate(const uint8_t addr)
        {
            return new I2CImpl(addr, _clock);
        }

        virtual TwoWire* getWire()
        {
            return nullptr;
        }
        virtual m5::hal::bus::Bus* getBus()
        {
            return nullptr;
        }

    protected:
        uint8_t _addr{};
        uint32_t _clock{100 * 1000U};
    };

    //
#if defined(ARDUINO)
    class WireImpl : public I2CImpl {
    public:
        WireImpl(TwoWire& wire, const uint8_t addr, const uint32_t clock);
        inline virtual TwoWire* getWire() override
        {
            return _wire;
        }
        inline virtual int16_t scl() const override
        {
            return _scl;
        }
        inline virtual int16_t sda() const override
        {
            return _sda;
        }
        virtual bool begin() override;
        virtual bool end() override;
        virtual m5::hal::error::error_t readWithTransaction(uint8_t* data, const size_t len) override;
        virtual m5::hal::error::error_t writeWithTransaction(const uint8_t* data, const size_t len,
                                                             const uint32_t stop) override;
        virtual m5::hal::error::error_t writeWithTransaction(const uint8_t reg, const uint8_t* data, const size_t len,
                                                             const uint32_t stop) override;
        virtual m5::hal::error::error_t writeWithTransaction(const uint16_t reg, const uint8_t* data, const size_t len,
                                                             const uint32_t stop) override;
        virtual I2CImpl* duplicate(const uint8_t addr) override;
        virtual m5::hal::error::error_t generalCall(const uint8_t* data, const size_t len) override;
        virtual m5::hal::error::error_t wakeup() override;

    protected:
        m5::hal::error::error_t write_with_transaction(const uint8_t addr, const uint8_t* data, const size_t len,
                                                       const uint32_t stop);

    private:
        TwoWire* _wire{};
        int16_t _sda{}, _scl{};
    };
#endif

    class BusImpl : public I2CImpl {
    public:
        BusImpl(m5::hal::bus::Bus* bus, const uint8_t addr, const uint32_t clock);
        inline virtual m5::hal::bus::Bus* getBus() override
        {
            return _bus;
        }
        inline virtual int16_t scl() const override
        {
            return _scl;
        }
        inline virtual int16_t sda() const override
        {
            return _sda;
        }

        inline virtual void setClock(const uint32_t clock) override
        {
            I2CImpl::setClock(clock);
            _access_cfg.freq = clock;
        }
        virtual bool begin() override
        {
            return true;
        }
        virtual bool end() override
        {
            return true;
        }
        virtual I2CImpl* duplicate(const uint8_t addr) override;
        virtual m5::hal::error::error_t readWithTransaction(uint8_t* data, const size_t len) override;
        virtual m5::hal::error::error_t writeWithTransaction(const uint8_t* data, const size_t len,
                                                             const uint32_t stop) override;

        virtual m5::hal::error::error_t writeWithTransaction(const uint8_t reg, const uint8_t* data, const size_t len,
                                                             const uint32_t stop) override;
        virtual m5::hal::error::error_t writeWithTransaction(const uint16_t reg, const uint8_t* data, const size_t len,
                                                             const uint32_t stop) override;
        virtual m5::hal::error::error_t generalCall(const uint8_t* data, const size_t len) override;
        virtual m5::hal::error::error_t wakeup() override;

    protected:
        m5::hal::error::error_t write_with_transaction(const m5::hal::bus::I2CMasterAccessConfig& cfg,
                                                       const uint8_t* data, const size_t len, const uint32_t stop);

    private:
        m5::hal::bus::Bus* _bus{};
        m5::hal::bus::I2CMasterAccessConfig _access_cfg{};
        int16_t _sda{-1}, _scl{-1};
    };

#if defined(ARDUINO)
    AdapterI2C(TwoWire& wire, uint8_t addr, const uint32_t clock);
#endif
    AdapterI2C(m5::hal::bus::Bus* bus, const uint8_t addr, const uint32_t clock);
    AdapterI2C(m5::hal::bus::Bus& bus, const uint8_t addr, const uint32_t clock) : AdapterI2C(&bus, addr, clock)
    {
    }

    inline I2CImpl* impl()
    {
        return static_cast<I2CImpl*>(_impl.get());
    }
    inline const I2CImpl* impl() const
    {
        return static_cast<I2CImpl*>(_impl.get());
    }

    inline uint8_t address() const
    {
        return impl()->address();
    }
    inline void setAddress(const uint8_t addr)
    {
        impl()->setAddress(addr);
    }

    inline uint32_t clock() const
    {
        return impl()->clock();
    }

    inline void setClock(const uint32_t clock)
    {
        impl()->setClock(clock);
    }

    inline int16_t scl() const
    {
        return impl()->scl();
    }
    inline int16_t sda() const
    {
        return impl()->sda();
    }

    virtual Adapter* duplicate(const uint8_t addr) override;

    /// @warning Functionality required for a specific unit
    /// @warning Will be improved when integrated with M5HAL
    /// @name Temporary API
    ///@{
    inline bool begin()
    {
        return impl()->begin();
    }
    inline bool end()
    {
        return impl()->end();
    }
    bool pushPin();
    bool popPin();
    ///@}

protected:
    AdapterI2C() : Adapter(Adapter::Type::I2C, new I2CImpl())
    {
    }

protected:
    gpio::pin_backup_t _backupSCL{-1}, _backupSDA{-1};
};

}  // namespace unit
}  // namespace m5
#endif
