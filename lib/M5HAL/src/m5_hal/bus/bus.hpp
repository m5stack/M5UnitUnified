
#ifndef M5_HAL_BUS_HPP_
#define M5_HAL_BUS_HPP_

#include "../interface/gpio.hpp"
#include "../interface/io.hpp"
#include "../error.hpp"

#include <memory>

/*!
  @namespace m5
  @brief Toplevel namespace of M5
 */
namespace m5 {
namespace hal {

namespace bus {

struct Bus;
struct AccessConfig;
struct Accessor;

//-------------------------------------------------------------------------
// 通信バスの初期化に必要な条件を記述するため基底インターフェイス
struct BusConfig {
    virtual ~BusConfig() = default;
    virtual types::bus_type_t getBusType(void) const = 0;
    types::periph_t periph;
};

//-------------------------------------------------------------------------
// 実際の通信に必要な条件を記述するための基底インターフェイス
struct AccessConfig {
    virtual ~AccessConfig() = default;
    virtual types::bus_type_t getBusType(void) const = 0;
    void (*callback_begin)(Accessor*) = nullptr; // 通信開始時時に呼び出すコールバック関数
    void (*callback_end)(Accessor*) = nullptr;   // 通信終了時に呼び出すコールバック関数
};

//-------------------------------------------------------------------------
// 特定の通信相手との各種通信条件を記述するため基底インターフェイス
struct Accessor : public interface::io::Output, public interface::io::Input
 {
    virtual ~Accessor(void) = default;
    virtual types::bus_type_t getBusType(void) const = 0;
    virtual const AccessConfig& getAccessConfig(void) const = 0;

    Accessor(Bus& bus) : _bus { bus } {}
    Bus& getBus(void) const { return _bus; };
    const BusConfig& getBusConfig(void) const;

    virtual m5::stl::expected<void, m5::hal::error::error_t> startWrite(void) { return {}; }
    virtual m5::stl::expected<void, m5::hal::error::error_t> startRead(void) { return {}; }
    virtual m5::stl::expected<void, m5::hal::error::error_t> stop(void) { return {}; }
    virtual m5::stl::expected<size_t, m5::hal::error::error_t> readLastNack(uint8_t* data, size_t len) { return readWithLastNackFlag(data, len, true); }
    virtual m5::stl::expected<size_t, m5::hal::error::error_t> readWithLastNackFlag(uint8_t* data, size_t len, bool last_nack = false) { return read(data, len); }
protected:
    Bus& _bus;
};

template<typename TBusAccessInterface>
struct DefaultBusExtension {
    DefaultBusExtension(TBusAccessInterface&) {}
};

template<typename TBus, typename TBusAccessInterface, typename TExtension = DefaultBusExtension<TBusAccessInterface>>
struct BusAccessor {
    BusAccessor() = delete;
    BusAccessor(TBus& bus, TBusAccessInterface& access) : _bus(bus), _access(access), _extension(access) {}
    BusAccessor(const BusAccessor&) = delete;
    BusAccessor& operator=(const BusAccessor&) = delete;
    BusAccessor(BusAccessor&& other) : _owner(other.owner), _bus(other.bus), _access(other.access), _extension(other.access) {
        other._owner = false;
    }
    void release() {
        if( this->_owner ) {
            this->_bus.release();
        }
        this->_owner = false;
    }
    BusAccessor& operator=(BusAccessor&& other) {
        this->release();
        this->_owner = other._owner;
        this->_bus = other._bus;
        this->_access = other._access;
        this->_extension = TExtension(this->_access);
        other._owner = false;
    }
/*
    void read() { this->_access.read(this->_bus);}
    void write() { this->_access.write(this->_bus); }
    void transmit() { this->_access.transmit(this->_bus); }
*/
    TExtension& extension() { return this->_extension; }

    ~BusAccessor() {
        this->release();
    }
protected:
    bool _owner = true;
    TBus& _bus;
    TBusAccessInterface& _access;
    TExtension _extension;
};

//-------------------------------------------------------------------------
// 通信バスの基底インターフェイス
struct Bus {
public:
    virtual ~Bus() = default;
    virtual types::bus_type_t getBusType(void) const = 0;

    // initとreleaseはユーザーは直接呼ばない。 createBus / releaseBus経由で使用する。

    virtual const BusConfig& getConfig(void) const = 0;

    // バスの初期化
    virtual error::error_t init(const BusConfig& config) { return error::error_t::NOT_IMPLEMENTED; }
    // バスの解放
    virtual error::error_t release(void) { return error::error_t::NOT_IMPLEMENTED; }

    /// @note  expectedは参照をサポートしていないのでポインタを含める形とした。
    virtual m5::stl::expected<Accessor*, m5::hal::error::error_t> beginAccess(const AccessConfig& access_config) = 0;

    virtual error::error_t endAccess(Accessor* Accessor) {
        if (Accessor && _Accessor.get() == Accessor) {
            _Accessor.reset(nullptr);
            return error::error_t::OK;
        }
        return error::error_t::INVALID_ARGUMENT;
    }

    // 無条件でバスをロックしたい場合に使用する
    virtual m5::stl::expected<void, error::error_t> lock(void) { return m5::stl::make_unexpected(error::error_t::NOT_IMPLEMENTED); }
    virtual m5::stl::expected<void, error::error_t> unlock(void) { return m5::stl::make_unexpected(error::error_t::NOT_IMPLEMENTED); }

protected:
    std::unique_ptr<Accessor> _Accessor;
};

//-------------------------------------------------------------------------
// error::error_t Bus::beginAccess(const Accessor* Accessor) { return (Accessor->getBusType() != getBusType()) ? error::error_t::INVALID_ARGUMENT : error::error_t::OK; }

} // namespace bus
} // namespace hal
} // namespace m5

#endif
