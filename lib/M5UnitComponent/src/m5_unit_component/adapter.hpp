/*!
  @file adapter.hpp
  @brief Adapters to treat M5HAL and TwoWire in the same way

  @warning Currently able to handle TwoWire, but will eventually be M5HAL only

  SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD

  SPDX-License-Identifier: MIT
*/
#ifndef M5_UNIT_COMPONENT_ADAPTER_HPP
#define M5_UNIT_COMPONENT_ADAPTER_HPP

#include <cstdint>
#include <cstddef>
#include <memory>
#include <m5_hal/error.hpp>

class TwoWire;
namespace m5 {
namespace hal {
namespace bus {
class Bus;
}  // namespace bus
}  // namespace hal
}  // namespace m5

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
    Adapter(m5::hal::bus::Bus& bus, const uint8_t addr) : Adapter(&bus, addr) {
    }
    ///@}
    Adapter(const Adapter&)            = delete;
    Adapter(Adapter&&)                 = default;
    Adapter& operator=(const Adapter&) = delete;
    Adapter& operator=(Adapter&&)      = default;
    ~Adapter()                         = default;

    Adapter* duplicate(const uint8_t addr);

    ///@name R/W
    ///@{
    /*! @brief Reading data with transactions */
    m5::hal::error::error_t readWithTransaction(uint8_t* data,
                                                const size_t len) {
        return _impl->readWithTransaction(data, len);
    }
    //! @brief Writeing data with transactions */
    m5::hal::error::error_t writeWithTransaction(const uint8_t* data,
                                                 const size_t len) {
        return _impl->writeWithTransaction(data, len);
    }
    ///@}

    inline uint8_t address() const {
        return _impl->address();
    }

    ///@cond
    class Impl {
       public:
        Impl() = default;
        explicit Impl(const uint8_t addr) : _addr(addr) {
        }
        virtual ~Impl() = default;

        virtual m5::hal::error::error_t readWithTransaction(uint8_t*,
                                                            const size_t) {
            return m5::hal::error::error_t::UNKNOWN_ERROR;
        }
        virtual m5::hal::error::error_t writeWithTransaction(
            const uint8_t*, const size_t) {
            return m5::hal::error::error_t::UNKNOWN_ERROR;
        }
        virtual Impl* duplicate(const uint8_t addr) {
            return new Impl(addr);
        }
        inline uint8_t address() const {
            return _addr;
        }

       protected:
        const uint8_t _addr{};
    };
    ///@endcond

   protected:
    std::unique_ptr<Impl> _impl{};
    Adapter* _parent{};
};

}  // namespace unit
}  // namespace m5
#endif
