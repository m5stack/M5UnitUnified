#ifndef M5_HAL_BUS_SPI_HPP_
#define M5_HAL_BUS_SPI_HPP_

#include "./bus.hpp"

namespace m5 {
namespace hal {
namespace bus {
    
/// @brief SPIバスの設定
/// @details 標準的なSPIだけでなく、QSPI,OSPIの設定も可能となっている
struct SPIBusConfig : public BusConfig {
    virtual ~SPIBusConfig() = default;
    types::bus_type_t getBusType(void) const override { return types::bus_type_t::SPI; };
    union {
        interface::gpio::Pin* pins[9];
        struct {
            interface::gpio::Pin* pin_clk;
            interface::gpio::Pin* pin_dc;
            interface::gpio::Pin* pin_mosi; // data0
            interface::gpio::Pin* pin_miso; // data1
            interface::gpio::Pin* pin_d2;   // data2 
            interface::gpio::Pin* pin_d3;   // data3
            interface::gpio::Pin* pin_d4;   // data4
            interface::gpio::Pin* pin_d5;   // data5
            interface::gpio::Pin* pin_d6;   // data6
            interface::gpio::Pin* pin_d7;   // data7
        };
    };
};

enum class SpiDataMode {
spi_halfduplex, // 半二重通信
spi_fullduplex,
spi_halfduplex_with_dc_pin, // 半二重通信+DCピン
spi_fullduplex_with_dc_pin, // 全二重通信+DCピン
spi_halfduplex_with_dc_bit, // 半二重通信+DCビット(9ビットSPI)
spi_fullduplex_with_dc_bit, // 全二重通信+DCビット(9ビットSPI)
spi_dual_output,
spi_dual_io,
spi_quad_output,
spi_quad_io,
spi_octal_output,
spi_octal_io,
};
typedef SpiDataMode spi_data_mode_t;

struct SPIMasterAccessConfig : public AccessConfig {
    virtual ~SPIMasterAccessConfig() = default;
    types::bus_type_t getBusType(void) const override { return types::bus_type_t::SPI; };
    interface::gpio::Pin* pin_cs;
    uint32_t freq;
    spi_data_mode_t spi_data_mode;
    struct {
        uint8_t spi_mode:2;
        uint8_t spi_order:1;
    };
    uint8_t spi_command_length = 0; // コマンド部のビット長
    uint8_t spi_address_length = 0; // アドレス部のビット長
    uint8_t spi_dummy_cycle = 0;    // アドレスからデータへ移る際のダミーサイクル数
};

namespace spi {
    struct SPIMasterAccessor : public Accessor
    {
        SPIMasterAccessor(Bus& bus, const SPIMasterAccessConfig& access_config) : Accessor { bus }, _access_config { access_config } {}
        types::bus_type_t getBusType(void) const override { return types::bus_type_t::SPI; };
        const AccessConfig& getAccessConfig(void) const override { return _access_config; }

        virtual m5::stl::expected<size_t, m5::hal::error::error_t> writeCommand(const uint8_t* data, size_t len);
        virtual m5::stl::expected<size_t, m5::hal::error::error_t> writeCommandData(const uint8_t* data, size_t len);
        virtual m5::stl::expected<void, m5::hal::error::error_t> sendDummyClock(size_t count);
        virtual m5::stl::expected<void, m5::hal::error::error_t> cs_control(bool cs_level);
    protected:
        SPIMasterAccessConfig _access_config;
    };

    class SoftwareSPIMasterAccessor : public SPIMasterAccessor
    {
        public:
        SoftwareSPIMasterAccessor(Bus& bus, const SPIMasterAccessConfig& access_config) : SPIMasterAccessor { bus, access_config } {}
        m5::stl::expected<size_t, m5::hal::error::error_t> read(uint8_t* data, size_t len) override;
        m5::stl::expected<size_t, m5::hal::error::error_t> write(const uint8_t* data, size_t len) override;
    };

//-------------------------------------------------------------------------

    struct SPIBus : public Bus {
        types::bus_type_t getBusType(void) const override { return types::bus_type_t::SPI; };
        const BusConfig& getConfig(void) const override { return _config; }

        // virtual SPIMasterAccessor* beginAccess(const SPIMasterAccessConfig& access_config) = 0;
        // Accessor* beginAccess(const AccessConfig& access_config) override { return beginAccess((const SPIMasterAccessConfig&)access_config); }
    protected: 
        SPIBusConfig _config;
    };

    class SoftwareSPIBus : public SPIBus
    {
    public:
        error::error_t init(const BusConfig& config) override;
        m5::stl::expected<Accessor*, m5::hal::error::error_t> beginAccess(const AccessConfig& access_config) override;
        // SPIMasterAccessor* beginAccess(const SPIMasterAccessConfig& access_config) override;
    };

    m5::stl::expected<SPIBus*, m5::hal::error::error_t> getBus(const SPIBusConfig& config);
    // SPIBus* createBus(const SPIBusConfig& config); // { return spi::createBus(config); }
} // namespace spi


} // namespace bus
} // namespace hal
} // namespace m5

#endif
