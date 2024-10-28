/*
 * SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
/*
  Demonstration of using M5UnitUnified with multiple units

  Required Devices:
  - Any Core with LCD
  - UnitPaHub2
  - UnitVmeter : 0
  - UnitTVOC   : 1
  - UnitENV3   : 2
  - UnitHEART  : 3
*/
#include <M5Unified.h>
#include <M5UnitUnified.h>
#include <M5UnitUnifiedHUB.h>
#include <M5UnitUnifiedENV.h>    // ENVIII,TVOC
#include <M5UnitUnifiedMETER.h>  // Vmeter
#include <M5UnitUnifiedHEART.h>  // HEART
#include <Wire.h>
#if __has_include(<esp_idf_version.h>)
#include <esp_idf_version.h>
#else  // esp_idf_version.h has been introduced in Arduino 1.0.5 (ESP-IDF3.3)
#define ESP_IDF_VERSION_VAL(major, minor, patch) ((major << 16) | (minor << 8) | (patch))
#define ESP_IDF_VERSION                          ESP_IDF_VERSION_VAL(3, 2, 0)
#endif

#include "../src/ui/ui_UnitVmeter.hpp"
#include "../src/ui/ui_UnitTVOC.hpp"
#include "../src/ui/ui_UnitENV3.hpp"
#include "../src/ui/ui_UnitHEART.hpp"

using namespace m5::unit;

