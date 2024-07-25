/*
  CO2 Unit (SCD40)
 */

#include <M5Unified.h>
#include <M5HAL.hpp>
#include <M5Utility.hpp>
#include <M5UnitUnified.hpp>
#include <unit/unit_SCD4x.hpp>
#include <unit/unit_PaHub.hpp>
#if defined(ARDUINO)
#include <esp_system.h>
#include <WiFi.h>
#include <esp_sntp.h>
#endif
#include <cmath>
#include <ctime>
#include <tuple>
#include <random>
#include "sprite2.hpp"


#define USING_BUS


namespace {
auto rng  = std::default_random_engine{};
auto& lcd = M5.Display;

#if defined(ARDUINO)
// Configrate time
constexpr char ntp0[]     = "ntp.nict.jp";
constexpr char ntp1[]     = "ntp.jst.mfeed.ad.jp";
constexpr char ntp2[]     = "time.cloudflare.com";
const char* ntpURLTable[] = {
    ntp0, ntp1,
    ntp2};  // DON'T USE PROGMEM! (because it will be shuffled later)
constexpr char posixTZ[] = "JST-9";  // Asia/Tokyo

void configTime() {
    // Check RTC if exists
    if (M5.Rtc.isEnabled()) {
        auto dt = M5.Rtc.getDateTime();  // GMT
        if (dt.date.year > 2016) {
            M5_LOGI("RTC time already set. (GMT) %04d/%02d/%2d %02d:%02d:%02d",
                    dt.date.year, dt.date.month, dt.date.date, dt.time.hours,
                    dt.time.minutes, dt.time.seconds);

            // Set timezone
            setenv("TZ", posixTZ, 1);
            tzset();
            return;
        }
    }

    M5_LOGI("Configrate time");

    // WiFi connect
    // Connect to credential in Hardware. (ESP32 saves the last WiFi connection)
    // WiFi.begin("SSID","PASSWORD");
    WiFi.begin();
    int32_t retry{10};
    while (WiFi.status() != WL_CONNECTED && --retry >= 0) {
        M5.Display.printf(".");
        delay(1000);
    }
    if (WiFi.status() != WL_CONNECTED) {
        M5.Display.clear(TFT_RED);
        M5.Display.setCursor(0, lcd.height() >> 1);
        M5.Display.printf("Failed to connect WiFi");
        do {
            delay(1000);
        } while (true);
    }

    std::shuffle(std::begin(ntpURLTable), std::end(ntpURLTable), rng);
    configTzTime(posixTZ, ntpURLTable[0], ntpURLTable[1], ntpURLTable[2]);

    // waiting for time synchronization
    retry = 10;
    sntp_sync_status_t st;
    while (((st = sntp_get_sync_status()) == SNTP_SYNC_STATUS_RESET) &&
           --retry >= 0) {
        M5_LOGI("   ...sync in progress");
        delay(1000);
    }
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);

    {
        std::tm discard{};
        if ((st != SNTP_SYNC_STATUS_COMPLETED) ||
            !getLocalTime(&discard, 10 * 1000 /* timeout */)) {
            M5.Display.clear(TFT_BLUE);
            M5.Display.setCursor(0, lcd.height() >> 1);
            M5.Display.printf("Failed to configTime");
            M5_LOGE("Failed to configTime");
            do {
                delay(1000);
            } while (true);
        }
    }

    // Set RTC if exists
    if (M5.Rtc.isEnabled()) {
        time_t t = time(nullptr) + 1;
        while (t > time(nullptr));
        M5.Rtc.setDateTime(std::gmtime(&t));
    }
}
#endif

m5::unit::UnitUnified Units;
m5::unit::UnitSCD40 unitSCD40;
m5::unit::UnitPaHub unitPaHub;

LGFX_Sprite bg;
LGFX_Sprite arrow, arrow2;
Sprite number, number2, number3;
Sprite watch, meter, meter2;

constexpr int16_t strip_div{3};
int16_t strip_height{};
LGFX_Sprite strip[2];

struct Linear {
    float operator()(const float t) const {
        return t;
    }
};

struct InBack {
    constexpr static float BackFactor = 1.70158f;
    float operator()(const float t) const {
        return 1.0f * t * t * ((BackFactor + 1.0f) * t - BackFactor);
    }
};

