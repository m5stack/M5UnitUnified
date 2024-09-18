# M5UnitUnified(α release)

[日本語](README.ja.md)

**A new approach to connect and handle various M5 units in the M5Stack**  
Library for M5Stack Series and M5Unit Series

**Notice: Now α version**  
Please send your comments and requests to Issue or PR.

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
Registration to Arduino/PlatformIO librarry manager is scheduled after the beta version.  
Until then, please install manually.

### Arduino IDE
1. Download ZIP files from each repositories  
- M5UnitUnified
- [M5Utility](https://github.com/m5stack/M5Utility/)
- [M5HAL](https://github.com/m5stack/M5HAL)
- [Unit to be used](#supported-units)
2. Extract the ZIP file to your ArduinoIDE library folder

### PlatformIO
1. Write lib_deps settings to platformio.ini
```ini
lib_deps= https://github.com/m5stack/M5UnitUnified
  https://github.com/m5stack/M5Utility
  https://github.com/m5stack/M5HAL
  https://github.com/m5stack/M5Unit-foo ; Unit to be used
```
## How to use

See also examples for each unit repositry too.

### UnitComponent with UnitUnified (Standard usage)

Simple example of the UnitCO2  
UnitCO2 is started with default settings in Units.begin(), and loop() print logs measurement data.

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
    Wire.begin(pin_num_sda, pin_num_scl, 400 * 1000U);

    M5.Display.clear(TFT_DARKGREEN);
    if (!Units.add(unit, Wire)  // Add unit to UnitUnified manager
        || !Units.begin()) {    // Begin each unit
        M5_LOGE("Failed to add/begin");
        M5.Display.clear(TFT_RED);
    }
}

void loop() {
    M5.begin();
    Units.update();
    if (unit.updated()) {
        // *3 Obtaining unit-specific measurements
        M5_LOGI("CO2:%u Temp:%f Hum:%f", unit.co2(), unit.temperature(), unit.humidity());
    }
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
- I2C with TwoWire

Support GPIO, UART in the future.

### Supported core device (Operation confirmed)

- M5Stack Core (BASIC / GRAY / FIRE)
- M5Stack Core2
- M5Stick CPlus
- M5Paper
- M5Stack CoreS3 CoreS3SE
- M5ATOMS3
- M5STAMPS3
- M5Dial
- M5Capsule
- M5NanoC6

Other devices will be added after confirmation of operation.

### Supported units 

#### Operation confirmed
- [UnitCO2](https://github.com/m5stack/M5Unit-ENV)
- [UnitENVIII](https://github.com/m5stack/M5Unit-ENV)
- [UnitAmeter](https://github.com/m5stack/M5Unit-METER)
- [UnitVmeter](https://github.com/m5stack/M5Unit-METER)
- [UnitPaHub2](https://github.com/m5stack/M5Unit-HUB)
- [UnitGESTURE](https://github.com/m5stack/M5Unit-GESTURE)
- [UnitHEART](https://github.com/m5stack/M5Unit-HEART)
- [UnitKmeterISO](https://github.com/m5stack/M5Unit-KMeterISO)
- [UnitTVOC](https://github.com/m5stack/M5Unit-TVOC)

#### There are a few problems
- [UnitENVPro]((https://github.com/m5stack/M5Unit-ENV))

Other units will be added after they are developed and tested.


## Doxygen document
If you want to generate documents on your local machine, execute the following command

```
bash docs/doxy.sh
```

It will output it under docs/html  
If you want to output Git commit hashes to html, do it for the git cloned folder.

### Required
- [Doxyegn](https://www.doxygen.nl/)
- [pcregrep](https://formulae.brew.sh/formula/pcre2)
- [Git](https://git-scm.com/) (Output commit hash to html)

