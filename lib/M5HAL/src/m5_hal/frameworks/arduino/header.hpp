#ifndef M5_HAL_FRAMEWORKS_ARDUINO_HEADER_HPP
#define M5_HAL_FRAMEWORKS_ARDUINO_HEADER_HPP

#include "../../interface/gpio.hpp"
#include "../../bus/bus.hpp"
#if __has_include(<Arduino.h>)
#include <Arduino.h>
#include <Wire.h>
#endif

#if defined( ARDUINO )

namespace m5 {
namespace hal {
namespace frameworks {
namespace arduino {
namespace gpio {

    // 単一のデジタルピンを表すインターフェイス
    class Pin : public interface::gpio::Pin {
    public:
        void write(bool value) override;
        void writeHigh(void) override;
        void writeLow(void) override;
        bool read(void) override;

        void setMode(types::gpio_mode_t mode) override;
        types::gpio_number_t getGpioNumber(void) const override { return _gpio_number; }

        Pin(void) = default;
        Pin(types::gpio_number_t gpio_number) : _gpio_number { gpio_number }
        {}
    protected:
        types::gpio_number_t _gpio_number;// = (int16_t)-1;
    };

    // 複数のPinを束ねた概念を表すインターフェイス
    class Port : public interface::gpio::Port {
    public:
        interface::gpio::Pin* getPin(uint8_t pinNumber) override;
        // uint8_t getPortNumber(void) const override { return 0; }
        // void setDirection(uint8_t mask, bool isOutput);
    protected:
    };

    // ポートを含めてGPIO全体を表すインターフェイス
    class GPIO : public interface::gpio::GPIO {
    public:
        interface::gpio::Port* getPort(uint8_t portNumber) override;
        interface::gpio::Pin* getPin(types::gpio_number_t pinNumber) override;
    private:
    };

    interface::gpio::GPIO* getGPIO(void);
    interface::gpio::Pin* getPin(types::gpio_number_t pinNumber);

} // namespace gpio

namespace bus {
namespace i2c { 

// bus::I2CBus* createBus(TwoWire& _wire);

} // namespace i2c
} // namespace bus
} // namespace arduino
} // namespace frameworks
} // namespace hal
} // namespace m5

#endif

#endif