struct OutBack {
    constexpr static float BackFactor = 1.70158f;
    float operator()(const float t) const {
        return 1.0f * ((t - 1.0f) * (t - 1.0f) *
                           ((BackFactor + 1.0f) * (t - 1.0f) + BackFactor) +
                       1.0f);
    }
};

struct InOutBack {
    constexpr static float BackFactor  = 1.70158f;
    constexpr static float BackFactor2 = BackFactor * 1.525f;
    float operator()(const float t) const {
        return (t * 2.0f) < 1.0f
                   ? 0.5f * ((t * 2.0f) * (t * 2.0f) *
                             ((BackFactor2 + 1.0f) * (t * 2.0f) - BackFactor2))
                   : 0.5f * ((t * 2.0f - 2.0f) * (t * 2.0f - 2.0f) *
                                 ((BackFactor2 + 1.0f) * (t * 2.0f - 2.0f) +
                                  BackFactor2) +
                             2.0f);
    }
};

template <typename T, class Func = Linear>
class Ease {
   public:
    Ease(const unsigned long duration, const T& init = T(0))
        : _duration(duration), _from(init), _to(init), _value(init) {
    }

    T value() {
        return _value;
    }

    void to(const T& v, const unsigned long ms,
            const unsigned long duration = 0) {
        if (_to != v) {
            if (duration) {
                _duration = duration;
            }
            _from     = _value;
            _to       = v;
            _start_at = ms;
        }
    }

    bool update(const unsigned long ms) {
        if (ms >= _start_at + _duration) {
            _value = _to;
            return true;
        }
        float t = (ms - _start_at) / (float)_duration;
        t       = _func(t);
        _value  = (_from * (1.0f - t)) + (_to * t);
        return false;
    }

   private:
    Func _func;
    unsigned long _duration{};
    T _from{}, _to{}, _value{};
    unsigned long _start_at{};
};

Ease<float, OutBack> left(1000, 135.0f);
Ease<float> right(1000, 45.0f);

}  // namespace

std::pair<int16_t, int16_t> rotate(const int16_t x, const int16_t y,
                                   const int16_t cx, const int16_t cy,
                                   const float deg) {
    float xx  = x - cx;
    float yy  = y - cy;
    float rad = deg * M_PI / 180.f;
    return {(xx * std::cos(rad) - yy * std::sin(rad)) + cx,
            (xx * std::sin(rad) + yy * std::cos(rad)) + cy};
}

std::pair<int16_t, int16_t> polar(const int16_t cx, const int16_t cy,
                                  const int16_t radius, const float deg) {
    float rad = deg * M_PI / 180.f;
    return {(int16_t)(cx + radius * std::cos(rad)),
            (int16_t)(cy - radius * std::sin(rad))};
}

void renderWhiteGauge(LGFX_Sprite* gfx, int16_t yoff, const unsigned long ms) {
    if (!yoff && left.update(ms)) {
        int32_t i = rng() % 40;
        int32_t d = rng() % 600 + 1000;
        left.to(95.f + i, ms, d);
    }
    if (!yoff && right.update(ms)) {
        int32_t i = rng() % 50;
        int32_t d = rng() % 700 + 1000;
        right.to(20.f + i, ms, d);
    }

    auto lpos = polar(lcd.width() >> 1, lcd.height() >> 1, 100, left.value());
    int16_t vlen = (lcd.height() - lpos.second) * 4 / 5;
    lpos.second += yoff;
    gfx->drawFastVLine(lpos.first, lpos.second, vlen, TFT_WHITE);
    auto rpos = polar(lcd.width() >> 1, lcd.height() >> 1, 100, right.value());
    rpos.second += yoff;
    gfx->drawFastVLine(rpos.first, rpos.second, lcd.height() - rpos.second,
                       TFT_WHITE);

    gfx->drawLine(lpos.first, lpos.second + vlen, rpos.first, rpos.second,
                  TFT_WHITE);

    gfx->drawFastHLine(rpos.first - 4, rpos.second + 30, 8, TFT_WHITE);
    gfx->drawFastHLine(rpos.first - 4, rpos.second + 45, 8, TFT_WHITE);
    gfx->drawFastHLine(rpos.first - 4, rpos.second + 60, 8, TFT_WHITE);
}

