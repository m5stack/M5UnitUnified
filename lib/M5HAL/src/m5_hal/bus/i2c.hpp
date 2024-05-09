#ifndef M5_HAL_BUS_I2C_HPP_
#define M5_HAL_BUS_I2C_HPP_

#include "./bus.hpp"

namespace m5 {
namespace hal {
namespace bus {

struct I2CBusConfig : public BusConfig {
    types::bus_type_t getBusType(void) const override { return types::bus_type_t::I2C; };
    interface::gpio::Pin* pin_scl;
    interface::gpio::Pin* pin_sda;
};

struct I2CMasterAccessConfig : public AccessConfig {
    types::bus_type_t getBusType(void) const override { return types::bus_type_t::I2C; };
    uint32_t freq = 100000;
    uint32_t timeout_msec = 1000;
    uint16_t i2c_addr;
    bool address_is_10bit = false;
};

namespace i2c {
    struct I2CMasterAccessor : public Accessor
    {
        I2CMasterAccessor(Bus& bus, const I2CMasterAccessConfig& access_config) : Accessor { bus }, _access_config { access_config } {}
        types::bus_type_t getBusType(void) const override { return types::bus_type_t::I2C; };
        const AccessConfig& getAccessConfig(void) const override { return _access_config; }

        m5::stl::expected<void, m5::hal::error::error_t> startWrite(void) override;
        m5::stl::expected<void, m5::hal::error::error_t> startRead(void) override;
        m5::stl::expected<void, m5::hal::error::error_t> stop(void) override;

        // read with ack
        m5::stl::expected<size_t, m5::hal::error::error_t> read(uint8_t* data, size_t len) override { return readWithLastNackFlag(data, len, false); };

        // virtual m5::stl::expected<size_t, m5::hal::error::error_t> readLastNack(uint8_t* data, size_t len) { return readWithLastNackFlag(data, len, true); }
        // virtual m5::stl::expected<size_t, m5::hal::error::error_t> readWithLastNackFlag(uint8_t* data, size_t len, bool last_nack = false) = 0;
    protected:
        I2CMasterAccessConfig _access_config;
        virtual m5::stl::expected<void, m5::hal::error::error_t> sendStartCondition(void);
        virtual m5::stl::expected<void, m5::hal::error::error_t> sendStopCondition(void);
        virtual m5::stl::expected<void, m5::hal::error::error_t> sendDummyClockWhileSdaLow(size_t count = 9);
        virtual m5::stl::expected<void, m5::hal::error::error_t> waitClockStretch(uint_fast16_t msec);
    };

    class SoftwareI2CMasterAccessor : public I2CMasterAccessor
    {
    public:
        SoftwareI2CMasterAccessor(Bus& bus, const I2CMasterAccessConfig& access_config) : I2CMasterAccessor { bus, access_config } {}
        m5::stl::expected<size_t, m5::hal::error::error_t> write(const uint8_t* data, size_t len) override;
        m5::stl::expected<size_t, m5::hal::error::error_t> readWithLastNackFlag(uint8_t* data, size_t len, bool last_nack = false) override;
    };

//-------------------------------------------------------------------------

    struct I2CBus : public Bus {
        types::bus_type_t getBusType(void) const override { return types::bus_type_t::I2C; };
        const BusConfig& getConfig(void) const override { return _config; }
    protected:
        I2CBusConfig _config;
    };

    class SoftwareI2CBus : public I2CBus
    {
    public:
        error::error_t init(const BusConfig& config) override;
        m5::stl::expected<Accessor*, m5::hal::error::error_t> beginAccess(const AccessConfig& access_config) override;
    };

    m5::stl::expected<I2CBus*, m5::hal::error::error_t> getBus(const I2CBusConfig& config);
} // namespace i2c

// static inline bus::I2CBus* createI2CBus(const bus::I2CBusConfig& config) { return i2c::createBus(config); }

} // namespace bus
} // namespace hal
} // namespace m5

#endif
