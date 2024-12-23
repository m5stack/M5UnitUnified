/*
 * SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
/*!
  @file adapter.cpp
  @brief Adapters to treat M5HAL and TwoWire in the same way
  @note  Currently handles TwoWire directly, but will handle via M5HAL in the
  future
*/
#include "adapter.hpp"
#include <cassert>
#if defined(ARDUINO)
#include <Wire.h>
#endif
#include <M5HAL.hpp>
#include <M5Utility.hpp>
#include <soc/gpio_struct.h>
#include <soc/gpio_sig_map.h>

namespace {
#if defined(ARDUINO)

int16_t search_pin_number(const int peripheral_sig)
{
#if defined(CONFIG_IDF_TARGET_ESP32C6)
    int16_t no =
        (peripheral_sig < m5::stl::size(GPIO.func_in_sel_cfg)) ? GPIO.func_in_sel_cfg[peripheral_sig].in_sel : -1;
    return (no < GPIO_NUM_MAX) ? no : -1;
#else
    int16_t no =
        (peripheral_sig < m5::stl::size(GPIO.func_in_sel_cfg)) ? GPIO.func_in_sel_cfg[peripheral_sig].func_sel : -1;
    return (no < GPIO_NUM_MAX) ? no : -1;
#endif
}

uint8_t idx_table[][2] = {
    {I2CEXT0_SDA_IN_IDX, I2CEXT0_SCL_IN_IDX},  // Wire
#if !defined(CONFIG_IDF_TARGET_ESP32C6)
    {I2CEXT1_SDA_IN_IDX, I2CEXT1_SCL_IN_IDX},  // Wire1
#else
    {I2CEXT0_SDA_IN_IDX, I2CEXT0_SCL_IN_IDX},  // Wire
#endif
};

#endif
}  // namespace

namespace m5 {
namespace unit {

#if defined(ARDUINO)
// Impl for TwoWire
class WireImpl : public Adapter::Impl {
public:
    WireImpl(TwoWire& wire, const uint8_t addr) : Adapter::Impl(addr), _wire(&wire)
    {
        uint32_t w = (&wire == &Wire1);
        _sda       = search_pin_number(idx_table[w][0]);
        _scl       = search_pin_number(idx_table[w][1]);
        M5_LIB_LOGI("I2C SDA: %d, SCL: %d", _sda, _scl);
    }

    virtual int16_t scl() const override
    {
        return _scl;
    }
    virtual int16_t sda() const override
    {
        return _sda;
    }

    virtual bool begin()
    {
        return _wire->begin();
    }
    virtual bool end()
    {
#if defined(WIRE_HAS_END)
        return _wire->end();
#else
        return false;
#endif
    }

    void pinMode(uint8_t pin, uint8_t mode)
    {
        ::pinMode(pin, mode);
    }
    void digitalWrite(uint8_t pin, uint8_t val)
    {
        ::digitalWrite(pin, val);
    }

    virtual m5::hal::error::error_t readWithTransaction(uint8_t* data, const size_t len) override
    {
        assert(_addr);
        //_wire->setClock(_clock); DON'T CALL

        if (data && _wire->requestFrom(_addr, len)) {
            auto count = std::min(len, (size_t)_wire->available());
            for (size_t i = 0; i < count; ++i) {
                data[i] = (uint8_t)_wire->read();
            }
            return (count == len) ? m5::hal::error::error_t::OK : m5::hal::error::error_t::I2C_BUS_ERROR;
        }
        return m5::hal::error::error_t::UNKNOWN_ERROR;
    }

    inline virtual m5::hal::error::error_t writeWithTransaction(const uint8_t* data, const size_t len,
                                                                const bool stop) override
    {
        return write_with_transaction(_addr, data, len, stop);
    }

    virtual m5::hal::error::error_t writeWithTransaction(const uint8_t reg, const uint8_t* data, const size_t len,
                                                         const bool stop) override
    {
        assert(_addr);
        _wire->setClock(_clock);

        _wire->beginTransmission(_addr);
        _wire->write(reg);
        if (data && len) {
            _wire->write(data, len);
        }
        auto ret = _wire->endTransmission(stop);
        if (ret) {
            M5_LIB_LOGE("%d endTransmission stop:%d", ret, stop);
        }
        return (ret == 0) ? m5::hal::error::error_t::OK : m5::hal::error::error_t::I2C_BUS_ERROR;
    }