void renderBackground(LGFX_Sprite* gfx, int16_t yoff) {
    int16_t cx  = lcd.width() >> 1;
    int16_t cy  = lcd.height() >> 1;
    int16_t rad = lcd.height() >> 1;

    // 黒十字
    gfx->drawFastHLine(0, cy + yoff, lcd.width(), TFT_BLACK);
    for (int x = 0; x < 10; ++x) {
        gfx->drawFastVLine(x * 24, cy - 8 + yoff, 16, TFT_BLACK);
    }
    gfx->drawFastVLine(cx, 0 + yoff, lcd.height(), TFT_BLACK);
    for (int y = 0; y < 10; ++y) {
        gfx->drawFastHLine(cx - 8, y * 24 + yoff, 16, TFT_BLACK);
    }

    // 白円かぶせ
    gfx->drawCircle(lcd.width() * 1.5f, lcd.height() * 1.5f + yoff, 400,
                    TFT_WHITE);

    // 真ん中上
    gfx->drawArc(cx + 8, cy + yoff, rad / 2, rad / 2, 270.f, 300.f, TFT_BLACK);
    gfx->drawFastVLine(cx + 8, cy - rad / 2 + yoff, 32, TFT_BLACK);
}

Ease<float, InOutBack> eff(500, 125.f);
void renderEffect(LGFX_Sprite* gfx, int16_t yoff, const unsigned long ms) {
    int16_t cx   = lcd.width() >> 1;
    int16_t cy   = lcd.height() >> 1;
    int16_t rad  = lcd.height() >> 1;
    int16_t rad0 = rad - 16;
    int16_t rad1 = rad - 24;

    if (!yoff && eff.update(ms)) {
        int32_t i = rng() % (35 - 10);
        int32_t d = rng() % 700 + 700;
        eff.to(125.f + 10.0f + i, ms, d);
    }

    gfx->fillArc(cx, cy + yoff, rad0, rad1, 125.f, 160.f, TFT_BLACK);
    gfx->fillArc(cx, cy + yoff, rad0, rad1, 125.0f, eff.value(),
                 lcd.color888(0x51, 0x6F, 0xA9));
    gfx->drawArc(cx, cy + yoff, rad0, rad1, 125.f, 160.f, TFT_WHITE);
}

void renderTopmost(LGFX_Sprite* gfx, const int16_t yoff) {
    // 右下
    int16_t cx  = lcd.width() >> 1;
    int16_t cy  = lcd.height() >> 1;
    int16_t rad = lcd.height() >> 1;
    gfx->drawArc(cx, cy + yoff, rad + 10, rad - 20, 12.f, 34.f, TFT_BLACK);

    // 全集 黒枠x2
    gfx->fillArc(cx, cy + yoff, rad - 6, rad - 2, 100.f, 80.f, TFT_BLACK);
    gfx->fillArc(cx, cy + yoff, rad - 10, rad - 8, 100.f, 80.f, TFT_BLACK);
}

constexpr int16_t numW = 14;
constexpr int16_t numH = 26;

class RotateNumber {
   public:
    RotateNumber(const int16_t nheight, const unsigned long duration,
                 const uint8_t num)
        : _nheight{nheight}, _duration(duration), _num(num) {
    }

    inline int16_t y() const {
        return _y;
    }

    void to(uint8_t n, const unsigned long ms) {
        n %= _num;
        if (_to != n) {
            _y = _fy  = _ty % (_nheight * 10);
            _start_at = ms;
            _ty       = n * _nheight;
            if (n < _to) {
                _ty += _nheight * 10;
            }
            _to = n;
            // M5_LOGI("[%d];%d -> %d", _to, _fy, _ty);
        }
    }
    void update(unsigned long ms) {
        if (_start_at) {
            if (ms >= _start_at + _duration) {
                _ty %= (_nheight * 10);
                _y = _fy = _ty;
            } else {
                float t = (ms - _start_at) / (float)_duration;
                _y      = (int16_t)(_fy + (_ty - _fy) * t) % (_nheight * 10);
                //                M5_LOGW("[%d] %d: (%d - %d) %f", _to, _y, _fy,
                //                _ty, t);
            }
        }
    }

   private:
    int16_t _fy{}, _ty{}, _y{};
    uint8_t _to{};
    unsigned long _start_at{};
    const int16_t _nheight{};
    const unsigned long _duration{};
    const uint8_t _num{};
};

std::tm latesTime{};

