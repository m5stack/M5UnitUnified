# M5UnitUnified

Work in progress  

 The operation of TwoWire-based and experimental M5HAL-based components can be checked.  

The final implementation will only use the M5HAL, but it has also been made to work on a TwoWire-based, so that development and testing can proceed without reliance on the M5HAL.

## For development and checking convenience, including those that are inherently separate repositories

### New repositories to be split in the future.

- lib/M5Utitliy  
M5UnitUnified, M5UnitComponent, M5HAL, and other depends it.
- lib/M5UnitComponent  
M5UnitUnified depends it.
- lib/M5PaHub  
For PaHub
- lib/M5HAL  
M5UnitComponent depends it.

### Add to existing libraries in the future

- lib/M5Unit-ENV  
UnitComponent for SCD40 and SCD41

## How to try examples (PlatformIO only)
- env:example\_SCD40\_foo

If you want to try it on other devices, edit the ini accordingly.  
**lib Once the repositories for each of the libraries below are in place, they can be compiled by ArduinoIDE.**

### behavioural change

[examples/SCD4x/SCD4x.cpp](examples/SCD4x/SCD4x.cpp )

```cpp
#define USING_PAHUB (2) // Connection channel number for use via PaHub.
#define USING_M5HAL  // When using M5HAL
```
- Switch between TwoWire and M5HAL versions with and without USING_M5HAL defines
- Switch between directly connected and via PaHub versions with and without USING_PAHUB defines  
**Connecting the SCD40 to the USING_PAHUB numbered channel.**



## How to try UnitTest (PlatformIO only)
- env:test_foo  
[Advanced]_ - [Test]


## API Documentation 
doxygen and pcregrep required
```.sh
bash docs/doxy.sh
open docs/html/index.html
```

By adding to existing libraries, the reference manual can be developed from the source.  
It would be useful to use GitHub Action to automatically generate documentation and publish it on GitHub Pages or m5doc upon master release.

## UnitTest
Create unit tests by googletest that are native (if possible) and embedded in each units.  
This is necessary to check operation, isolate problems when they occur and prevent regressions.

The M5UnitUnified test checks for collisions between the unique IDs assigned to the units.  
Unit-specific tests will be placed in the respective library repositories.
(Some working test are currently placed in examples for development convenience. Will be moved before release).


### Examples
The example of the UnitUnified itself depends on the connecting unit and will be added to the example in each unit's library.  
(Some working examples are currently placed in examples for development convenience. Will be moved before release).


## Questions
### https://github.com/m5stack/M5Template-C-CPP/

.clang-format is much older.  
There are items that have been changed in the current version, etc.  
Prerequisite version is 13?  
What about updating to the latest settings?  
The number of column digits is 80, can it be made larger?


---
---
---
## Overview

### SKU:xxx

Description of the product

## Related Link

- [Document & Datasheet](https://docs.m5stack.com/en/unit/product_Link)

## Required Libraries:

- [Adafruit_BMP280_Library](https://github.com/adafruit/Required_Libraries_Link)

## License

- [Product Name- MIT](LICENSE)

## Remaining steps(Editorial Staff Look,After following the steps, remember to delete all the content below)

1. Change [clang format check path](./.github/workflows/clang-format-check.yml#L9-L15).
2. Add License content to [LICENSE](/LICENSE).
3. Change link on line 78 of [bug-report.yml](./.github/ISSUE_TEMPLATE/bug-report.yml#L78).

```cpp
Example
# M5Unit-ENV

## Overview

### SKU:U001 & U001-B & U001-C

Contains M5Stack-**UNIT ENV** series related case programs.ENV is an environmental sensor with integrated SHT30 and QMP6988 internally to detect temperature, humidity, and atmospheric pressure data.

## Related Link

- [Document & Datasheet](https://docs.m5stack.com/en/unit/envIII)

## Required Libraries:

- [Adafruit_BMP280_Library](https://github.com/adafruit/Adafruit_BMP280_Library)

## License

- [M5Unit-ENV - MIT](LICENSE)
```
