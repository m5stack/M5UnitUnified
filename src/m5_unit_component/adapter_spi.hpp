/*
 * SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
/*!
  @file adapter_spi.hpp
  @brief Adapters to treat M5HAL and SPI in the same way using Searial
  @note  Currently handles SPI directly, but will handle via M5HAL in the future
*/
#ifndef M5_UNIT_COMPONENT_ADAPTER_SPI_HPP
#define M5_UNIT_COMPONENT_ADAPTER_SPI_HPP

#include "adapter_base.hpp"
#if defined(ARDUINO)
#include <SPI.h>
#endif

namespace m5 {
namespace unit {

/*!
  @class m5::unit::AdapterSPI
  @brief SPI access adapter
 */
class AdapterSPI : public Adapter {
public:
    class SPIImpl : public Adapter::Impl {
    public:
        explicit SPIImpl(const uint8_t cs = 0xFF) : _cs{cs}
        {
        }
        virtual ~SPIImpl() = default;

        inline virtual SPIClass* getSPI()
        {
            return nullptr;
        }
        inline uint8_t cs_pin() const
        {
            return _cs;
        }

        virtual void beginTransaction()
        {
        }
        virtual void endTransaction()
        {
        }

    protected:
        uint8_t _cs{};
    };

#if defined(ARDUINO)
    class SPIClassImpl : public SPIImpl {
    public:
        SPIClassImpl(SPIClass& serial, const SPISettings& settings, const uint8_t cs);
        inline virtual SPIClass* getSPI() override
        {
            return _spi;
        }
        virtual void beginTransaction() override;
        virtual void endTransaction() override;
        virtual m5::hal::error::error_t readWithTransaction(uint8_t* data, const size_t len) override;
        virtual m5::hal::error::error_t writeWithTransaction(const uint8_t* data, const size_t len,
                                                             const uint32_t stop) override;
        virtual m5::hal::error::error_t writeWithTransaction(const uint8_t reg, const uint8_t* data, const size_t len,
                                                             const uint32_t stop) override;
        virtual m5::hal::error::error_t writeWithTransaction(const uint16_t reg, const uint8_t* data, const size_t len,
                                                             const uint32_t stop) override;

    protected:
        SPIClass* _spi{};
        SPISettings _settings{};
        static uint32_t transaction_count;
    };
#endif

#if defined(ARDUINO)
    AdapterSPI(SPIClass& spi, const SPISettings& settings, const uint8_t cs);
#endif

    inline SPIImpl* impl()
    {
        return static_cast<SPIImpl*>(_impl.get());
    }
    inline const SPIImpl* impl() const
    {
        return static_cast<SPIImpl*>(_impl.get());
    }

    inline uint8_t cs_pin() const
    {
        return impl()->cs_pin();
    }

    inline virtual void beginTransaction() override
    {
        impl()->beginTransaction();
    }
    inline virtual void endTransaction() override
    {
        impl()->endTransaction();
    }

protected:
    AdapterSPI() : Adapter(Adapter::Type::SPI, new SPIImpl())
    {
    }

protected:
    uint8_t _cs{};
};

}  // namespace unit
}  // namespace m5
#endif
