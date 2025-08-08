/*
 * SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
/*!
  @file adapter_uart.hpp
  @brief Adapters to treat M5HAL and UART in the same way using Searial
  @note  Currently handles Serial directly, but will handle via M5HAL in the future
*/
#ifndef M5_UNIT_COMPONENT_ADAPTER_UART_HPP
#define M5_UNIT_COMPONENT_ADAPTER_UART_HPP

#include "adapter_base.hpp"

class HardwareSerial;

namespace m5 {
namespace unit {

/*!
  @class m5::unit::AdapterUART
  @brief UART access adapter
 */
class AdapterUART : public Adapter {
public:
    class UARTImpl : public Adapter::Impl {
    public:
        UARTImpl()          = default;
        virtual ~UARTImpl() = default;
        inline virtual HardwareSerial* getSerial()
        {
            return nullptr;
        }
        virtual void flush()
        {
        }
        virtual void flushRX()
        {
        }
        virtual void setTimeout(const uint32_t)
        {
        }
    };

    //
#if defined(ARDUINO)
    class SerialImpl : public UARTImpl {
    public:
        explicit SerialImpl(HardwareSerial& serial);
        inline virtual HardwareSerial* getSerial() override
        {
            return _serial;
        }
        virtual void flush() override;
        virtual void flushRX() override;
        virtual void setTimeout(const uint32_t ms) override;
        virtual m5::hal::error::error_t readWithTransaction(uint8_t* data, const size_t len) override;
        virtual m5::hal::error::error_t writeWithTransaction(const uint8_t* data, const size_t len,
                                                             const uint32_t stop) override;

    protected:
        HardwareSerial* _serial{};
    };
#endif

#if defined(ARDUINO)
    AdapterUART(HardwareSerial& serial);
#endif

    inline void flush()
    {
        impl()->flush();
    }
    inline void flushRX()
    {
        impl()->flushRX();
    }
    inline void setTimeout(const uint32_t ms)
    {
        impl()->setTimeout(ms);
    }

    inline UARTImpl* impl()
    {
        return static_cast<UARTImpl*>(_impl.get());
    }
    inline const UARTImpl* impl() const
    {
        return static_cast<UARTImpl*>(_impl.get());
    }

protected:
    AdapterUART() : Adapter(Adapter::Type::UART, new UARTImpl())
    {
    }

protected:
};

}  // namespace unit
}  // namespace m5
#endif
