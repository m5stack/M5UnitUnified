
#ifndef M5_HAL_INTERFACE_GPIO_HPP
#define M5_HAL_INTERFACE_GPIO_HPP

#include "../types.hpp"
#include <vector>

/*!
  @namespace m5
  @brief Toplevel namespace of M5
 */
namespace m5 { namespace hal { namespace interface {
// 抽象化されたGPIOインターフェースを置く。これは LowLayerではなく汎用的なもの。
// このGPIOインターフェースを継承してGPIOエキスパンダの実装を作ることもできる。
namespace gpio {

// 単一のデジタルピンを表すインターフェイス
struct Pin {
    virtual ~Pin() = default;
    virtual void write(bool value) = 0;
    virtual bool read(void) = 0;
    virtual void writeHigh(void) { write(true); }
    virtual void writeLow(void) { write(false); }

    virtual types::gpio_number_t getGpioNumber(void) const = 0;
    virtual void setMode(types::gpio_mode_t mode) = 0;
};

// 複数のPinを保持する概念を表すインターフェイス
struct Port {
    virtual ~Port() = default;
    virtual Pin* getPin(uint8_t pinNumber) = 0;
    // virtual uint8_t getPortNumber(void) const = 0;
};

// Portを複数含むGPIO全体を表すインターフェイス
struct GPIO {
    virtual ~GPIO() = default;
    virtual Port* getPort(uint8_t portNumber) = 0;
    virtual Pin* getPin(types::gpio_number_t gpioNumber) = 0;
    virtual void digitalWrite(types::gpio_number_t gpioNumber, bool value) {
        getPin(gpioNumber)->write(value);
    }
    virtual bool digitalRead(types::gpio_number_t gpioNumber) {
        return getPin(gpioNumber)->read();
    }
    virtual int analogRead(types::gpio_number_t gpioNumber) {
        return -1;
    }
};


// n個のピンを内包するポート (例: 2個ならGROVEコネクタ、等)
// コンストラクタにてピンの数を指定する。生成後のサイズ変更は不可とする。
// 暫定的に std::vector でポインタを保持するが、将来的にはメモリ効率を考慮して変更する可能性あり。
class MultiPinPort : public Port {
public:
    MultiPinPort(size_t size) : _pins { size, nullptr } {}
    Pin* getPin(uint8_t pinNumber) override {
        return _pins.size() > pinNumber ? _pins[pinNumber] : nullptr;
    }
    void setPin(uint8_t pinNumber, Pin* pin) {
        if (_pins.size() > pinNumber) _pins[pinNumber] = pin;
    }
    size_t getPinCount(void) const { return _pins.size(); }
protected:
    std::vector<Pin*> _pins;
};

} // namespace gpio
} // namespace interface
} // namespace hal
} // namespace m5

#endif
