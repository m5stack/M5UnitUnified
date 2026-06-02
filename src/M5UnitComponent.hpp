/*
 * SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
/*!
  @file M5UnitComponent.hpp
  @brief Main header of M5UnitComponent
*/
#ifndef M5_UNIT_COMPONENT_HPP
#define M5_UNIT_COMPONENT_HPP

#include "m5_unit_component/types.hpp"
#include "m5_unit_component/adapter.hpp"
#if defined(ESP_PLATFORM)
#include <driver/uart.h>        // for uart_port_t
#include <driver/spi_master.h>  // for spi_device_handle_t
#endif
#if defined(ESP_PLATFORM) && __has_include(<driver/i2c_master.h>)
#include <driver/i2c_master.h>  // for i2c_master_bus_handle_t
#elif defined(ESP_PLATFORM)
#include <driver/i2c.h>  // for i2c_port_t / gpio_num_t
#endif
#include <cstdint>
#include <vector>
#include <algorithm>
#include <iterator>
#include <type_traits>
#include <memory>

#if defined(ARDUINO) || defined(DOXYGEN_PROCESS)
class TwoWire;
class HardwareSerial;
class SPIClass;
struct SPISettings;
#endif

namespace m5 {
class I2C_Class;
namespace unit {

class UnitUnified;
class Adapter;

/*!
  @class m5::unit::Component
  @brief Base class of unit component
 */
class Component {
public:
    /*!
      @struct component_config_t
      @brief Component basic settings for begin
     */
    struct component_config_t {
        //! Clock for communication (default as 100000)
        uint32_t clock{100000};
        //! Maximum number of periodic measurement data to be stored
        uint32_t stored_size{1};
        //! Does the user call Unit's update? (default as false)
        bool self_update{false};
        //! Maximum number of units that can be connected (default as 0)
        uint8_t max_children{0};
    };

    ///@warning Define the same name and type in the derived class.
    ///@name Fixed parameters for class
    ///@{
    static const types::uid_t uid;    //!< @brief Unique identifier
    static const types::attr_t attr;  //!< @brief Attributes
    static const char name[];         //!< @brief Device name string
    ///@}

    ///@warning COPY PROHIBITED
    ///@name Constructor
    ///@{
    explicit Component(const uint8_t addr = 0x00);  // I2C address

    Component(const Component&) = delete;

    Component(Component&&) noexcept = default;
    ///@}

    ///@warning COPY PROHIBITED
    ///@name Assignment
    ///@{
    Component& operator=(const Component&) = delete;

    Component& operator=(Component&&) noexcept = default;
    ///@}

    virtual ~Component() = default;

    ///@name Component settings
    ///@{
    /*!
      @brief Gets the common configurations in each unit
      @return Current component configuration
    */
    inline component_config_t component_config() const
    {
        return _component_cfg;
    }
    /*!
      @brief Set the common configurations in each unit
      @param cfg Configuration to apply
    */
    inline void component_config(const component_config_t& cfg)
    {
        _component_cfg = cfg;
    }
    ///@}

    ///@name Functions that must be inherited
    ///@{
    /*!
      @brief Begin unit
      @details Initiate functions based on component config and unit config
    */
    virtual bool begin()
    {
        return true;
    }
    /*!
      @brief Update unit
      @param force Forced communication for updates if true
    */
    virtual void update(const bool force = false)
    {
        (void)force;
    }
    ///@}

