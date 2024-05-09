/*!
  @file adapter.cpp
  @brief Adapters to treat M5HAL and TwoWire in the same way

  @warning Currently able to handle TwoWire, but will eventually be M5HAL only

  @copyright M5Stack. All rights reserved.
  @license Licensed under the MIT license. See LICENSE file in the project root
  for full license information.
*/
#include "adapter.hpp"
#include <cassert>
#if defined(ARDUINO)
#include <Wire.h>
#endif
#include <M5HAL.hpp>
#include <M5Utility.hpp>

namespace m5 {
namespace unit {

#if defined(ARDUINO)
// Impl for TwoWire
class WireImpl : public Adapter::Impl {
   public:
    WireImpl(TwoWire& wire, const uint8_t addr)
        : Adapter::Impl(addr), _wire(&wire) {
    }

    virtual m5::hal::error::error_t readWithTransaction(
        uint8_t* data, const size_t len) override {
        if (data && _wire->requestFrom(_addr, len)) {
            auto count = std::min(len, (size_t)_wire->available());
            for (size_t i = 0; i < count; ++i) {
                data[i] = (uint8_t)_wire->read();
            }
            return (count == len) ? m5::hal::error::error_t::OK
                                  : m5::hal::error::error_t::I2C_BUS_ERROR;
        }
        return m5::hal::error::error_t::UNKNOWN_ERROR;
    }

    virtual m5::hal::error::error_t writeWithTransaction(
        const uint8_t* data, const size_t len) override {
        if (data) {
            _wire->beginTransmission(_addr);
            _wire->write(data, len);
            return (_wire->endTransmission() == 0)
                       ? m5::hal::error::error_t::OK
                    : m5::hal::error::error_t::I2C_BUS_ERROR;
        }
        return m5::hal::error::error_t::UNKNOWN_ERROR;
    }

    virtual Impl* duplicate(const uint8_t addr) override {
        return new WireImpl(*_wire, addr);
    }

   private:
    TwoWire* _wire{};
};
#endif

// Impl for M5::HAL
struct BusImpl : public Adapter::Impl {
    BusImpl(m5::hal::bus::Bus* bus, const uint8_t addr)
        : Adapter::Impl(addr), _bus(bus) {
        _access_cfg.i2c_addr = addr;
    }

    virtual m5::hal::error::error_t readWithTransaction(
        uint8_t* data, const size_t len) override {
        if (_bus && data) {
            auto acc = _bus->beginAccess(_access_cfg);
            if (acc) {
                auto trans = acc.value();
                auto result =
                    trans->startRead().and_then([&trans, &data, &len]() {
                        return trans->read(data, len).and_then(
                            [&trans](size_t&&) { return trans->stop(); });
                    });
                // Clean-up must be called
                auto eresult = this->_bus->endAccess(std::move(trans));
                return result.error_or(eresult);
            }
            return acc.error();
        }
        return m5::hal::error::error_t::INVALID_ARGUMENT;
    }

    virtual m5::hal::error::error_t writeWithTransaction(
        const uint8_t* data, const size_t len) override {
        if (_bus && data) {
            auto acc = _bus->beginAccess(_access_cfg);
            if (acc) {
                auto trans = acc.value();
                auto result =
                    trans->startWrite().and_then([&trans, &data, &len]() {
                        return trans->write(data, len).and_then(
                            [&trans](size_t&&) { return trans->stop(); });
                    });
                // Clean-up must be called
                auto eresult = this->_bus->endAccess(std::move(trans));
                return result.error_or(eresult);
            }
            return acc.error();
        }
        return m5::hal::error::error_t::INVALID_ARGUMENT;
    }

    virtual Impl* duplicate(const uint8_t addr) override {
        return new BusImpl(_bus, addr);
    }

   private:
    m5::hal::bus::Bus* _bus{};
    m5::hal::bus::I2CMasterAccessConfig _access_cfg{};
};

// Adapter
Adapter::Adapter(const uint8_t addr) : _impl{new Impl(addr)} {
    assert(_impl);
}

#if defined(ARDUINO)
Adapter::Adapter(TwoWire& wire, const uint8_t addr)
    : _impl{new WireImpl(wire, addr)} {
    assert(_impl);
}
#else
#pragma message "Not support TwoWire on native"
Adapter::Adapter(TwoWire& wire, const uint8_t addr)
    : _impl{new Impl(addr)} {
    assert(_impl);
    M5_LIB_LOGE("Not support TwoWire on native");
}
#endif

Adapter::Adapter(m5::hal::bus::Bus* bus, const uint8_t addr)
    : _impl{new BusImpl(bus, addr)} {
    assert(_impl);
}

Adapter* Adapter::duplicate(const uint8_t addr) {
    auto ptr = new Adapter(addr);
    if (ptr) {
        ptr->_impl.reset(_impl->duplicate(addr));
        if (ptr->_impl) {
            return ptr;
        }
        delete ptr;
    }
    M5_LIB_LOGE("Failed to duplicate");
    return nullptr;
}

}  // namespace unit
}  // namespace m5
