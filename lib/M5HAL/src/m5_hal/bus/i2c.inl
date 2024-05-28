
#include "i2c.hpp"
#include "../error.hpp"

namespace m5 {
namespace hal {
namespace bus {
namespace i2c {

void delayCycle(uint32_t count)
{
    for (uint32_t i = count; i > 0; --i)
    {
        __asm__ __volatile__("nop");
    }
}

m5::stl::expected<I2CBus*, m5::hal::error::error_t> getBus(const I2CBusConfig& config)
{
    // @TODO ソフトウェアSPIの複数のインスタンスを管理できるようにすること。
    static SoftwareI2CBus bus;
    bus.init(config);
    return &bus;
}

error::error_t SoftwareI2CBus::init(const BusConfig& config) {
    if (config.getBusType() != types::bus_type_t::I2C) {
        M5_LIB_LOGE("SoftwareI2C::init: error %s", __PRETTY_FUNCTION__);
        return error::error_t::INVALID_ARGUMENT;
    }
    _config = static_cast<const I2CBusConfig&>(config);
    if (_config.pin_scl == nullptr || _config.pin_sda == nullptr) {
      M5_LIB_LOGE("SoftwareI2C::init: error %s", __PRETTY_FUNCTION__);
      return error::error_t::INVALID_ARGUMENT;
    }
    M5_LIB_LOGV("SoftwareI2C::init: ok %s", __PRETTY_FUNCTION__);

    _config.pin_scl->setMode(m5::hal::types::gpio_mode_t::Output_OpenDrain);
    _config.pin_scl->writeLow();
    _config.pin_sda->setMode(m5::hal::types::gpio_mode_t::Output_OpenDrain);
    _config.pin_sda->writeLow();
    _config.pin_scl->writeHigh();
    delayCycle(128);
    _config.pin_sda->writeHigh();

    return error::error_t::OK;
}

m5::stl::expected<m5::hal::bus::Accessor*, m5::hal::error::error_t> SoftwareI2CBus::beginAccess(const m5::hal::bus::AccessConfig& access_config)
{
    /// @TODO ここで排他制御＆ロック処理を行うこと。
    if (_Accessor.get() != nullptr) {
        M5_LIB_LOGE("SoftwareI2C::beginAccess: error %s", __PRETTY_FUNCTION__);
        return m5::stl::make_unexpected( m5::hal::error::error_t::INVALID_ARGUMENT );
    }
    if (access_config.getBusType() != getBusType()) {
        M5_LIB_LOGE("SoftwareI2C::beginAccess: error %s", __PRETTY_FUNCTION__);
        return m5::stl::make_unexpected( m5::hal::error::error_t::INVALID_ARGUMENT );
    }
    auto result = new SoftwareI2CMasterAccessor(*this, (I2CMasterAccessConfig&)access_config);
    _Accessor.reset(result);
    M5_LIB_LOGV("SoftwareI2C::beginAccess: ok %s", __PRETTY_FUNCTION__);
    return result;
}

m5::stl::expected<void, m5::hal::error::error_t> I2CMasterAccessor::sendDummyClockWhileSdaLow(size_t count)
{
    auto bc = static_cast<const I2CBusConfig&>(getBusConfig());

    auto sda = bc.pin_sda;
    auto scl = bc.pin_scl;

    /// SDAがLOWになっている場合は処理
    if (!sda->read())
    {
/// @TODO:ディレイサイクル数を求める仕組みを作っておくこと。
int delay_cycle = 200;//_nop_wait_w >> 1;

      scl->writeLow();
      sda->writeHigh();
      delayCycle(delay_cycle);

      // SDAがHIGHになるまでクロック送出しながら待機する。;
      while (!(sda->read()) && (--count))
      {
        scl->writeHigh();
        delayCycle(delay_cycle);
        scl->writeLow();
        delayCycle(delay_cycle);
      }
    }
    if (count == 0) { return m5::stl::make_unexpected ( error::error_t::I2C_BUS_ERROR ); }
    return {};
}

m5::stl::expected<void, m5::hal::error::error_t> I2CMasterAccessor::sendStartCondition(void)
{
    auto bc = static_cast<const I2CBusConfig&>(getBusConfig());

    auto sda = bc.pin_sda;
    auto scl = bc.pin_scl;

/// @TODO:ディレイサイクル数を求める仕組みを作っておくこと。
int delay_cycle = 200;//_nop_wait_w >> 1;

// start condition
    scl->writeHigh();
    delayCycle(delay_cycle);
    sda->writeLow();
    delayCycle(delay_cycle);
    scl->writeLow();

    return {};
}

m5::stl::expected<void, m5::hal::error::error_t> I2CMasterAccessor::sendStopCondition(void)
{
    auto bc = static_cast<const I2CBusConfig&>(getBusConfig());
    auto sda = bc.pin_sda;
    auto scl = bc.pin_scl;

/// @TODO:ディレイサイクル数を求める仕組みを作っておくこと。
int delay_cycle = 200;//_nop_wait_w >> 1;

    auto res = sendDummyClockWhileSdaLow();
    if (res.has_value()) {
  // stop condition
      sda->writeLow();
      scl->writeHigh();
      delayCycle(delay_cycle);
      sda->writeHigh();
    }
    return res;
}

m5::stl::expected<void, m5::hal::error::error_t> I2CMasterAccessor::waitClockStretch(uint_fast16_t msec)
{
    auto bc = static_cast<const I2CBusConfig&>(getBusConfig());
    auto scl = bc.pin_scl;
    auto ms = m5::utility::millis();
    do
    {
      std::this_thread::yield();
      if (scl->read())
      {
        return {};
      }
    } while ((m5::utility::millis() - ms) < msec);
    return m5::stl::make_unexpected ( error::error_t::I2C_BUS_ERROR );
}

m5::stl::expected<void, m5::hal::error::error_t> I2CMasterAccessor::startWrite(void)
{
    auto ac = static_cast<const I2CMasterAccessConfig&>(getAccessConfig());
    sendStartCondition();
    uint8_t data[] = { (uint8_t)(ac.i2c_addr << 1) }; 
    auto result = write(data, sizeof(data));
    if (result.has_value()) return {};
    return m5::stl::make_unexpected(result.error());
}

m5::stl::expected<void, m5::hal::error::error_t> I2CMasterAccessor::startRead(void)
{
    auto ac = static_cast<const I2CMasterAccessConfig&>(getAccessConfig());
    sendStartCondition();
    uint8_t data[] = { (uint8_t)(ac.i2c_addr << 1 | 1) }; 
    auto result = write(data, sizeof(data));
    if (result.has_value()) return {};
    return m5::stl::make_unexpected(result.error());
}

m5::stl::expected<void, m5::hal::error::error_t> I2CMasterAccessor::stop(void)
{
    return sendStopCondition();
}

m5::stl::expected<size_t, m5::hal::error::error_t> SoftwareI2CMasterAccessor::write(const uint8_t* data, size_t len)
{
    auto bc = static_cast<const I2CBusConfig&>(getBusConfig());
    auto ac = static_cast<const I2CMasterAccessConfig&>(getAccessConfig());

    auto scl = bc.pin_scl;
    auto sda = bc.pin_sda;

/// @TODO:ディレイサイクル数を求める仕組みを作っておくこと。
int delay_cycle = 200;//_nop_wait_w >> 1;
    int_fast16_t dc0 = (delay_cycle) >> 2;
    int_fast16_t dc1 = delay_cycle - dc0;
    size_t result = 0;
    do
    {
      uint_fast8_t mask = 0x80;
      uint_fast8_t d = *data++;

      // 最初の1bitの送信
      sda->write((bool)(d & mask));
      delayCycle(dc0);
      scl->writeHigh();
      delayCycle(dc1);
      // クロックストレッチの判定と待機;
      if (!scl->read())
      { // タイムアウト時間までクロックストレッチが解除されるのを待つ;
        auto result = waitClockStretch(ac.timeout_msec);
        if (!result)
        {
            sda->writeHigh();
            return m5::stl::make_unexpected ( error::error_t::TIMEOUT_ERROR );
        }
      }

      mask >>= 1;
      do
      { // 2bit~8bit目の送信;
        scl->writeLow();
        sda->write((bool)(d & mask));
        delayCycle(dc0);
        scl->writeHigh();
        delayCycle(dc1);
        mask >>= 1;
      } while (mask);

      // ACK応答チェック;
      scl->writeLow();  // SCL lo
      sda->writeHigh(); // SDA hi
      delayCycle(dc1);
      scl->writeHigh(); // hi
      delayCycle(dc0);

      // クロックストレッチの判定と待機;
      if (!scl->read())
      { // タイムアウト時間までクロックストレッチが解除されるのを待つ;
        auto result = waitClockStretch(ac.timeout_msec);
        if (!result)
        {
          return m5::stl::make_unexpected ( error::error_t::TIMEOUT_ERROR );
        }
      }
      if (sda->read())
      { // ToDo:ACK応答がない場合の処理;
        return m5::stl::make_unexpected ( error::error_t::I2C_NO_ACK );
      }
      scl->writeLow();
    } while (++result < len);
    return result;
}

m5::stl::expected<size_t, m5::hal::error::error_t> SoftwareI2CMasterAccessor::readWithLastNackFlag(uint8_t* data, size_t len, bool last_nack)
{
    auto bc = static_cast<const I2CBusConfig&>(getBusConfig());
    auto ac = static_cast<const I2CMasterAccessConfig&>(getAccessConfig());

    auto scl = bc.pin_scl;
    auto sda = bc.pin_sda;

/// @TODO:ディレイサイクル数を求める仕組みを作っておくこと。
int delay_cycle = 200;//_nop_wait_w;
    int_fast16_t dc0 = (delay_cycle) >> 1;
    int_fast16_t dc1 = delay_cycle - dc0;
    size_t result = 0;
    do
    {
      // 最初の1bitの受信
      sda->writeHigh();

      delayCycle(dc0);
      scl->writeHigh();
      delayCycle(dc1);

      // クロックストレッチの判定と待機;
      if (!scl->read())
      { // タイムアウト時間までクロックストレッチが解除されるのを待つ;
        if (!waitClockStretch(ac.timeout_msec))
        {
          return m5::stl::make_unexpected ( error::error_t::TIMEOUT_ERROR );
        }
      }

      uint_fast8_t mask = 0x80;
      uint_fast8_t byte = sda->read() ? mask : 0;
      mask >>= 1;

      do
      {
        scl->writeLow();
        delayCycle(dc0);
        scl->writeHigh();
        delayCycle(dc1);

        if (sda->read())
        {
          byte |= mask;
        }
        mask >>= 1;
      } while (mask);
      scl->writeLow();
      /// ACKを返す (ただし、データ末尾かつNACK指定がある場合はACKを返さない);
      if ((++result < len) || !last_nack)
      {
        sda->writeLow();
      }
      delayCycle(dc0);
      scl->writeHigh();
      delayCycle(dc1);
      scl->writeLow();
      *data++ = byte;
    } while (result < len);
    return result;
}



// I2CBus* createBus(const I2CBusConfig& config)
// {
//     static SoftwareI2CBus i2c_bus;
//     return &i2c_bus;
// }

// error::error_t SoftwareI2CMasterAccessor::init(const interface::bus::BusConfig& config) { return error::error_t::NOT_IMPLEMENTED; }
// error::error_t SoftwareI2CMasterAccessor::release(void) { return error::error_t::NOT_IMPLEMENTED; }
// error::error_t SoftwareI2CMasterAccessor::start(void) { return error::error_t::NOT_IMPLEMENTED; }
// error::error_t SoftwareI2CMasterAccessor::stop(void) { return error::error_t::NOT_IMPLEMENTED; }
// error::error_t SoftwareI2CMasterAccessor::lock(void) { return error::error_t::NOT_IMPLEMENTED; }
// error::error_t SoftwareI2CMasterAccessor::unlock(void) { return error::error_t::NOT_IMPLEMENTED; }
// error::error_t SoftwareI2CMasterAccessor::beginAccess(interface::bus::Accessor* Accessor) { return error::error_t::NOT_IMPLEMENTED; }
// error::error_t SoftwareI2CMasterAccessor::endAccess(void) { return error::error_t::NOT_IMPLEMENTED; }
// error::error_t SoftwareI2CMasterAccessor::setFrequency(uint32_t freq) { return error::error_t::NOT_IMPLEMENTED; }
// error::error_t SoftwareI2CMasterAccessor::read(uint8_t* data, size_t len, interface::io::AsyncResult* async) { return error::error_t::NOT_IMPLEMENTED; }
// error::error_t SoftwareI2CMasterAccessor::write(const uint8_t* data, size_t len, interface::io::AsyncResult* async) { return error::error_t::NOT_IMPLEMENTED; }


} // namespace i2c
} // namespace bus
} // namespace hal
} // namespace m5