    ///@name Properties
    ///@{
    /*!
      @brief Gets the device name
      @return Pointer to the null-terminated device name string
    */
    inline const char* deviceName() const
    {
        return unit_device_name();
    }
    /*!
      @brief Gets the identifier
      @return Unique identifier of the unit
    */
    inline types::uid_t identifier() const
    {
        return unit_identifier();
    }
    /*!
      @brief Gets the attributes
      @return Attribute flags of the unit
    */
    inline types::attr_t attribute() const
    {
        return unit_attribute();
    }
    /*!
      @brief Gets the category
      @return Category of the unit
    */
    inline types::category_t category() const
    {
        return unit_category();
    }
    /*!
      @brief Gets the registered order (== 0 means not yet)
      @return Registration order; 0 if not yet registered
    */
    inline uint32_t order() const
    {
        return _order;
    }
    /*!
      @brief Gets the channel if connected to another unit
      @return Channel number; negative if not connected to a parent
    */
    inline int16_t channel() const
    {
        return _channel;
    }
    /*!
      @brief Is the unit registered with the manager?
      @return True if registered with a UnitUnified manager
    */
    inline bool isRegistered() const
    {
        return _manager != nullptr;
    }
    /*!
      @brief Address used to I2C access the device
      @return I2C address of the device
    */
    inline uint8_t address() const
    {
        return _addr;
    }
    /*!
      @brief Gets the access adapter
      @return Pointer to the adapter; ownership is retained by the unit
      @warning Ownership is retained by the unit and should not be released
     */
    inline Adapter* adapter() const
    {
        return _adapter.get();
    }
    /*!
      @brief Gets the access adapter cast to the specified type
      @tparam T Adapter-derived pointer type to cast to
      @param t Expected adapter type
      @return Pointer to the adapter cast to T, or nullptr if type does not match
    */
    template <class T>
    inline auto asAdapter(const Adapter::Type t) ->
        typename std::remove_cv<typename std::remove_pointer<T>::type>::type*
    {
        using U = typename std::remove_cv<typename std::remove_pointer<T>::type>::type;
        static_assert(std::is_base_of<Adapter, U>::value, "T must be derived from Adapter");
        return (_adapter->type() == t) ? static_cast<U*>(_adapter.get()) : nullptr;
    }
    /*!
      @brief Gets the access adapter cast to the specified type (const overload)
      @tparam T Adapter-derived pointer type to cast to
      @param t Expected adapter type
      @return Const pointer to the adapter cast to T, or nullptr if type does not match
    */
    template <class T>
    inline auto asAdapter(const Adapter::Type t) const -> const
        typename std::remove_cv<typename std::remove_pointer<T>::type>::type*
    {
        using U = typename std::remove_cv<typename std::remove_pointer<T>::type>::type;
        static_assert(std::is_base_of<Adapter, U>::value, "T must be derived from Adapter");
        return (_adapter->type() == t) ? static_cast<const U*>(_adapter.get()) : nullptr;
    }
    ///@}

    ///@name Attributes
    ///@{
    /*!
      @brief Can the unit access via I2C?
      @return True if the current adapter supports I2C access
    */
    bool canAccessI2C() const;
    /*!
      @brief Can the unit access via GPIO?
      @return True if the current adapter supports GPIO access
    */
    bool canAccessGPIO() const;
    /*!
      @brief Can the unit access via UART?
      @return True if the current adapter supports UART access
    */
    bool canAccessUART() const;
    /*!
      @brief Can the unit access via SPI?
      @return True if the current adapter supports SPI access
    */
    bool canAccessSPI() const;
    ///@}

    ///@name Periodic measurement
    ///@{
    /*!
      @brief In periodic measurement?
      @return True if the unit is currently in periodic measurement mode
    */
    inline bool inPeriodic() const
    {
        return in_periodic();
    }
    /*!
      @brief Periodic measurement data updated?
      @return True if measurement data was updated in the last update() call
    */
    inline bool updated() const
    {
        return _updated;
    }
    /*!
      @brief Time elapsed since start-up when the measurement data was updated in update()
      @return Updated time (Unit: ms)
    */
    inline types::elapsed_time_t updatedMillis() const
    {
        return _latest;
    }
    /*!
      @brief Gets the periodic measurement interval
      @return interval time (Unit: ms)
     */
    inline types::elapsed_time_t interval() const
    {
        return _interval;
    }
    ///@}

