/*
 * SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
/*
  Example: UnitKmeterISO
*/

// #define USING_M5HAL  // When using M5HAL

#include <M5Unified.h>
#include <M5UnitUnified.h>
#include <unit/unit_KmeterISO.hpp>
#if !defined(USING_M5HAL)
#include <Wire.h>
#endif

namespace {
auto& display = M5.Display;
M5Canvas canvas[2];

m5::unit::UnitUnified Units;
m5::unit::UnitKmeterISO unit;

// M5_KMeter::error_code_t* errdata_buf;
constexpr size_t avg_count  = 1 << 5;
constexpr size_t delay_msec = 50;
float* tempdata_buf;
size_t tempdata_count;
size_t tempdata_idx = 0;
int graph_height;
int graph_y_offset;
float min_temp;
float max_temp;
float avg_buf[avg_count];
size_t avg_index = 0;
int vertline_idx = 0;
}  // namespace

void setup() {
    M5.begin();

    m5::utility::delay(3000);

    auto pin_num_sda = M5.getPin(m5::pin_name_t::port_a_sda);
    auto pin_num_scl = M5.getPin(m5::pin_name_t::port_a_scl);
    M5_LOGI("getPin: SDA:%u SCL:%u", pin_num_sda, pin_num_scl);

#if defined(USING_M5HAL)
#pragma message "Using M5HAL"
    // Using M5HAL
    m5::hal::bus::I2CBusConfig i2c_cfg;
    i2c_cfg.pin_sda = m5::hal::gpio::getPin(pin_num_sda);
    i2c_cfg.pin_scl = m5::hal::gpio::getPin(pin_num_scl);
    auto i2c_bus    = m5::hal::bus::i2c::getBus(i2c_cfg);
    M5_LOGI("Bus:%d", i2c_bus.has_value());
    if (!Units.add(unit, i2c_bus ? i2c_bus.value() : nullptr) ||
        !Units.begin()) {
        M5_LOGE("Failed to begin");
        display.clear(TFT_RED);
        while (true) {
            m5::utility::delay(10000);
        }
    }
#else
#pragma message "Using Wire"
    // Using TwoWire
    Wire.begin(pin_num_sda, pin_num_scl, 100000U);
    for (int i = 0; i < 10; ++i) {
        Wire.beginTransmission(unit.address());
        auto wret = Wire.endTransmission();
        M5_LOGW(">>%d", wret);
        delay(10);
    }

    if (!Units.add(unit, Wire) || !Units.begin()) {
        M5_LOGE("Failed to begin");
        display.clear(TFT_RED);
        while (true) {
            m5::utility::delay(10000);
        }
    }
#endif

    M5_LOGI("M5UnitUnified has been begun");
    M5_LOGI("%s", Units.debugInfo().c_str());
    display.clear(TFT_DARKGREEN);

    //
    display.clear();
    display.setEpdMode(epd_mode_t::epd_fast);
    if (display.width() > display.height()) {
        display.setRotation(display.getRotation() ^ 1);
    }
    display.setFont(&fonts::Font4);
    display.setTextColor((uint32_t)~display.getBaseColor(),
                         display.getBaseColor());
    display.setTextDatum(textdatum_t::top_right);
    display.setTextPadding(display.textWidth("1888.88", &fonts::Font4));
    graph_y_offset = display.fontHeight(&fonts::Font4);

    graph_height = display.height() - graph_y_offset;

    for (int i = 0; i < 2; ++i) {
        canvas[i].setColorDepth(display.getColorDepth());
        canvas[i].createSprite(1, graph_height);
        canvas[i].setTextDatum(textdatum_t::bottom_right);
        canvas[i].setTextColor(TFT_LIGHTGRAY);
    }

    tempdata_count = display.width() + 1;
    tempdata_buf   = (float*)malloc(tempdata_count * sizeof(float));

    int32_t tmp{};
    float temperature = unit.readCelsiusTemperature(tmp)
                            ? m5::unit::UnitKmeterISO::conversion(tmp)
                            : 0.0f;
    min_temp = max_temp = temperature;
    for (size_t i = 0; i < tempdata_count; ++i) {
        tempdata_buf[i] = temperature;
    }
    for (size_t i = 0; i < avg_count; ++i) {
        avg_buf[i] = temperature;
    }
}