    virtual m5::hal::error::error_t writeWithTransaction(const uint16_t reg, const uint8_t* data, const size_t len,
                                                         const bool stop) override
    {
        assert(_addr);
        _wire->setClock(_clock);

        types::big_uint16_t r(reg);
        _wire->beginTransmission(_addr);
        _wire->write(r.data(), r.size());
        if (data && len) {
            _wire->write(data, len);
        }
        auto ret = _wire->endTransmission(stop);
        if (ret) {
            M5_LIB_LOGE("%d endTransmission stop:%d", ret, stop);
        }
        return (ret == 0) ? m5::hal::error::error_t::OK : m5::hal::error::error_t::I2C_BUS_ERROR;
    }

    virtual Impl* duplicate(const uint8_t addr) override
    {
        return new WireImpl(*_wire, addr);
    }

    inline virtual m5::hal::error::error_t generalCall(const uint8_t* data, const size_t len) override
    {
        return write_with_transaction(0x00, data, len, true);
    }

    virtual m5::hal::error::error_t wakeup()
    {
        return write_with_transaction(_addr, nullptr, 0, true);
    }

protected:
    m5::hal::error::error_t write_with_transaction(const uint8_t addr, const uint8_t* data, const size_t len,
                                                   const bool stop)
    {
        _wire->setClock(_clock);  //
        _wire->beginTransmission(addr);
        if (data) {
            _wire->write(data, len);
        }
        auto ret = _wire->endTransmission(stop);
        if (ret) {
            M5_LIB_LOGE("%d endTransmission stop:%d", ret, stop);
        }
        return (ret == 0) ? m5::hal::error::error_t::OK : m5::hal::error::error_t::I2C_BUS_ERROR;
    }

private:
    TwoWire* _wire{};
    int16_t _sda{}, _scl{};
};
#endif

// Impl for M5::HAL
struct BusImpl : public Adapter::Impl {
    BusImpl(m5::hal::bus::Bus* bus, const uint8_t addr) : Adapter::Impl(addr), _bus(bus)
    {
        _access_cfg.i2c_addr = addr;
    }

    virtual Impl* duplicate(const uint8_t addr) override
    {
        return new BusImpl(_bus, addr);
    }

    virtual m5::hal::error::error_t readWithTransaction(uint8_t* data, const size_t len) override
    {
        if (_bus && data) {
            auto acc = _bus->beginAccess(_access_cfg);
            if (acc) {
                auto trans  = acc.value();
                auto result = trans->startRead().and_then([&trans, &data, &len]() {
                    return trans->read(data, len).and_then([&trans](size_t&&) { return trans->stop(); });
                });
                // Clean-up must be called
                auto eresult = this->_bus->endAccess(std::move(trans));
                return result.error_or(eresult);
            }
            return acc.error();
        }
        return m5::hal::error::error_t::INVALID_ARGUMENT;
    }

    virtual m5::hal::error::error_t writeWithTransaction(const uint8_t* data, const size_t len,
                                                         const bool stop) override
    {
        if (_bus) {
            auto acc = _bus->beginAccess(_access_cfg);
            if (acc) {
                auto trans  = acc.value();
                auto result = trans->startWrite().and_then([&trans, &data, &len, &stop]() {
                    return trans->write(data, len).and_then([&trans, &stop](size_t&&) {
                        return stop ? trans->stop() : m5::stl::expected<void, m5::hal::error::error_t>();
                    });
                });
                // Clean-up must be called
                auto eresult = this->_bus->endAccess(std::move(trans));
                return result.error_or(eresult);
            }
            return acc.error();
        }
        return m5::hal::error::error_t::INVALID_ARGUMENT;
    }

    virtual m5::hal::error::error_t writeWithTransaction(const uint8_t reg, const uint8_t* data, const size_t len,
                                                         const bool stop) override
    {
        assert(_addr);

        if (_bus) {
            auto acc = _bus->beginAccess(_access_cfg);
            if (acc) {
                auto trans  = acc.value();
                auto result = trans->startWrite().and_then([&trans, &reg, &data, &len, &stop]() {
                    return trans->write(&reg, 1).and_then([&trans, &data, &len, &stop](size_t&&) {
                        return ((data && len) ? trans->write(data, len)
                                              : m5::stl::expected<size_t, m5::hal::error::error_t>((size_t)0UL))
                            .and_then([&trans, &stop](size_t&&) {
                                return stop ? trans->stop() : m5::stl::expected<void, m5::hal::error::error_t>();
                            });
                    });
                });
                // Clean-up must be called
                auto eresult = this->_bus->endAccess(std::move(trans));
                return result.error_or(eresult);
            }
            return acc.error();
        }
        return m5::hal::error::error_t::INVALID_ARGUMENT;
    }