    ///@name Assign(I2C)
    ///@{
#if defined(ARDUINO) || defined(DOXYGEN_PROCESS)
    /*!
      @brief Assign TwoWire as the communication bus
      @param wire TwoWire to be used
      @return True if successful
    */
    virtual bool assign(TwoWire& wire);
#endif
#if defined(DOXYGEN_PROCESS) || (defined(ESP_PLATFORM) && __has_include(<driver/i2c_master.h>))
    /*!
      @brief Assign I2C master bus (ESP-IDF native driver)
      @param bus ESP-IDF I2C master bus handle
      @return True if successful
     */
    virtual bool assign(i2c_master_bus_handle_t bus);
#endif
#if defined(DOXYGEN_PROCESS) || (defined(ESP_PLATFORM) && !__has_include(<driver/i2c_master.h>))
    /*!
      @brief Assign I2C (ESP-IDF legacy driver, pre-installed port)
      @param port I2C port (the driver must be installed beforehand via i2c_param_config / i2c_driver_install)
      @param sda SDA GPIO
      @param scl SCL GPIO
      @return True if successful
     */
    virtual bool assign(const i2c_port_t port, const gpio_num_t sda, const gpio_num_t scl);
#endif
    /*!
      @brief Assign I2C_Class as the communication bus
      @param i2c I2C_Class to be used (e.g. M5.In_I2C)
      @return True if successful
    */
    virtual bool assign(m5::I2C_Class& i2c);
    ///@}

    ///@name Assign(GPIO)
    ///@{
    /*!
      @brief Assign GPIO pins as the communication bus
      @param rx_pin Pin number to be used for RX
      @param tx_pin Pin number to be used for TX
      @return True if successful
    */
    virtual bool assign(const int8_t rx_pin, const int8_t tx_pin);
    ///@}

    ///@name Assign(UART)
    ///@{
#if defined(ARDUINO) || defined(DOXYGEN_PROCESS)
    /*!
      @brief Assign HardwareSerial as the communication bus
      @param serial HardwareSerial to be used
      @return True if successful
    */
    virtual bool assign(HardwareSerial& serial);
#endif
#if defined(ESP_PLATFORM) || defined(DOXYGEN_PROCESS)
    /*!
      @brief Assign UART (ESP-IDF native driver, pre-installed port)
      @param uart_num UART port number (the driver must be installed beforehand via
                      uart_driver_install / uart_param_config / uart_set_pin)
      @return True if successful
     */
    virtual bool assign(const uart_port_t uart_num);
#endif
    ///@}

    ///@name Assign(SPI)
    ///@{
#if defined(ARDUINO) || defined(DOXYGEN_PROCESS)
    /*!
      @brief Assign SPIClass as the communication bus
      @param spi SPIClass to be used
      @param settings SPI settings to be applied
      @return True if successful
    */
    virtual bool assign(SPIClass& spi, const SPISettings& settings);
#endif
#if defined(ESP_PLATFORM) || defined(DOXYGEN_PROCESS)
    /*!
      @brief Assign SPI device handle (ESP-IDF native driver, borrowed)
      @param handle ESP-IDF SPI device handle (create with spics_io_num = -1; init bus with SPI_DMA_DISABLED)
      @param cs CS GPIO controlled manually by this library. If `GPIO_NUM_NC` (default), uses `address()` as the CS pin
      (same convention as Arduino SPI)
      @return True if successful
     */
    virtual bool assign(spi_device_handle_t handle, const gpio_num_t cs = GPIO_NUM_NC);
#endif
    ///@}

    ///@name Assign(M5HAL)
    ///@{
    /*!
      @brief Assign m5::hal::bus as the communication bus
      @param bus Bus to be used
      @return True if successful
    */
    virtual bool assign(m5::hal::bus::Bus* bus);
    ///@}

