# M5UnitUnified(α release)

[日本語](README.ja.md)

**A new approach to connect and handle various M5 units in the M5Stack**  
Library for M5Stack Series and M5Unit Series

## Overview
M5UnitUnified is a library for unified handling of various M5 units products.

### Unified APIs
Each unit's external library has its own API design.  
Unify basic APIs so that all units can be handled in the same way.

### Unified connections and communications
Each unit's external library requires its own communication functions and assumptions.  
Unify prerequisites and communication methods.  
In the future, we plan to work with [M5HAL (Hardware Abstraction Layer)](https://github.com/m5stack/M5HAL) to unified communicatation  with each unit.

### Unified Licensing
External library licenses for each unit are mixed.  
All M5UnitUnified and related libraries are under the [MIT license](LICENSE).


## How to install
The library is registered with the library manager.

### Arduino IDE

1. Using library manager and select the library of the unit you want to use (e.g. M5Unit-GESTURE)

Dependent M5UnitUnfied related libraries will be downloaded automatically.

### PlatformIO
1. Write lib\_deps settings to platformio.ini
```ini
lib_deps= m5stack/M5Unit-foo ; Unit to be used
```
Dependent M5UnitUnfied related libraries will be downloaded automatically.


## How to use

See also examples for each unit repositry too.

### UnitComponent with UnitUnified (Standard usage)

#### Unit using Wire
```cpp
// If you use other units, change include files(*1), instances(*2), and get values(*3)
#include <M5Unified.h>
#include <M5UnitUnified.h>
#include <M5UnitUnifiedENV.h>  // *1 Include the header of the unit to be used

m5::unit::UnitUnified Units;
m5::unit::UnitCO2 unit;  // *2 Instance of the unit

void setup() {
    M5.begin();

    auto pin_num_sda = M5.getPin(m5::pin_name_t::port_a_sda);
    auto pin_num_scl = M5.getPin(m5::pin_name_t::port_a_scl);
    M5_LOGI("getPin: SDA:%u SCL:%u", pin_num_sda, pin_num_scl);
    Wire.end();
    Wire.begin(pin_num_sda, pin_num_scl, 400 * 1000U);

    if (!Units.add(unit, Wire)  // Add unit to UnitUnified manager
        || !Units.begin()) {    // Begin each unit
        M5_LOGE("Failed to add/begin");
    }
}

void loop() {
    M5.update();
    Units.update();
    if (unit.updated()) {
        // *3 Obtaining unit-specific measurements
        M5.Log.printf("CO2:%u Temp:%f Hum:%f\n", unit.co2(), unit.temperature(), unit.humidity());
    }
}
```

#### Unit using GPIO

```cpp
// If you use other units, change include files(*1), instances(*2), and get values(*3)
#include <M5Unified.h>
#include <M5UnitUnified.h>
#include <M5UnitUnifiedTUBE.h> // *1 Include the header of the unit to be used

m5::unit::UnitUnified Units;
m5::unit::UnitTubePressure unit; // *2 Instance of the unit

void setup()
{
    M5.begin();

    // PortB if available, PortA if not
    auto pin_num_gpio_in  = M5.getPin(m5::pin_name_t::port_b_in);
    auto pin_num_gpio_out = M5.getPin(m5::pin_name_t::port_b_out);
    if (pin_num_gpio_in < 0 || pin_num_gpio_out < 0) {
        M5_LOGW("PortB is not available");
        Wire.end();
        pin_num_gpio_in  = M5.getPin(m5::pin_name_t::port_a_pin1);
        pin_num_gpio_out = M5.getPin(m5::pin_name_t::port_a_pin2);
    }

    if (!Units.add(unit, pin_num_gpio_in, pin_num_gpio_out) // Add unit to UnitUnified manager
        || !Units.begin()) { // Begin each unit
        M5_LOGE("Failed to add/begin");
    }
}

void loop()
{
    M5.update();
    Units.update();
    if (unit.updated()) {
        // *3 Obtaining unit-specific measurements
        M5.Log.printf("Pressure:%.2f\n", unit.pressure());
    }
}
```

#### Unit using UART(Serial)

```cpp
// If you use other units, change include files(*1), instances(*2), and call any API(*3)
#include <M5Unified.h>
#include <M5UnitUnified.h>
#include <M5UnitUnifiedFINGER.h> // *1 Include the header of the unit to be used

m5::unit::UnitUnified Units;
m5::unit::UnitFinger unit; // *2 Instance of the unit

void setup()
{
    M5.begin();

    // PortC if available, PortA if not
    auto pin_num_in  = M5.getPin(m5::pin_name_t::port_c_rxd);
    auto pin_num_out = M5.getPin(m5::pin_name_t::port_c_txd);
    if (pin_num_in < 0 || pin_num_out < 0) {
        M5_LOGW("PortC is not available");
        Wire.end();
        pin_num_in  = M5.getPin(m5::pin_name_t::port_a_pin1);
        pin_num_out = M5.getPin(m5::pin_name_t::port_a_pin2);
    }

#if SOC_UART_NUM > 2
    auto& s = Serial2;
#elif SOC_UART_NUM > 1
    auto& s = Serial1;
#else
#error "Not enough Serial"
#endif
    s.end();
    // Note that the argument varies depending on the target unit
    s.begin(19200, SERIAL_8N1, pin_num_in, pin_num_out);

    if (!Units.add(unit, s) // Add unit to UnitUnified manager
        || !Units.begin()) { // Begin each unit
        M5_LOGE("Failed to begin");
    }
}

void loop() {
    M5.update();
    Units.update();
    // *3 Arbitrary API calls to the unit...
}

```

#### Unit using SPI

```cpp
// If you use other units, change include files(*1), instances(*2), and call any API(*3)
#include <M5Unified.h>
#include <M5UnitUnified.h>
#include <M5UnitUnifiedFoo.h> // *1 Include the header of the unit to be used

m5::unit::UnitUnified Units;
m5::unit::UnitFoo unit; // *2 Instance of the unit

void setup()
{
    M5.begin();

    if (!SPI.bus()) {
        auto spi_sclk = M5.getPin(m5::pin_name_t::sd_spi_sclk);
        auto spi_mosi = M5.getPin(m5::pin_name_t::sd_spi_mosi);
        auto spi_miso = M5.getPin(m5::pin_name_t::sd_spi_miso);
        M5_LOGI("getPin: %d,%d,%d", spi_sclk, spi_mosi, spi_miso);
        SPI.begin(spi_sclk, spi_miso, spi_mosi);
    }

    // Note that the depending on the target unit
    SPISettings settings = {10000000, MSBFIRST, SPI_MODE1};
    if (!Units.add(cap, SPI, settings) || !Units.begin()) {
        M5_LOGE("Failed to begin");
        lcd.fillScreen(TFT_RED);
        while (true) {
            m5::utility::delay(10000);
        }
    }
}

void loop() {
    M5.update();
    Units.update();
    // *3 Arbitrary API calls to the unit...
}
```

- Nonstandard usage
  - [To update the unit yourself usage example](examples/Basic/SelfUpdate)
  - [Using only unit component without UnitUnified manager](examples/Basic/ComponentOnly)

## Supported things
### Supported frameworks
- Arduino

Support ESP-IDF with M5HAL in the future.

### Supported connection
- I2C with TwoWire class
- GPIO (Currently only functions required for the units are included)
- UART with HardwareSerial class
- SPI with SPI class

### Supported devices, units
See also [Wiki](https://github.com/m5stack/M5UnitUnified/wiki/)

## Examples
For exampless of each unit, please refer to the respective unit's repository.  
[The examples in this repository](examples/Basic) are for M5UnitUnified in general

## Doxygen document

[GitHub Pages](https://m5stack.github.io/M5UnitUnified/)

If you want to generate documents on your local machine, execute the following command

```
bash docs/doxy.sh
```

It will output it under docs/html  
If you want to output Git commit hashes to html, do it for the git cloned folder.

### Required
- [Doxygen](https://www.doxygen.nl/)
- [pcregrep](https://formulae.brew.sh/formula/pcre2)
- [Git](https://git-scm.com/) (Output commit hash to html)