    virtual m5::hal::error::error_t writeWithTransaction(const uint16_t reg, const uint8_t* data, const size_t len,
                                                         const bool stop) override
    {
        assert(_addr);

        if (_bus) {
            auto acc = _bus->beginAccess(_access_cfg);
            if (acc) {
                types::big_uint16_t r(reg);
                auto trans  = acc.value();
                auto result = trans->startWrite().and_then([&trans, &r, &data, &len, &stop]() {
                    return trans->write(r.data(), r.size()).and_then([&trans, &data, &len, &stop](size_t&&) {
                        return ((data && len) ? trans->write(data, len)
                                              : m5::stl::expected<size_t, m5::hal::error::error_t>((size_t)0UL))
                            .and_then([&trans, &stop](size_t&&) {
                                return stop ? trans->stop() : m5::stl::expected<void, m5::hal::error::error_t>();
                            });
                    });
                });

                // Clean-up must be called
                auto eresult = this->_bus->endAccess(std::move(trans));
                return result.error_or(eresult);
            }
            return acc.error();
        }
        return m5::hal::error::error_t::INVALID_ARGUMENT;
    }

    virtual m5::hal::error::error_t generalCall(const uint8_t* data, const size_t len) override
    {
        m5::hal::bus::I2CMasterAccessConfig gcfg = _access_cfg;
        gcfg.i2c_addr                            = 0x00;
        return write_with_transaction(gcfg, data, len, true);
    }