std::array<RotateNumber, 6> rotWatch = {
    RotateNumber(16, 1000, 3), RotateNumber(16, 1000, 10),
    RotateNumber(16, 1000, 6), RotateNumber(16, 1000, 10),
    RotateNumber(16, 1000, 6), RotateNumber(16, 1000, 10)};

void renderWatch(LGFX_Sprite* gfx, const int16_t left, const int16_t top,
                 const int16_t yoff, const unsigned long ms) {
    auto now = time(nullptr);
    auto ltm = std::localtime(&now);
    if (!yoff) {
        rotWatch[0].to(ltm->tm_hour / 10, ms);
        rotWatch[1].to(ltm->tm_hour % 10, ms);
        rotWatch[2].to(ltm->tm_min / 10, ms);
        rotWatch[3].to(ltm->tm_min % 10, ms);
        rotWatch[4].to(ltm->tm_sec / 10, ms);
        rotWatch[5].to(ltm->tm_sec % 10, ms);
    }
    uint_fast8_t idx{};
    for (auto&& e : rotWatch) {
        e.update(ms);
        number3.pushPartial(&watch, 10 * idx, 0, 10, 16, 0, e.y());
        ++idx;
    }
    watch.pushSprite(gfx, left, top + yoff);
}

std::array<RotateNumber, 6> rotLatest = {
    RotateNumber(16, 1000, 3), RotateNumber(16, 1000, 10),
    RotateNumber(16, 1000, 6), RotateNumber(16, 1000, 10),
    RotateNumber(16, 1000, 6), RotateNumber(16, 1000, 10)};
void renderLatest(LGFX_Sprite* gfx, const int16_t left, const int16_t top,
                  const int16_t yoff, const unsigned long ms) {
    uint_fast8_t idx{};
    for (auto&& e : rotLatest) {
        e.update(ms);
        number3.pushPartial(&watch, 10 * idx, 0, 10, 16, 0, e.y());
        ++idx;
    }
    watch.pushSprite(gfx, left, top + yoff);
}

std::array<RotateNumber, 4> rotTemp = {
    RotateNumber(8, 1000, 10), RotateNumber(8, 1000, 10),
    RotateNumber(8, 1000, 10), RotateNumber(8, 1000, 10)};

Ease<float, InBack> easeTemp(1000, 25.0f);
void renderTemp(LGFX_Sprite* gfx, const int16_t x, const int16_t y,
                const float tmp, const int16_t yoff, const unsigned long ms) {
    gfx->fillCircle(x, y + yoff, 24, TFT_BLACK);
    gfx->drawCircle(x, y + yoff, 24, TFT_WHITE);
    gfx->fillArc(x, y + yoff, 22, 23, 100.0f, 80.0f, TFT_DARKGRAY);

    if (!yoff) {
        easeTemp.to(tmp, ms);
        easeTemp.update(ms);

        int32_t ti = (int32_t)(tmp * 100);
        rotTemp[0].to(ti / 1000, ms);
        rotTemp[1].to((ti / 100) % 10, ms);
        rotTemp[2].to((ti / 10) % 10, ms);
        rotTemp[3].to(ti % 10, ms);
    }

    float t           = easeTemp.value() / 50.0f;
    const float range = 180.0f;
    float deg         = range * t - range * 0.5f;
    if (deg < 0.0f) {
        deg += 360.0f;
    }
    //    M5_LOGW("tmp:%f deg:%f", easeTemp.value(), deg);
    arrow2.setPaletteColor(1, TFT_YELLOW);
    arrow2.pushRotateZoom(gfx, x, y + yoff, deg, 1.f, 1.f, 0);

    uint_fast8_t idx{};
    for (auto&& e : rotTemp) {
        e.update(ms);
        number2.pushPartial(&meter, 8 * idx, 0, 8, 8, 0, e.y());
        ++idx;
    }
    meter.pushSprite(gfx, x - 16, y + 8 + yoff);

    gfx->setCursor(x - 6 * 2, y + 24 + yoff);
    gfx->printf("%s", "temp");
}

std::array<RotateNumber, 4> rotHum = {
    RotateNumber(8, 1000, 10), RotateNumber(8, 1000, 10),
    RotateNumber(8, 1000, 10), RotateNumber(8, 1000, 10)};

