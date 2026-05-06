/*
 * SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
/*!
  @file adapter_uart.hpp
  @brief Adapters to treat M5HAL and UART in the same way using Serial
  @note  Currently handles Serial directly, but will handle via M5HAL in the future
*/
#ifndef M5_UNIT_COMPONENT_ADAPTER_UART_HPP
#define M5_UNIT_COMPONENT_ADAPTER_UART_HPP

#include "adapter_base.hpp"
#if defined(ESP_PLATFORM)
#include <driver/uart.h>
#endif

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

#if defined(ESP_PLATFORM)
    class ESPIDFImpl : public UARTImpl {
    public:
        struct Config {
            uart_port_t uart_num{UART_NUM_1};
            int baud_rate{115200};
            int rx_pin{-1};
            int tx_pin{-1};
            int buf_size{1024};
            uart_word_length_t data_bits{UART_DATA_8_BITS};
            uart_parity_t parity{UART_PARITY_DISABLE};
            uart_stop_bits_t stop_bits{UART_STOP_BITS_1};
            uart_hw_flowcontrol_t flow_ctrl{UART_HW_FLOWCTRL_DISABLE};
        };
        explicit ESPIDFImpl(const Config& cfg);
        virtual ~ESPIDFImpl();
        virtual void flush() override;
        virtual void flushRX() override;
        virtual void setTimeout(const uint32_t ms) override;
        virtual m5::hal::error::error_t readWithTransaction(uint8_t* data, const size_t len) override;
        virtual m5::hal::error::error_t writeWithTransaction(const uint8_t* data, const size_t len,
                                                             const uint32_t stop) override;
        inline uart_port_t uartPort() const
        {
            return _cfg.uart_num;
        }
        inline bool installed() const
        {
            return _installed;
        }

    protected:
        Config _cfg{};
        bool _installed{false};
        uint32_t _timeout_ms{1000};
    };
#endif

#if defined(ARDUINO)
    explicit AdapterUART(HardwareSerial& serial);
#endif

#if defined(ESP_PLATFORM)
    explicit AdapterUART(const ESPIDFImpl::Config& cfg);
    AdapterUART(uart_port_t uart_num, int baud_rate, int rx_pin, int tx_pin, int buf_size = 1024);

    inline bool espidfInstalled() const
    {
        auto* p = dynamic_cast<ESPIDFImpl*>(_impl.get());
        return p ? p->installed() : false;
    }
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