    ///@note For daisy-chaining units such as hubs
    ///@name Parent-children relationship
    ///@{
    /*!
      @brief Has parent unit?
      @return True if this unit is connected to a parent unit
    */
    inline bool hasParent() const
    {
        return _parent != nullptr;
    }
    /*!
      @brief Are there any other devices connected to the same parent unit besides yourself?
      @return True if sibling units exist on the same parent
    */
    inline bool hasSiblings() const
    {
        return (_prev != nullptr) || (_next != nullptr);
    }
    /*!
      @brief Are there other devices connected to me?
      @return True if at least one child unit is connected
    */
    inline bool hasChildren() const
    {
        return _child;
    }
    /*!
      @brief Number of units connected to me
      @return Number of child units connected to this unit
    */
    size_t childrenSize() const;
    /*!
      @brief Is there another unit connected to the specified channel?
      @param ch Channel number to check
      @return True if a child unit is connected on the specified channel
    */
    bool existsChild(const uint8_t ch) const;
    /*!
      @brief Gets the parent unit
      @return Pointer to the parent unit, or nullptr if there is no parent
    */
    inline Component* parent()
    {
        return _parent;
    }
    /*!
      @brief Gets the device connected to the specified channel
      @param channel Channel number to query
      @return Pointer to the child unit on that channel, or nullptr if none
    */
    Component* child(const uint8_t channel) const;
    /*!
      @brief Connect the unit to the specified channel
      @param c Child component to connect
      @param channel Channel number to connect on
      @return True if successful
    */
    bool add(Component& c, const int16_t channel);
    /*!
      @brief Select valid channel if exists
      @param ch Channel number to select
      @return True if the channel was selected successfully
    */
    bool selectChannel(const uint8_t ch = 8);
    ///@}

    ///@cond 0
    template <typename T>
    class iterator {
    public:
        using iterator_category = std::forward_iterator_tag;
        using difference_type   = std::ptrdiff_t;
        using value_type        = T;
        using pointer           = T*;
        using reference         = T&;

        explicit iterator(Component* c = nullptr) : _ptr(c)
        {
        }

        reference operator*() const
        {
            return *_ptr;
        }
        pointer operator->() const
        {
            return _ptr;
        }
        iterator& operator++()
        {
            _ptr = _ptr ? _ptr->_next : nullptr;
            return *this;
        }
        iterator operator++(int)
        {
            auto tmp = *this;
            ++(*this);
            return tmp;
        }
        friend bool operator==(const iterator& a, const iterator& b)
        {
            return a._ptr == b._ptr;
        }
        friend bool operator!=(const iterator& a, const iterator& b)
        {
            return a._ptr != b._ptr;
        }

    private:
        Component* _ptr;
    };

    using child_iterator       = iterator<Component>;
    using const_child_iterator = iterator<const Component>;
    inline child_iterator childBegin() noexcept
    {
        return child_iterator(_child);
    }
    inline child_iterator childEnd() noexcept
    {
        return child_iterator();
    }
    inline const_child_iterator childBegin() const noexcept
    {
        return const_child_iterator(_child);
    }
    inline const_child_iterator childEnd() const noexcept
    {
        return const_child_iterator();
    }
    ///@endcond

    /*!
      @brief General call for I2C
      @param data Pointer to data to send
      @param len Length of data
      @return True if successful
    */
    bool generalCall(const uint8_t* data, const size_t len);

    /*!
      @brief Output information for debug
      @return String containing debug information about this unit
    */
    virtual std::string debugInfo() const;

    ////// TODO : Split interface (I2C, GPIO, UART, SPI)

    // I2C R/W
    ///@cond 0
    m5::hal::error::error_t readWithTransaction(uint8_t* data, const size_t len);

    template <typename Reg,
              typename std::enable_if<std::is_integral<Reg>::value && std::is_unsigned<Reg>::value && sizeof(Reg) <= 2,
                                      std::nullptr_t>::type = nullptr>
    bool readRegister(const Reg reg, uint8_t* rbuf, const size_t len, const uint32_t delayMillis,
                      const bool stop = true);
    template <typename Reg,
              typename std::enable_if<std::is_integral<Reg>::value && std::is_unsigned<Reg>::value && sizeof(Reg) <= 2,
                                      std::nullptr_t>::type = nullptr>
    bool readRegister8(const Reg reg, uint8_t& result, const uint32_t delayMillis, const bool stop = true);

    template <typename Reg,
              typename std::enable_if<std::is_integral<Reg>::value && std::is_unsigned<Reg>::value && sizeof(Reg) <= 2,
                                      std::nullptr_t>::type = nullptr>
    inline bool readRegister16BE(const Reg reg, uint16_t& result, const uint32_t delayMillis, const bool stop = true)
    {
        return read_register16E(reg, result, delayMillis, stop, true);
    }