Ease<float, InBack> easeHumidity(1000, 25.0f);
void renderHumidity(LGFX_Sprite* gfx, const int16_t x, const int16_t y,
                    const float hum, const int16_t yoff,
                    const unsigned long ms) {
    gfx->fillCircle(x, y + yoff, 24, TFT_BLACK);
    gfx->drawCircle(x, y + yoff, 24, TFT_WHITE);
    gfx->fillArc(x, y + yoff, 22, 23, 100.0f, 80.0f, TFT_DARKGRAY);

    if (!yoff) {
        easeHumidity.to(hum, ms);
        easeHumidity.update(ms);

        int32_t ti = (int32_t)(hum * 100);
        rotHum[0].to(ti / 1000, ms);
        rotHum[1].to((ti / 100) % 10, ms);
        rotHum[2].to((ti / 10) % 10, ms);
        rotHum[3].to(ti % 10, ms);
    }

    float t           = easeHumidity.value() / 100.0f;
    const float range = 180.0f;
    float deg         = range * t - range * 0.5f;
    if (deg < 0.0f) {
        deg += 360.0f;
    }
    //    M5_LOGW("hum:%f t:%f deg:%f", easeHumidity.value(), t, deg);
    arrow2.pushRotateZoom(gfx, x, y + yoff, deg, 1.f, 1.f, 0);

    uint_fast8_t idx{};
    for (auto&& e : rotHum) {
        e.update(ms);
        number2.pushPartial(&meter, 8 * idx, 0, 8, 8, 0, e.y());
        ++idx;
    }
    meter.pushSprite(gfx, x - 16, y + 8 + yoff);

    gfx->setCursor(x - 6 * 2, y + 24 + yoff);
    gfx->printf("%s", " RH ");
}

Ease<float, InBack> easeCO2(1000, 25.0f);
std::array<RotateNumber, 4> rotCO2 = {
    RotateNumber(numH, 1000, 10), RotateNumber(numH, 1000, 10),
    RotateNumber(numH, 1000, 10), RotateNumber(numH, 1000, 10)};

void renderCO2(LGFX_Sprite* gfx, const int16_t x, const int16_t y,
               const float ppm, const int16_t yoff, const unsigned long ms) {
    uint16_t color = ppm > 10000.f ? TFT_RED : TFT_WHITE;

    if (!yoff) {
        easeCO2.to(ppm, ms);
        easeCO2.update(ms);

        int32_t ti = (int32_t)(ppm);
        rotCO2[0].to(ti / 1000, ms);
        rotCO2[1].to((ti / 100) % 10, ms);
        rotCO2[2].to((ti / 10) % 10, ms);
        rotCO2[3].to(ti % 10, ms);
    }

    float t = easeCO2.value() / (6000.f);
    if (t < 0.0f) {
        t = 0.0f;
    }
    if (t > 1.0f) {
        t = 1.0f;
    }
    const float range = 75.0f;  // 70
    float deg         = range * t - range * 0.5f;
    if (deg < 0.0f) {
        deg += 360.0f;
    }
    // M5_LOGW("co2:%f t:%f deg:%f", easeCO2.value(), t, deg);
    arrow.setPaletteColor(1, color);
    arrow.pushRotateZoom(gfx, x, y + yoff, deg, 1.f, 1.f, 0);

    meter2.setPaletteColor(1, color);
    uint_fast8_t idx{};
    for (auto&& e : rotCO2) {
        e.update(ms);
        number.pushPartial(&meter2, (numW + 2) * idx, 0, (numW + 2), numH, 0,
                           e.y());
        ++idx;
    }
    meter2.pushSprite(gfx, x - 2 * (numW + 2), y - 16 + yoff);

    number.setPaletteColor(1, TFT_WHITE);
    gfx->setCursor(x - 6 * 2, y + 16 + yoff);
    gfx->printf("%s", " ppm ");
}