namespace {
LGFX_Sprite strips[2];
constexpr uint32_t SPLIT_NUM{4};
int32_t strip_height{};
auto& lcd = M5.Display;

UnitUnified Units;
UnitPaHub2 unitPaHub{0x71};  // NEED changed register to 0x71. see also https://docs.m5stack.com/en/unit/pahub2
UnitVmeter unitVmeter;       // channel 0
UnitTVOC unitTVOC;           // channel 1
UnitENV3 unitENV3;           // channel 2
UnitHEART unitHeart;         // channel 3

auto& unitSHT30   = unitENV3.sht30;    // alias
auto& unitQMP6988 = unitENV3.qmp6988;  // alias

UnitVmeterSmallUI vmeterSmallUI(&lcd);
UnitTVOCSmallUI tvocSmallUI(&lcd);
UnitHEARTSmallUI heartSmallUI(&lcd);
UnitENV3SmallUI env3SmallUI(&lcd);

volatile SemaphoreHandle_t _updateLock{};
constexpr TickType_t ui_take_wait{0};

void prepare()
{
    // Each unit settings
    {
        auto ccfg        = unitVmeter.component_config();
        ccfg.self_update = true;  // Don't update in UnitUnified::update, update explicitly myself.
        ccfg.stored_size = 64;    // Number of elements in the circular buffer that the instance has.
        unitVmeter.component_config(ccfg);

        // Setup fro begin
        auto cfg = unitVmeter.config();
        // 12 ms is used by TVOC, so frequency is reduced
        cfg.rate = m5::unit::ads111x::Sampling::Rate64;  // 64mps
        unitVmeter.config(cfg);
    }

    {
        // Setup fro begin
        auto cfg     = unitTVOC.config();
        cfg.interval = 1000 / 10;  // 10 mps
        unitTVOC.config(cfg);

        auto ccfg        = unitTVOC.component_config();
        ccfg.self_update = true;                 // Don't update in UnitUnified::update, update explicitly myself.
        ccfg.stored_size = 1000 / cfg.interval;  // Number of elements in the circular buffer that the instance has
        unitTVOC.component_config(ccfg);
    }

    {
        auto ccfg        = unitSHT30.component_config();
        ccfg.self_update = true;
        ccfg.stored_size = 10;  // Number of elements in the circular buffer that the instance has
        unitSHT30.component_config(ccfg);

        // Setup fro begin
        auto cfg = unitSHT30.config();
        cfg.mps  = m5::unit::sht30::MPS::Ten;  // 10 mps
        unitSHT30.config(cfg);
    }
    {
        auto ccfg        = unitQMP6988.component_config();
        ccfg.self_update = true;
        ccfg.stored_size = 16;  // Number of elements in the circular buffer that the instance has
        unitQMP6988.component_config(ccfg);

        // Setup fro begin
        auto cfg = unitQMP6988.config();
        cfg.standby_time =
            m5::unit::qmp6988::Standby::Time50ms;  // about 16 mps (Calculated from other parameters and this value
        unitQMP6988.config(cfg);
    }

    {
        auto ccfg        = unitHeart.component_config();
        ccfg.self_update = true;  // Don't update in UnitUnified::update, update explicitly myself.
        ccfg.stored_size = 160;   // Number of elements in the circular buffer that the instance has
        unitHeart.component_config(ccfg);
    }

    // UI
    heartSmallUI.construct();
    tvocSmallUI.construct();
    vmeterSmallUI.construct();
    env3SmallUI.construct();
    heartSmallUI.monitor().setSamplingRate(m5::unit::max30100::getSamplingRate(unitHeart.config().sampling_rate));
}

// task for Vmeter
void update_vmeter(void*)
{
    static uint32_t fcnt{}, mps{}, mcnt{};
    static unsigned long start_at{};

    for (;;) {
        // Exclusive control of TwoWire access and unit updates
        xSemaphoreTake(_updateLock, portMAX_DELAY);
        unitVmeter.update();
        xSemaphoreGive(_updateLock);

        // If measurement data is available, acquire it and pass it to the UI.
        // If the UI is locked, skip and continue.
        if (!unitVmeter.empty()) {
            if (vmeterSmallUI.lock(ui_take_wait)) {
                mcnt += unitVmeter.available();
                while (unitVmeter.available()) {
                    vmeterSmallUI.push_back(unitVmeter.voltage());  // Gets the oldest data
                    unitVmeter.discard();                           // Discard oldest one
                }
                vmeterSmallUI.unlock();
            }
        }
        // std::this_thread::yield();
        m5::utility::delay(1);

        ++fcnt;
        auto now = m5::utility::millis();
        if (now >= start_at + 1000) {
            mps = fcnt;
            M5_LOGD("Vmeter:%u (%u)", mps, mcnt);
            fcnt = mcnt = 0;
            start_at    = now;
        }
    }
}

// Task for TVOC
void update_tvoc(void*)
{
    static uint32_t fcnt{}, mps{}, mcnt{};
    static unsigned long start_at{};

    // Waiting for SGP30 to start periodic measurement (15sec)
    for (;;) {
        if (unitTVOC.canMeasurePeriodic()) {
            break;
        }
        // Exclusive control of TwoWire access and unit updates
        xSemaphoreTake(_updateLock, portMAX_DELAY);
        unitTVOC.update();
        xSemaphoreGive(_updateLock);
        m5::utility::delay(1000);
    }

    for (;;) {
        // Exclusive control of TwoWire access and unit updates
        xSemaphoreTake(_updateLock, portMAX_DELAY);
        // TVOC measuement needs 12ms for read...
        unitTVOC.update();
        xSemaphoreGive(_updateLock);

        // If measurement data is available, acquire it and pass it to the UI.
        // If the UI is locked, skip and continue.
        if (!unitTVOC.empty()) {
            if (tvocSmallUI.lock(ui_take_wait)) {
                mcnt += unitTVOC.available();
                while (unitTVOC.available()) {
                    tvocSmallUI.push_back(unitTVOC.co2eq(), unitTVOC.tvoc());  // Gets the oldest data
                    unitTVOC.discard();                                        // Discard oldest one
                }
                tvocSmallUI.unlock();
            }
        }
        //        std::this_thread::yield();
        m5::utility::delay(1);

        ++fcnt;
        auto now = m5::utility::millis();
        if (now >= start_at + 1000) {
            mps = fcnt;
            M5_LOGD("TVOC:%u (%u)", mps, mcnt);
            fcnt = mcnt = 0;
            start_at    = now;
        }
    }
}

// Task for SHT30(ENV3)
void update_sht30(void*)
{
    static uint32_t fcnt{}, mps{}, mcnt{};
    static unsigned long start_at{};

    for (;;) {
        // Exclusive control of TwoWire access and unit updates
        xSemaphoreTake(_updateLock, portMAX_DELAY);
        unitSHT30.update();
        xSemaphoreGive(_updateLock);

        // If measurement data is available, acquire it and pass it to the UI.
        // If the UI is locked, skip and continue.
        if (!unitSHT30.empty()) {
            if (env3SmallUI.lock(ui_take_wait)) {
                mcnt += unitSHT30.available();
                auto latest = unitSHT30.latest();
                env3SmallUI.sht30_push_back(latest.temperature(), latest.humidity());  // Gets the latest data
                unitSHT30.flush();                                                     // Discard all data
                env3SmallUI.unlock();
            }
        }
        //        std::this_thread::yield();
        m5::utility::delay(1);

        ++fcnt;
        auto now = m5::utility::millis();
        if (now >= start_at + 1000) {
            mps = fcnt;
            M5_LOGD("SHT30:%u (%u)", mps, mcnt);
            fcnt = mcnt = 0;
            start_at    = now;
        }
    }
}

// Task for QMP6988(ENV3)
void update_qmp6988(void*)
{
    static uint32_t fcnt{}, mps{}, mcnt{};
    static unsigned long start_at{};

    for (;;) {
        // Exclusive control of TwoWire access and unit updates
        xSemaphoreTake(_updateLock, portMAX_DELAY);
        unitQMP6988.update();
        xSemaphoreGive(_updateLock);

        // If measurement data is available, acquire it and pass it to the UI.
        // If the UI is locked, skip and continue.
        if (!unitQMP6988.empty()) {
            if (env3SmallUI.lock(ui_take_wait)) {
                mcnt += unitQMP6988.available();
                auto latest = unitQMP6988.latest();
                env3SmallUI.qmp6988_push_back(latest.temperature(), latest.pressure());  // Gets the latest data
                unitQMP6988.flush();                                                     // Discard all data
                env3SmallUI.unlock();
            }
        }
        //        std::this_thread::yield();
        m5::utility::delay(1);

        ++fcnt;
        auto now = m5::utility::millis();
        if (now >= start_at + 1000) {
            mps = fcnt;
            M5_LOGD("QMP6988:%u (%u)", mps, mcnt);
            fcnt = mcnt = 0;
            start_at    = now;
        }
    }
}

// Task for HEART
void update_heart(void*)
{
    static uint32_t fcnt{}, mps{}, mcnt{};
    static unsigned long start_at{};

    for (;;) {
        // Exclusive control of TwoWire access and unit updates
        xSemaphoreTake(_updateLock, portMAX_DELAY);
        unitHeart.update();
        xSemaphoreGive(_updateLock);

        // If measurement data is available, acquire it and pass it to the UI.
        // If the UI is locked, skip and continue.
        if (!unitHeart.empty()) {
            if (heartSmallUI.lock(ui_take_wait)) {
                mcnt += unitHeart.available();
                while (unitHeart.available()) {
                    heartSmallUI.push_back(unitHeart.ir(), unitHeart.red());
                    unitHeart.discard();
                }
                heartSmallUI.unlock();
            }
        }
        m5::utility::delay(1);
        // std::this_thread::yield();

        ++fcnt;
        auto now = m5::utility::millis();
        if (now >= start_at + 1000) {
            mps = fcnt;
            M5_LOGD("Heart:%u (%u)", mps, mcnt);
            fcnt = mcnt = 0;
            start_at    = now;
        }
    }
}

void drawUI(LovyanGFX& dst, const uint32_t x, const uint32_t yoffset)
{
    vmeterSmallUI.push(&dst, 0, 0 + yoffset);
    tvocSmallUI.push(&dst, lcd.width() >> 1, 0 + yoffset);
    env3SmallUI.push(&dst, 0, (lcd.height() >> 1) + yoffset);
    heartSmallUI.push(&dst, lcd.width() >> 1, (lcd.height() >> 1) + yoffset);
}
}  // namespace