    template <typename Reg,
              typename std::enable_if<std::is_integral<Reg>::value && std::is_unsigned<Reg>::value && sizeof(Reg) <= 2,
                                      std::nullptr_t>::type = nullptr>
    inline bool readRegister16LE(const Reg reg, uint16_t& result, const uint32_t delayMillis, const bool stop = true)
    {
        return read_register16E(reg, result, delayMillis, stop, false);
    }

    template <typename Reg,
              typename std::enable_if<std::is_integral<Reg>::value && std::is_unsigned<Reg>::value && sizeof(Reg) <= 2,
                                      std::nullptr_t>::type = nullptr>
    inline bool readRegister32BE(const Reg reg, uint32_t& result, const uint32_t delayMillis, const bool stop = true)
    {
        return read_register32E(reg, result, delayMillis, stop, true);
    }

    template <typename Reg,
              typename std::enable_if<std::is_integral<Reg>::value && std::is_unsigned<Reg>::value && sizeof(Reg) <= 2,
                                      std::nullptr_t>::type = nullptr>
    inline bool readRegister32LE(const Reg reg, uint32_t& result, const uint32_t delayMillis, const bool stop = true)
    {
        return read_register32E(reg, result, delayMillis, stop, false);
    }

    m5::hal::error::error_t writeWithTransaction(const uint8_t* data, const size_t len, const uint32_t exparam = 1);

    template <typename Reg,
              typename std::enable_if<std::is_integral<Reg>::value && std::is_unsigned<Reg>::value && sizeof(Reg) <= 2,
                                      std::nullptr_t>::type = nullptr>
    m5::hal::error::error_t writeWithTransaction(const Reg reg, const uint8_t* data, const size_t len,
                                                 const bool stop = true);

    template <typename Reg,
              typename std::enable_if<std::is_integral<Reg>::value && std::is_unsigned<Reg>::value && sizeof(Reg) <= 2,
                                      std::nullptr_t>::type = nullptr>
    bool writeRegister(const Reg reg, const uint8_t* buf = nullptr, const size_t len = 0U, const bool stop = true);

    template <typename Reg,
              typename std::enable_if<std::is_integral<Reg>::value && std::is_unsigned<Reg>::value && sizeof(Reg) <= 2,
                                      std::nullptr_t>::type = nullptr>
    bool writeRegister8(const Reg reg, const uint8_t value, const bool stop = true);

    template <typename Reg,
              typename std::enable_if<std::is_integral<Reg>::value && std::is_unsigned<Reg>::value && sizeof(Reg) <= 2,
                                      std::nullptr_t>::type = nullptr>
    inline bool writeRegister16BE(const Reg reg, const uint16_t value, const bool stop = true)
    {
        return write_register16E(reg, value, stop, true);
    }

    template <typename Reg,
              typename std::enable_if<std::is_integral<Reg>::value && std::is_unsigned<Reg>::value && sizeof(Reg) <= 2,
                                      std::nullptr_t>::type = nullptr>
    inline bool writeRegister16LE(const Reg reg, const uint16_t value, const bool stop = true)
    {
        return write_register16E(reg, value, stop, false);
    }

    template <typename Reg,
              typename std::enable_if<std::is_integral<Reg>::value && std::is_unsigned<Reg>::value && sizeof(Reg) <= 2,
                                      std::nullptr_t>::type = nullptr>
    inline bool writeRegister32BE(const Reg reg, const uint32_t value, const bool stop = true)
    {
        return write_register32E(reg, value, stop, true);
    }

    template <typename Reg,
              typename std::enable_if<std::is_integral<Reg>::value && std::is_unsigned<Reg>::value && sizeof(Reg) <= 2,
                                      std::nullptr_t>::type = nullptr>
    inline bool writeRegister32LE(const Reg reg, const uint32_t value, const bool stop = true)
    {
        return write_register32E(reg, value, stop, false);
    }