void setup() {
    M5.begin();

    m5::utility::delay(5000);
    M5_LOGI("Wait........");

    //    configTime();

    auto pin_num_sda = M5.getPin(m5::pin_name_t::port_a_sda);
    auto pin_num_scl = M5.getPin(m5::pin_name_t::port_a_scl);
    M5_LOGI("getPin: SDA:%u SCL:%u", pin_num_sda, pin_num_scl);

    // 直接接続
#if 0

#if defined(USING_BUS)
    // Bus 版
    m5::hal::bus::I2CBusConfig i2c_cfg;
    i2c_cfg.pin_sda = m5::hal::gpio::getPin(pin_num_sda);
    i2c_cfg.pin_scl = m5::hal::gpio::getPin(pin_num_scl);
    auto i2c_bus = m5::hal::bus::i2c::getBus(i2c_cfg);
    Units.add(unitSCD40, i2c_bus ? i2c_bus.value() : nullptr);
#else
    // The Wire to be used must be initiated in advance.
    Wire.begin(pin_num_sda, pin_num_scl, 400000U);
    Units.add(unitSCD40, Wire);
#endif

    // PaHub経由
#else

#if defined(USING_BUS)
    M5_LOGI("Bus & PaHub");
    // Bus 版
    m5::hal::bus::I2CBusConfig i2c_cfg;
    i2c_cfg.pin_sda = m5::hal::gpio::getPin(pin_num_sda);
    i2c_cfg.pin_scl = m5::hal::gpio::getPin(pin_num_scl);
    auto i2c_bus    = m5::hal::bus::i2c::getBus(i2c_cfg);

    unitPaHub.add(unitSCD40, 5 /*ch*/);
    Units.add(unitSCD40, i2c_bus ? i2c_bus.value() : nullptr);
#else

    M5_LOGI("Wire & PaHub");
    // The Wire to be used must be initiated in advance.
    Wire.begin(pin_num_sda, pin_num_scl, 400000U);

    unitPaHub.add(unitSCD40, 5 /*ch*/);
    Units.add(unitPaHub, Wire);
#endif
#endif

    M5_LOGW("%s", Units.debugInfo().c_str());
    auto b = Units.begin();
    M5_LOGI("Units.begin: %d", b);

    //
    strip_height = (lcd.height() + strip_div - 1) / strip_div;
    size_t sc{0};
    for (; sc < 2; ++sc) {
        strip[sc].setColorDepth(lcd.getColorDepth());
        strip[sc].setPsram(false);
        if (!strip[sc].createSprite(lcd.width(), strip_height)) {
            break;
        }
    }
    if (sc != 2) {
        M5_LOGE("Failed to create strip sprites");
        do {
            m5::utility::delay(1000);
        } while (true);
    }

    //
    bg.setPsram(false);
    bg.createSprite(lcd.width(), strip_height);
    for (int_fast16_t y = 0; y < bg.height(); ++y) {
        bg.drawGradientLine(0, y, bg.width(), y, 0x4ed1, TFT_DARKGREEN);
    }

    //
    number.setPsram(false);
    number.setColorDepth(1);
    number.createSprite(numW + 2, numH * 11);
    number.setPaletteColor(0, TFT_BLACK);
    number.setPaletteColor(1, TFT_WHITE);
    number.setTextColor(1, 0);
    number.setFont(&fonts::Font4);  // 14x26
    for (int i = 0; i < 11; ++i) {
        number.setCursor(1, i * numH + 1);
        number.printf("%d", i % 10);
    }
    number.drawFastVLine(0, 0, number.height(), 1);
    number.drawFastVLine(number.width() - 1, 0, number.height(), 1);

    //
    meter2.setPsram(false);
    meter2.setColorDepth(1);
    meter2.createSprite((numW + 2) * 4, numH);
    meter2.setPaletteColor(0, TFT_BLACK);
    meter2.setPaletteColor(1, TFT_WHITE);

    {
        constexpr int16_t nw = 6;
        constexpr int16_t nh = 8;
        number2.setPsram(false);
        number2.setColorDepth(1);
        number2.createSprite(nw + 2, nh * 11);
        number2.setPaletteColor(0, TFT_BLACK);
        number2.setPaletteColor(1, TFT_WHITE);
        number2.setTextColor(1, 0);
        number2.setFont(&fonts::Font0);  // 6x8 / 8x16
        for (int i = 0; i < 11; ++i) {
            number2.setCursor(1, i * nh + 1);
            number2.printf("%d", i % 10);
        }
        number2.drawFastVLine(0, 0, number2.height(), 1);
        number2.drawFastVLine(number2.width() - 1, 0, number2.height(), 1);

        meter.setPsram(false);
        meter.setColorDepth(1);
        meter.createSprite((nw + 2) * 4, nh);
        meter.setPaletteColor(0, TFT_BLACK);
        meter.setPaletteColor(1, TFT_WHITE);
        number2.setTextColor(1, 0);
    }

    {
        constexpr int16_t nw = 8;
        constexpr int16_t nh = 16;
        number3.setPsram(false);
        number3.setColorDepth(1);
        number3.createSprite(nw + 2, nh * 11);
        number3.setPaletteColor(0, TFT_BLACK);
        number3.setPaletteColor(1, TFT_WHITE);
        number3.setTextColor(1, 0);
        number3.setFont(&fonts::Font2);  // 8x16
        for (int i = 0; i < 11; ++i) {
            number3.setCursor(1, i * nh + 1);
            number3.printf("%d", i % 10);
        }
        number3.drawFastVLine(0, 0, number3.height(), 1);
        number3.drawFastVLine(number3.width() - 1, 0, number3.height(), 1);

        watch.setPsram(false);
        watch.setColorDepth(1);
        watch.createSprite((nw + 2) * 6, nh);
        watch.setPaletteColor(0, TFT_BLACK);
        watch.setPaletteColor(1, TFT_WHITE);
        number3.setTextColor(1, 0);
    }

    //

    //
    {
        constexpr int16_t awidth  = 16;
        constexpr int16_t aheight = 176;
        constexpr int16_t thgt    = 16;
        arrow.setPsram(false);
        arrow.setColorDepth(1);
        arrow.setPaletteColor(0, TFT_BLACK);
        arrow.setPaletteColor(1, TFT_WHITE);
        arrow.createSprite(awidth, aheight);
        arrow.fillTriangle(0, aheight - thgt, awidth, aheight - thgt,
                           awidth / 2, 0,
                           1);  // half top
        arrow.fillTriangle(0, aheight - thgt, awidth, aheight - thgt,
                           awidth / 2, aheight, 1);  // half bottom
        arrow.setPivot(awidth / 2, aheight - thgt);
    }
    {
        arrow2.setPsram(false);
        arrow2.setColorDepth(1);
        arrow2.setPaletteColor(0, TFT_BLACK);
        arrow2.setPaletteColor(1, TFT_WHITE);
        arrow2.createSprite(7, 24);
        arrow2.fillTriangle(0, 20, 6, 20, 3, 0);
        arrow2.fillTriangle(0, 20, 6, 20, 3, 23);
        arrow2.setPivot(3.5f, 20.0f);
    }

    M5_LOGI("Heap:%u", esp_get_free_heap_size());

    lcd.startWrite();
}