void setup()
{
    M5.begin();
    lcd.startWrite();
    lcd.clear(TFT_DARKGRAY);
    //
    strip_height = lcd.height() / SPLIT_NUM;
    uint32_t cnt{};
    for (auto&& spr : strips) {
        spr.setPsram(false);
        spr.setColorDepth(lcd.getColorDepth());
        cnt += spr.createSprite(lcd.width(), strip_height) ? 1 : 0;
    }
    assert(cnt == 2 && "Failed to create sprite");

    prepare();

    auto pin_num_sda = M5.getPin(m5::pin_name_t::port_a_sda);
    auto pin_num_scl = M5.getPin(m5::pin_name_t::port_a_scl);
    M5_LOGI("getPin: SDA:%u SCL:%u", pin_num_sda, pin_num_scl);
    Wire.begin(pin_num_sda, pin_num_scl, 400000U);

    if (!unitPaHub.add(unitVmeter, 0)   /* Connect Vmeter to PaHub2 ch:0 */
        || !unitPaHub.add(unitTVOC, 1)  /* Connect TVOC to PaHub2 ch:1 */
        || !unitPaHub.add(unitENV3, 2)  /* Connect ENV3 to PaHub2 ch:2 */
        || !unitPaHub.add(unitHeart, 3) /* Connect HEART to PaHub2 ch:3 */
        || !Units.add(unitPaHub, Wire)  /* Connect PaHub2 to Core */
        || !Units.begin()               /* Begin UnitUnified */
    ) {
        M5_LOGE("Failed to begin");
        lcd.clear(TFT_RED);
        while (true) {
            m5::utility::delay(10000);
        }
    }

    lcd.clear(TFT_DARKGREEN);

    M5_LOGI("M5UnitUnified has been begun");
    M5_LOGI("%s", Units.debugInfo().c_str());
    M5_LOGI("CPP %ld", __cplusplus);
    M5_LOGI("ESP-IDF Version %d.%d.%d", (ESP_IDF_VERSION >> 16) & 0xFF, (ESP_IDF_VERSION >> 8) & 0xFF,
            ESP_IDF_VERSION & 0xFF);
    M5_LOGI("BOARD:%X", M5.getBoard());
    M5_LOGI("Heap: %u", esp_get_free_heap_size());

    //
    _updateLock = xSemaphoreCreateBinary();
    xSemaphoreGive(_updateLock);
    xTaskCreateUniversal(update_vmeter, "vmeter", 8192, nullptr, 2, nullptr, PRO_CPU_NUM);
    xTaskCreateUniversal(update_tvoc, "tvoc", 8192, nullptr, 1, nullptr, PRO_CPU_NUM);
    xTaskCreateUniversal(update_sht30, "sht30", 8192, nullptr, 1, nullptr, PRO_CPU_NUM);
    xTaskCreateUniversal(update_qmp6988, "qmp6988", 8192, nullptr, 1, nullptr, PRO_CPU_NUM);
    xTaskCreateUniversal(update_heart, "heart", 8192, nullptr, 1, nullptr, PRO_CPU_NUM);
}

void loop()
{
    static uint32_t fpsCnt{}, fps{};
    static unsigned long start_at{};

    ++fpsCnt;
    auto now = m5::utility::millis();
    if (now >= start_at + 1000) {
        fps = fpsCnt;
        M5_LOGD("FPS:%u", fps);
        fpsCnt   = 0;
        start_at = now;
    }

    M5.update();

    // All units do their own updates, so there is no need to call for a unit-wide update here.
    // xSemaphoreTake(_updateLock, portMAX_DELAY);
    // unitTVOC.update();
    // xSemaphoreGive(_updateLock);

    tvocSmallUI.update();
    vmeterSmallUI.update();
    heartSmallUI.update();
    env3SmallUI.update();

    static uint32_t current{};
    int32_t offset{};
    uint32_t cnt{SPLIT_NUM};
    while (cnt--) {
        auto& spr = strips[current];
        spr.clear();
        drawUI(spr, 0, offset);
        spr.pushSprite(&lcd, 0, -offset);
        current ^= 1;
        offset -= strip_height;
    }
}