    virtual m5::hal::error::error_t wakeup()
    {
        return write_with_transaction(_access_cfg, nullptr, 0, true);
    }

protected:
    m5::hal::error::error_t write_with_transaction(const m5::hal::bus::I2CMasterAccessConfig& cfg, const uint8_t* data,
                                                   const size_t len, const bool stop)
    {
        if (_bus) {
            auto acc = _bus->beginAccess(cfg);
            if (acc) {
                auto trans = acc.value();
                auto result =
                    trans->startWrite()
                        .and_then([&trans, &data, &len]() {
                            return ((data && len) ? trans->write(data, len)
                                                  : m5::stl::expected<size_t, m5::hal::error::error_t>((size_t)0UL));
                        })
                        .and_then([&trans, &stop](size_t&&) {
                            return stop ? trans->stop() : m5::stl::expected<void, m5::hal::error::error_t>();
                        });
                // Clean-up must be called
                auto eresult = this->_bus->endAccess(std::move(trans));
                return result.error_or(eresult);
            }
            return acc.error();
        }
        return m5::hal::error::error_t::INVALID_ARGUMENT;
    }

private:
    m5::hal::bus::Bus* _bus{};
    m5::hal::bus::I2CMasterAccessConfig _access_cfg{};
};

// Adapter
Adapter::Adapter(const uint8_t addr) : _impl{new Impl(addr)}
{
    assert(_impl);
}

#if defined(ARDUINO)
Adapter::Adapter(TwoWire& wire, const uint8_t addr) : _impl{new WireImpl(wire, addr)}
{
    assert(_impl);
}
#else
#pragma message "Not support TwoWire"
Adapter::Adapter(TwoWire& wire, const uint8_t addr) : _impl{new Impl(addr)}
{
    assert(_impl);
    M5_LIB_LOGE("Not support TwoWire");
}
#endif

Adapter::Adapter(m5::hal::bus::Bus* bus, const uint8_t addr) : _impl{new BusImpl(bus, addr)}
{
    assert(_impl);
}

Adapter* Adapter::duplicate(const uint8_t addr)
{
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

m5::hal::error::error_t Adapter::generalCall(const uint8_t* data, const size_t len)
{
    return _impl->generalCall(data, len);
}

bool Adapter::pushPin()
{
#if defined(ARDUINO)
    if (_backupSCL.getPin() < 0 && _backupSDA.getPin() < 0) {
        _backupSCL.setPin(scl());
        _backupSCL.backup();
        _backupSDA.setPin(sda());
        _backupSDA.backup();
        M5_LIB_LOGE(">>Push SCL:%u SDA:%u", _backupSCL.getPin(), _backupSDA.getPin());
        return true;
    }
    return false;

#else
    return false;
#endif
}

bool Adapter::popPin()
{
#if defined(ARDUINO)
    if (_backupSCL.getPin() >= 0 && _backupSDA.getPin() >= 0) {
        M5_LIB_LOGE("<<Pop SCL:%u SDA:%u", _backupSCL.getPin(), _backupSDA.getPin());

        _backupSCL.restore();
        _backupSDA.restore();
        _backupSCL.setPin(-1);
        _backupSDA.setPin(-1);
        return true;
    }
    return false;
#else
    return false;
#endif
}

}  // namespace unit
}  // namespace m5

#if 0
//CoreS3SE
18:02:03.752 > [  3554][W][pin.cpp:65] restore(): restore pin:1 
18:02:03.758 > [  3554][W][pin.cpp:67] restore(): restore IO_MUX_GPIO0_REG          :00001a00 -> 00001b00 
18:02:03.766 > [  3560][W][pin.cpp:69] restore(): restore GPIO_PIN0_REG             :00000000 -> 00000004 
18:02:03.774 > [  3568][W][pin.cpp:71] restore(): restore GPIO_FUNC0_OUT_SEL_CFG_REG:00000100 -> 00000059 
18:02:03.780 > [  3576][W][pin.cpp:85] restore(): restore GPIO_ENABLE_REG:00001818
18:02:03.784 > [  3582][W][pin.cpp:65] restore(): restore pin:2 
18:02:03.792 > [  3586][W][pin.cpp:67] restore(): restore IO_MUX_GPIO0_REG          :00001a00 -> 00001b00 
18:02:03.800 > [  3594][W][pin.cpp:69] restore(): restore GPIO_PIN0_REG             :00000000 -> 00000004 
18:02:03.809 > [  3602][W][pin.cpp:71] restore(): restore GPIO_FUNC0_OUT_SEL_CFG_REG:00000100 -> 0000005a 
18:02:03.815 > [  3611][W][pin.cpp:85] restore(): restore GPIO_ENABLE_REG:0000181a
18:02:03.822 > [  3616][W][unit_MLX90614.cpp:311] start_periodic_measurement(): IIR:4 FIR:7 IT:140
//NanoC6
18:03:55.995 > [  2432][W][pin.cpp:65] restore(): restore pin:1 
18:03:56.001 > [  2432][W][pin.cpp:66] restore(): restore IO_MUX_GPIO0_REG          :00001a02 -> 00001b02 
18:03:56.009 > [  2439][W][pin.cpp:68] restore(): restore GPIO_PIN0_REG             :00000000 -> 00000004 
18:03:56.017 > [  2447][W][pin.cpp:70] restore(): restore GPIO_FUNC0_OUT_SEL_CFG_REG:00000080 -> 0000002d 
18:03:56.023 > [  2455][W][pin.cpp:85] restore(): restore GPIO_ENABLE_REG:00000000
18:03:56.027 > [  2461][W][pin.cpp:65] restore(): restore pin:2 
18:03:56.035 > [  2465][W][pin.cpp:66] restore(): restore IO_MUX_GPIO0_REG          :00001a02 -> 00001b02 
18:03:56.044 > [  2473][W][pin.cpp:68] restore(): restore GPIO_PIN0_REG             :00000000 -> 00000004 
18:03:56.051 > [  2481][W][pin.cpp:70] restore(): restore GPIO_FUNC0_OUT_SEL_CFG_REG:00000080 -> 0000002e 
18:03:56.058 > [  2489][W][pin.cpp:85] restore(): restore GPIO_ENABLE_REG:00000002
//Core
18:24:54.954 > [  2690][W][pin.cpp:64] restore(): restore pin:22 
18:24:54.959 > [  2690][W][pin.cpp:66] restore(): restore IO_MUX_GPIO0_REG          :00002a08 -> 00002b08 
18:24:54.970 > [  2697][W][pin.cpp:68] restore(): restore GPIO_PIN0_REG             :00000000 -> 00000004 
18:24:54.975 > [  2705][W][pin.cpp:70] restore(): restore GPIO_FUNC0_OUT_SEL_CFG_REG:00000100 -> 0000001d 
18:24:54.987 > [  2713][W][pin.cpp:84] restore(): restore GPIO_ENABLE_REG:0a84cf90
18:24:54.992 > [  2719][W][pin.cpp:64] restore(): restore pin:21 
18:24:54.998 > [  2723][W][pin.cpp:66] restore(): restore IO_MUX_GPIO0_REG          :00002a08 -> 00002b08 
18:24:55.003 > [  2731][W][pin.cpp:68] restore(): restore GPIO_PIN0_REG             :00000000 -> 00000004 
18:24:55.009 > [  2739][W][pin.cpp:70] restore(): restore GPIO_FUNC0_OUT_SEL_CFG_REG:00000100 -> 0000001e 
18:24:55.020 > [  2747][W][pin.cpp:84] restore(): restore GPIO_ENABLE_REG:0ac4cf90
#endif