void loop() {
    auto ms = m5::utility::millis();

    M5.update();
    Units.update();

    if (unitSCD40.updated()) {
        auto now = time(nullptr);
        auto ltm = std::localtime(&now);
        rotLatest[0].to(ltm->tm_hour / 10, ms);
        rotLatest[1].to(ltm->tm_hour % 10, ms);
        rotLatest[2].to(ltm->tm_min / 10, ms);
        rotLatest[3].to(ltm->tm_min % 10, ms);
        rotLatest[4].to(ltm->tm_sec / 10, ms);
        rotLatest[5].to(ltm->tm_sec % 10, ms);

        //        M5_LOGI("Heap:%u", esp_get_free_heap_size());
        M5_LOGI("%u / %f / %f", unitSCD40.co2(), unitSCD40.temperature(),
                unitSCD40.humidity());
    }
    
    int_fast8_t cnt = 3;
    int_fast8_t cur = 0;
    int16_t y       = 0;
    while (cnt--) {
        auto spr = &strip[cur];
        bg.pushSprite(&strip[cur], 0, 0);  // clear by bg

        renderBackground(spr, y);
        renderWhiteGauge(spr, y, ms);
        renderEffect(spr, y, ms);

        renderCO2(spr, lcd.width() >> 1, 180, unitSCD40.co2(), y, ms);
        renderTemp(spr, 60, 80, unitSCD40.temperature(), y, ms);
        renderHumidity(spr, 120 + 64, 120 + 28, unitSCD40.humidity(), y, ms);

        renderWatch(spr, 184 - 24, 80, y, ms);
        renderLatest(spr, 184 - 24, 100, y, ms);
        renderTopmost(spr, y);

        spr->pushSprite(&lcd, 0, -y);

        y -= strip_height;
        cur ^= 1;
    }
    //    lcd.setCursor(64, 100);
    //    lcd.printf("Heap:%u", esp_get_free_heap_size());
    //    lcd.display();

    m5::utility::delay(1000 / 27);
}
