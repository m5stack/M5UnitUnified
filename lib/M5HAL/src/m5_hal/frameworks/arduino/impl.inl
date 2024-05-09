#ifndef M5_HAL_PLATFORMS_ARDUINO_IMPL_INL
#define M5_HAL_PLATFORMS_ARDUINO_IMPL_INL

#include "../src/m5_hal/bus/bus.hpp"
#include <M5Utility.hpp>

#if __has_include(<Arduino.h>)
#include <Arduino.h>
#include <Wire.h>
#endif

#include <vector>
#include <list>

#if defined( ARDUINO )

namespace m5 {
namespace hal {
namespace frameworks {
namespace arduino {
namespace gpio {

interface::gpio::GPIO* getGPIO(void)
{
M5_LIB_LOGV("getGPIO\n");
    static GPIO s_gpio_instance;
    return &s_gpio_instance;
}

interface::gpio::Pin* getPin(types::gpio_number_t pinNumber)
{
M5_LIB_LOGV("getPin %d", pinNumber);
    return getGPIO()->getPin(pinNumber);
}

interface::gpio::Port* GPIO::getPort(uint8_t portNumber)
{
    static Port s_port_instance;
    return &s_port_instance;
}

interface::gpio::Pin* GPIO::getPin(types::gpio_number_t pinNumber)
{
M5_LIB_LOGV("GPIO::getPin %d", pinNumber);
    return getPort(0)->getPin(pinNumber);
}

interface::gpio::Pin* Port::getPin(uint8_t pinNumber) {
    static std::vector<Pin*> s_pins_pointer;
    static std::list<Pin> s_pins_instance;
    if (s_pins_pointer.size() <= pinNumber) {
        s_pins_pointer.resize(pinNumber + 1);
    }
    if (s_pins_pointer[pinNumber] == nullptr) {
        s_pins_instance.push_back(Pin(pinNumber));
        s_pins_pointer[pinNumber] = &s_pins_instance.back();
    }
    return s_pins_pointer[pinNumber];
}

void Pin::write(bool value)
{
    // M5_LIB_LOGV("Pin::write %d  [pin %d]", value, _gpio_number);
    digitalWrite(_gpio_number, value);
}
bool Pin::read(void) { return digitalRead(_gpio_number); }
void Pin::writeHigh(void) {
    // M5_LIB_LOGV("Pin::setHigh  [pin %d]", _gpio_number);
    digitalWrite(_gpio_number, true);
}
void Pin::writeLow(void) {
    // M5_LIB_LOGV("Pin::setLow  [pin %d]", _gpio_number);
    digitalWrite(_gpio_number, false);
}

void Pin::setMode(types::gpio_mode_t mode)
{
    switch (mode) {
    case types::gpio_mode_t::Input:
        pinMode(_gpio_number, INPUT);
        break;
    case types::gpio_mode_t::Output:
        pinMode(_gpio_number, OUTPUT);
        break;
    case types::gpio_mode_t::Output_OpenDrain:
        pinMode(_gpio_number, OUTPUT_OPEN_DRAIN);
        break;
    default:
        break;
    }
}

} // namespace gpio

namespace bus {
namespace i2c { 
// interface::bus::I2C* createBus(TwoWire& _wire)
// {
//     return nullptr;
// }
} // namespace i2c
} // namespace bus

//

} // namespace arduino
} // namespace frameworks
} // namespace hal
} // namespace m5

#endif

#endif