void drawGraph(void) {
    float min_t = INT16_MAX;
    float max_t = INT16_MIN;
    for (int i = 0; i < tempdata_count; ++i) {
        float t = tempdata_buf[i];
        if (min_t > t) {
            min_t = t;
        }
        if (max_t < t) {
            max_t = t;
        }
    }
    min_temp = (min_temp + (min_t - 0.5f)) / 2;
    max_temp = (max_temp + (max_t + 0.5f)) / 2;

    float magnify = (float)graph_height / (max_temp - min_temp);

    static constexpr int steps[] = {1,  2,   5,   10,  20,
                                    50, 100, 200, 500, INT_MAX};
    int step_index               = 0;
    while (magnify * steps[step_index] < 10) {
        ++step_index;
    }
    int step  = steps[step_index];
    bool flip = 0;

    canvas[flip].clear(display.getBaseColor());

    int gauge = ((int)min_temp / step) * step;
    do {
        canvas[flip].drawPixel(0, graph_height - ((gauge - min_temp) * magnify),
                               TFT_DARKGRAY);
    } while (max_temp > (gauge += step));

    size_t buffer_len = canvas[flip].bufferLength();
    auto buffer       = (uint8_t*)alloca(buffer_len);
    memcpy(buffer, canvas[flip].getBuffer(), buffer_len);

    int drawindex = tempdata_idx;
    int draw_x    = -1;
    int y0        = 0;

    if (++vertline_idx >= 20) {
        vertline_idx = 0;
    }
    int vidx = vertline_idx;

    display.startWrite();
    do {
        if (++vidx >= 20) {
            vidx = 0;
            memset(canvas[flip].getBuffer(), 0x55, buffer_len);
        } else {
            memcpy(canvas[flip].getBuffer(), buffer, buffer_len);
        }

        int y1 = y0;
        y0 = graph_height - ((tempdata_buf[drawindex] - min_temp) * magnify);
        if (++drawindex >= tempdata_count) {
            drawindex = 0;
        }

        if (display.width() - draw_x < 24) {
            gauge = ((int)min_temp / step) * step;
            do {
                canvas[flip].drawNumber(
                    (int)gauge, display.width() - draw_x,
                    graph_height - ((gauge - min_temp) * magnify));
            } while (max_temp > (gauge += step));
        }

        canvas[flip].setColor(~display.getBaseColor());
        //                                  : 0xFF0000u);
        int y_min = y0, y_max = y1;
        if (y_min > y_max) {
            std::swap(y_min, y_max);
        }
        canvas[flip].drawFastVLine(0, y_min, y_max - y_min + 1);
        canvas[flip].pushSprite(&display, draw_x, graph_y_offset);
        flip = !flip;
    } while (++draw_x < display.width());
    display.endWrite();
}

void loop() {
    M5.update();
    Units.update();
    if (M5.BtnA.wasClicked()) {
    }

    if (unit.updated()) {
        // M5_LOGI("\n>Current:%f", std::fabs(raw) * correction);
    }

    float temperature = unit.celsiusTemperature();

    avg_buf[avg_index] = temperature;
    if (++avg_index >= avg_count) {
        avg_index = 0;
    }
    float avg_temp = 0;
    for (size_t i = 0; i < avg_count; ++i) {
        size_t k = abs((int)(i - avg_index) * 2 + 1);
        if (k > avg_count) {
            k = avg_count * 2 - k;
        }
        avg_temp += avg_buf[i] * k / avg_count;
    }

    tempdata_buf[tempdata_idx] = 2 * avg_temp / avg_count;
    if (++tempdata_idx >= tempdata_count) {
        tempdata_idx = 0;
    }

    drawGraph();

    static uint32_t prev_msec;
    uint32_t msec = millis();
    if (msec - prev_msec < delay_msec) {
        M5_LOGI("\n>Temp:%6.2f", temperature);
        m5gfx::delay(delay_msec - (msec - prev_msec));
    }
    prev_msec = msec;
}