    // GPIO
    bool pinModeRX(const gpio::Mode m);
    bool writeDigitalRX(const bool high);
    bool readDigitalRX(bool& high);
    bool writeAnalogRX(const uint16_t v);
    bool readAnalogRX(uint16_t& v);
    bool readAnalogMilliVoltsRX(uint32_t& mv);
    bool pulseInRX(uint32_t& duration, const int state, const uint32_t timeout_us = 1000000);

    bool pinModeTX(const gpio::Mode m);
    bool writeDigitalTX(const bool high);
    bool readDigitalTX(bool& high);
    bool writeAnalogTX(const uint16_t v);
    bool readAnalogTX(uint16_t& v);
    bool readAnalogMilliVoltsTX(uint32_t& mv);
    bool pulseInTX(uint32_t& duration, const int state, const uint32_t timeout_us = 1000000);
    ///@endcond

#if defined(DOXYGEN_PROCESS)
    // There is a problem with the Doxygen output of templates containing std::enable_if,
    // so we need a section for Doxygen output
    ///@name Read/Write
    ///@{
    //! @brief Read any data with transaction
    m5::hal::error::error_t readWithTransaction(uint8_t* data, const size_t len);
    //! @brief Read any data with transaction from register
    template <typename Reg>
    bool readRegister(const Reg reg, uint8_t* rbuf, const size_t len, const uint32_t delayMillis,
                      const bool stop = true);
    //! @brief Read byte with transaction from register
    template <typename Reg>
    bool readRegister8(const Reg reg, uint8_t& result, const uint32_t delayMillis, const bool stop = true);
    //! @brief Read word in big-endian order with transaction from register
    template <typename Reg>
    bool readRegister16BE(const Reg reg, uint16_t& result, const uint32_t delayMillis, const bool stop = true);
    //! @brief Read word in little-endian order with transaction from register
    template <typename Reg>
    bool readRegister16LE(const Reg reg, uint16_t& result, const uint32_t delayMillis, const bool stop = true);
    //! @brief Read dword in big-endian order with transaction from register
    template <typename Reg>
    bool readRegister32BE(const Reg reg, uint32_t& result, const uint32_t delayMillis, const bool stop = true);
    //! @brief Read dword in little-endian order with transaction from register
    template <typename Reg>
    bool readRegister32LE(const Reg reg, uint32_t& result, const uint32_t delayMillis, const bool stop = true);

    //! @brief Write any data with transaction
    m5::hal::error::error_t writeWithTransaction(const uint8_t* data, const size_t len, const bool stop = true);
    //! @brief Write any data with transaction to register
    template <typename Reg>
    m5::hal::error::error_t writeWithTransaction(const Reg reg, const uint8_t* data, const size_t len,
                                                 const bool stop = true);
    //! @brief Write any data with transaction to register
    template <typename Reg>
    bool writeRegister(const Reg reg, const uint8_t* buf = nullptr, const size_t len = 0U, const bool stop = true);
    //! @brief Write byte with transaction to register
    template <typename Reg>
    bool writeRegister8(const Reg reg, const uint8_t value, const bool stop = true);
    //! @brief Write word in big-endian order with transaction to register
    template <typename Reg>
    bool writeRegister16BE(const Reg reg, const uint16_t value, const bool stop = true);
    //! @brief Write word in little-endian order with transaction to register
    template <typename Reg>
    bool writeRegister16LE(const Reg reg, const uint16_t value, const bool stop = true);
    //! @brief Write dword in big-endian order with transaction to register
    template <typename Reg>
    bool writeRegister32BE(const Reg reg, const uint32_t value, const bool stop = true);
    //! @brief Write dword in little-endian order with transaction to register
    template <typename Reg>
    bool writeRegister32LE(const Reg reg, const uint32_t value, const bool stop = true);
    ///@}
#endif

protected:
    // Proper implementation in derived classes is required
    virtual const char* unit_device_name() const = 0;
    virtual types::uid_t unit_identifier() const = 0;
    virtual types::attr_t unit_attribute() const = 0;
    inline virtual types::category_t unit_category() const
    {
        return types::category_t::None;
    }
    inline virtual bool in_periodic() const
    {
        return _periodic;
    }

    inline virtual std::shared_ptr<Adapter> ensure_adapter(const uint8_t /*ch*/)
    {
        return _adapter;  // By default, offer my adapter for sharing
    }

    // Select valid channel if exists(PaHub etc...)
    inline virtual m5::hal::error::error_t select_channel(const uint8_t)
    {
        return m5::hal::error::error_t::OK;
    }

    inline size_t stored_size() const
    {
        return _component_cfg.stored_size;
    }

    bool add_child(Component* c);

    // I2C
    bool changeAddress(const uint8_t addr);  // Functions for dynamically addressable devices
    template <typename Reg,
              typename std::enable_if<std::is_integral<Reg>::value && std::is_unsigned<Reg>::value && sizeof(Reg) <= 2,
                                      std::nullptr_t>::type = nullptr>
    bool read_register16E(const Reg reg, uint16_t& result, const uint32_t delayMillis, const bool stop,
                          const bool endian);
    template <typename Reg,
              typename std::enable_if<std::is_integral<Reg>::value && std::is_unsigned<Reg>::value && sizeof(Reg) <= 2,
                                      std::nullptr_t>::type = nullptr>
    bool write_register16E(const Reg reg, const uint16_t value, const bool stop, const bool endian);
    template <typename Reg,
              typename std::enable_if<std::is_integral<Reg>::value && std::is_unsigned<Reg>::value && sizeof(Reg) <= 2,
                                      std::nullptr_t>::type = nullptr>
    bool read_register32E(const Reg reg, uint32_t& result, const uint32_t delayMillis, const bool stop,
                          const bool endian);
    template <typename Reg,
              typename std::enable_if<std::is_integral<Reg>::value && std::is_unsigned<Reg>::value && sizeof(Reg) <= 2,
                                      std::nullptr_t>::type = nullptr>
    bool write_register32E(const Reg reg, const uint32_t value, const bool stop, const bool endian);

protected:
    // For periodic measurement
    types::elapsed_time_t _latest{}, _interval{};
    bool _periodic{};  // During periodic measurement?
    bool _updated{};

private:
    UnitUnified* _manager{};
    std::shared_ptr<m5::unit::Adapter> _adapter{};

    uint32_t _order{};
    component_config_t _component_cfg{};
    int16_t _channel{-1};  // valid [0...]
    uint8_t _addr{};
    bool _begun{};

    // for chain
    Component* _parent{};
    Component* _next{};
    Component* _prev{};
    Component* _child{};

    friend class UnitUnified;
};

/*!
  @class PeriodicMeasurementAdapter
  @brief Interface class for periodic measurement (CRTP)
  @details Common interface for accumulated periodic measurement data
  @details Provide a common interface for periodic measurements for each unit
  @tparam Derived Derived class
  @tparam MD Type of the measurement data group
  @warning MUST IMPLEMENT some functions (NOT VIRTUAL)
  - MD Derived::oldest_periodic_data() const;
  - MD Derived::latest_periodic_data() const;
  - bool Derived::start_periodic_measurement(any arguments);
  - bool Derived::stop_periodic_measurement():
  @warning  MUST ADD std::unique_ptr<m5::container::CircularBuffer<MD>> _data{}
  in Derived class
  @warning This class is an interface class and should not have any data
  @note See also M5_UNIT_COMPONENT_PERIODIC_MEASUREMENT_ADAPTER_HPP_BUILDER
*/
template <class Derived, typename MD>
class PeriodicMeasurementAdapter {
public:
    ///@name Periodic measurement
    ///@{
    /*!
      @brief Start periodic measurement
      @tparam Args Optional arguments
      @return True if successful
      @note Call Derived::start_periodic_measurement
    */
    template <typename... Args>
    inline bool startPeriodicMeasurement(Args&&... args)
    {
        // Prepare for future common initiation preprocessing needs
        return static_cast<Derived*>(this)->start_periodic_measurement(std::forward<Args>(args)...);
    }
    /*!
      @brief Stop periodic measurement
      @tparam Args Optional arguments
      @return True if successful
      @note Call Derived::stop_periodic_measurement
    */
    template <typename... Args>
    inline bool stopPeriodicMeasurement(Args&&... args)
    {
        // Prepare for future common stopping preprocessing needs
        return static_cast<Derived*>(this)->stop_periodic_measurement(std::forward<Args>(args)...);
    }
    ///@}

    ///@name Data
    ///@{
    //! @brief Gets the number of stored data
    inline size_t available() const
    {
        return available_periodic_measurement_data();
    }
    //! @brief Is empty stored data?
    inline bool empty() const
    {
        return empty_periodic_measurement_data();
    }
    //! @brief Is stored data full?
    inline bool full() const
    {
        return full_periodic_measurement_data();
    }
    //! @brief Retrieve oldest stored data
    inline MD oldest() const
    {
        return static_cast<const Derived*>(this)->oldest_periodic_data();
    }
    //! @brief Retrieve latest stored data
    inline MD latest() const
    {
        return static_cast<const Derived*>(this)->latest_periodic_data();
    }
    //! @brief Discard the oldest data accumulated
    inline void discard()
    {
        discard_periodic_measurement_data();
    }
    //! @brief Discard all data
    inline void flush()
    {
        flush_periodic_measurement_data();
    }
    ///@}

protected:
    ///@note Must implement in derived class
    ///@name Pure virtual functions
    ///@{
    virtual size_t available_periodic_measurement_data() const = 0;
    virtual bool empty_periodic_measurement_data() const       = 0;
    virtual bool full_periodic_measurement_data() const        = 0;
    virtual void discard_periodic_measurement_data()           = 0;
    virtual void flush_periodic_measurement_data()             = 0;
    ///@}
};

}  // namespace unit
}  // namespace m5

// Helper for creating derived classes from Component
///@cond
#define M5_UNIT_COMPONENT_HPP_BUILDER(cls, reg)                  \
public:                                                          \
    constexpr static uint8_t DEFAULT_ADDRESS{(reg)};             \
    static const types::uid_t uid;                               \
    static const types::attr_t attr;                             \
    static const char name[];                                    \
                                                                 \
    cls(const cls&) = delete;                                    \
                                                                 \
    cls& operator=(const cls&) = delete;                         \
                                                                 \
    cls(cls&&) noexcept = default;                               \
                                                                 \
    cls& operator=(cls&&) noexcept = default;                    \
                                                                 \
protected:                                                       \
    inline virtual const char* unit_device_name() const override \
    {                                                            \
        return name;                                             \
    }                                                            \
    inline virtual types::uid_t unit_identifier() const override \
    {                                                            \
        return uid;                                              \
    }                                                            \
    inline virtual types::attr_t unit_attribute() const override \
    {                                                            \
        return attr;                                             \
    }

// Helper for creating derived class from PeriodicMeasurementAdapter
#define M5_UNIT_COMPONENT_PERIODIC_MEASUREMENT_ADAPTER_HPP_BUILDER(cls, md)    \
protected:                                                                     \
    friend class PeriodicMeasurementAdapter<cls, md>;                          \
                                                                               \
    inline md oldest_periodic_data() const                                     \
    {                                                                          \
        return !_data->empty() ? _data->front().value() : md{};                \
    }                                                                          \
    inline md latest_periodic_data() const                                     \
    {                                                                          \
        return !_data->empty() ? _data->back().value() : md{};                 \
    }                                                                          \
    inline virtual size_t available_periodic_measurement_data() const override \
    {                                                                          \
        return _data->size();                                                  \
    }                                                                          \
    inline virtual bool empty_periodic_measurement_data() const override       \
    {                                                                          \
        return _data->empty();                                                 \
    }                                                                          \
    inline virtual bool full_periodic_measurement_data() const override        \
    {                                                                          \
        return _data->full();                                                  \
    }                                                                          \
    inline virtual void discard_periodic_measurement_data() override           \
    {                                                                          \
        _data->pop_front();                                                    \
    }                                                                          \
    inline virtual void flush_periodic_measurement_data() override             \
    {                                                                          \
        _data->clear();                                                        \
    }

///@endcond
#endif
