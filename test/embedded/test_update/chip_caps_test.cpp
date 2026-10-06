/*
 * SPDX-FileCopyrightText: 2026 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
/*
  UnitTest for chip-dependent paths (ADC channel mapping, I2C pin lookup, RMT v1 channel allocation)
  No wiring required
*/
#include <gtest/gtest.h>
#include <M5Unified.h>
#include <M5UnitUnified.hpp>
#include <m5_unit_component/adapter.hpp>
#include <Wire.h>
#include <driver/gpio.h>
#include <soc/soc_caps.h>
#include <soc/adc_channel.h>
#include <memory>
#include <vector>
#include <wiring/m5_unit_unified_wiring.hpp>  // include last; M5_UNIT_UNIFIED_WIRING_HAS_WIRE1

using namespace m5::unit;

namespace {
constexpr uint32_t I2C_FREQ{100000U};
constexpr uint8_t I2C_ADDR{0x42};

bool valid_pin(const int pin)
{
    return pin >= 0 && pin < GPIO_NUM_MAX;
}

// Pick two distinct output-capable pins from the board's free ports (I2C needs both SDA/SCL as outputs)
bool pick_output_pin_pair(int& first, int& second)
{
    const m5::pin_name_t candidates[] = {m5::pin_name_t::port_b_out, m5::pin_name_t::port_b_in,
                                         m5::pin_name_t::port_c_txd, m5::pin_name_t::port_c_rxd};
    first                             = -1;
    second                            = -1;
    for (auto&& c : candidates) {
        const int pin = M5.getPin(c);
        if (!valid_pin(pin) || !GPIO_IS_VALID_OUTPUT_GPIO(pin) || pin == first) {
            continue;
        }
        if (first < 0) {
            first = pin;
        } else {
            second = pin;
            return true;
        }
    }
    return false;
}

}  // namespace

// gpio_to_adc_channel() must agree with the SoC definition (soc/adc_channel.h) for every GPIO
TEST(ChipCaps, GpioToAdcChannel)
{
    struct gpio_adc_t {
        int gpio;
        int8_t channel;  // 0-9: ADC1, 10-: ADC2 + 10
    };
    constexpr gpio_adc_t expected[] = {
#if defined(ADC1_CHANNEL_0_GPIO_NUM)
        {ADC1_CHANNEL_0_GPIO_NUM, 0},
#endif
#if defined(ADC1_CHANNEL_1_GPIO_NUM)
        {ADC1_CHANNEL_1_GPIO_NUM, 1},
#endif
#if defined(ADC1_CHANNEL_2_GPIO_NUM)
        {ADC1_CHANNEL_2_GPIO_NUM, 2},
#endif
#if defined(ADC1_CHANNEL_3_GPIO_NUM)
        {ADC1_CHANNEL_3_GPIO_NUM, 3},
#endif
#if defined(ADC1_CHANNEL_4_GPIO_NUM)
        {ADC1_CHANNEL_4_GPIO_NUM, 4},
#endif
#if defined(ADC1_CHANNEL_5_GPIO_NUM)
        {ADC1_CHANNEL_5_GPIO_NUM, 5},
#endif
#if defined(ADC1_CHANNEL_6_GPIO_NUM)
        {ADC1_CHANNEL_6_GPIO_NUM, 6},
#endif
#if defined(ADC1_CHANNEL_7_GPIO_NUM)
        {ADC1_CHANNEL_7_GPIO_NUM, 7},
#endif
#if defined(ADC1_CHANNEL_8_GPIO_NUM)
        {ADC1_CHANNEL_8_GPIO_NUM, 8},
#endif
#if defined(ADC1_CHANNEL_9_GPIO_NUM)
        {ADC1_CHANNEL_9_GPIO_NUM, 9},
#endif
#if defined(ADC2_CHANNEL_0_GPIO_NUM)
        {ADC2_CHANNEL_0_GPIO_NUM, 10},
#endif
#if defined(ADC2_CHANNEL_1_GPIO_NUM)
        {ADC2_CHANNEL_1_GPIO_NUM, 11},
#endif
#if defined(ADC2_CHANNEL_2_GPIO_NUM)
        {ADC2_CHANNEL_2_GPIO_NUM, 12},
#endif
#if defined(ADC2_CHANNEL_3_GPIO_NUM)
        {ADC2_CHANNEL_3_GPIO_NUM, 13},
#endif
#if defined(ADC2_CHANNEL_4_GPIO_NUM)
        {ADC2_CHANNEL_4_GPIO_NUM, 14},
#endif
#if defined(ADC2_CHANNEL_5_GPIO_NUM)
        {ADC2_CHANNEL_5_GPIO_NUM, 15},
#endif
#if defined(ADC2_CHANNEL_6_GPIO_NUM)
        {ADC2_CHANNEL_6_GPIO_NUM, 16},
#endif
#if defined(ADC2_CHANNEL_7_GPIO_NUM)
        {ADC2_CHANNEL_7_GPIO_NUM, 17},
#endif
#if defined(ADC2_CHANNEL_8_GPIO_NUM)
        {ADC2_CHANNEL_8_GPIO_NUM, 18},
#endif
#if defined(ADC2_CHANNEL_9_GPIO_NUM)
        {ADC2_CHANNEL_9_GPIO_NUM, 19},
#endif
    };

    for (int pin = -1; pin <= GPIO_NUM_MAX; ++pin) {
        int8_t ch{-1};
        for (auto&& e : expected) {
            if (e.gpio == pin) {
                ch = e.channel;
                break;
            }
        }
        EXPECT_EQ(gpio::gpio_to_adc_channel(pin), ch) << "GPIO" << pin;
    }
}

// AdapterI2C(TwoWire) looks up SDA/SCL from the GPIO matrix input selection of the I2C peripheral
TEST(ChipCaps, I2CWirePinLookup)
{
    const int sda = M5.getPin(m5::pin_name_t::port_a_sda);
    const int scl = M5.getPin(m5::pin_name_t::port_a_scl);
    if (!valid_pin(sda) || !valid_pin(scl)) {
        GTEST_SKIP() << "No PortA on this board";
    }

    Wire.end();
    const bool began = Wire.begin(sda, scl, I2C_FREQ);
    EXPECT_TRUE(began);
    if (began) {
        AdapterI2C adapter(Wire, I2C_ADDR, I2C_FREQ);
        EXPECT_EQ(adapter.sda(), sda);
        EXPECT_EQ(adapter.scl(), scl);
    }
    Wire.end();
}

#if M5_UNIT_UNIFIED_WIRING_HAS_WIRE1
// Wire1 is HP I2C1 (routed through the GPIO matrix) or LP I2C (fixed pads, not looked up)
// This checks only the pin lookup logic of AdapterI2C, not bus communication; it also runs on boards
// that do not route Wire1 to any connector (e.g. NanoC6, whose LP I2C pads are not on the GROVE port)
TEST(ChipCaps, I2CWire1PinLookup)
{
    int sda{-1}, scl{-1};
    constexpr bool wire1_is_hp{M5_UNIT_UNIFIED_WIRING_HP_I2C_NUM > 1};
    if (wire1_is_hp && !pick_output_pin_pair(sda, scl)) {
        GTEST_SKIP() << "No output-capable free port pins on this board";
    }

    Wire1.end();
    const bool began = wire1_is_hp ? Wire1.begin(sda, scl, I2C_FREQ) : true;
    EXPECT_TRUE(began);
    if (began) {
        AdapterI2C adapter(Wire1, I2C_ADDR, I2C_FREQ);
        if (wire1_is_hp) {
            EXPECT_EQ(adapter.sda(), sda);
            EXPECT_EQ(adapter.scl(), scl);
        } else {
            // LP I2C on fixed pads: not looked up, and pushPin/popPin must not touch HP I2C0 pins
            EXPECT_EQ(adapter.sda(), -1);
            EXPECT_EQ(adapter.scl(), -1);
        }
    }
    Wire1.end();
}
#endif

namespace {
int rmt_test_pin()
{
    // PortA is the last resort (e.g. NanoC6 has PortA only); the I2C tests release it with Wire.end()
    const m5::pin_name_t candidates[] = {m5::pin_name_t::port_b_out, m5::pin_name_t::port_b_in,
                                         m5::pin_name_t::port_c_txd, m5::pin_name_t::port_c_rxd,
                                         m5::pin_name_t::port_a_sda, m5::pin_name_t::port_a_scl};
    for (auto&& c : candidates) {
        const int pin = M5.getPin(c);
        if (valid_pin(pin)) {
            return pin;
        }
    }
    return -1;
}
}  // namespace

#if !defined(M5_UNIT_UNIFIED_USING_RMT_V2)
// RMT v1 (ESP-IDF 4.x): TX uses the TX-capable channels, RX uses the RX-capable channels
TEST(ChipCaps, RmtV1ChannelAllocation)
{
    const int pin = rmt_test_pin();
    if (!valid_pin(pin)) {
        GTEST_SKIP() << "No free GPIO port on this board";
    }
    constexpr int rx_first = SOC_RMT_CHANNELS_PER_GROUP - SOC_RMT_RX_CANDIDATES_PER_GROUP;

    // TX
    {
        gpio::adapter_config_t cfg{};
        cfg.mode          = gpio::Mode::RmtTX;
        cfg.tx.tick_ns    = 1000;
        cfg.tx.mem_blocks = 1;

        std::vector<std::unique_ptr<AdapterGPIO>> adapters;
        for (int i = 0; i < SOC_RMT_CHANNELS_PER_GROUP; ++i) {
            std::unique_ptr<AdapterGPIO> a(new AdapterGPIO(-1, pin));
            if (!a->begin(cfg)) {
                break;
            }
            const int ch = a->impl()->rmtTxChannel();
            EXPECT_GE(ch, 0);
            EXPECT_LT(ch, SOC_RMT_TX_CANDIDATES_PER_GROUP);
            adapters.emplace_back(std::move(a));
        }
        EXPECT_EQ(adapters.size(), static_cast<size_t>(SOC_RMT_TX_CANDIDATES_PER_GROUP));
    }
    // RX
    {
        gpio::adapter_config_t cfg{};
        cfg.mode                    = gpio::Mode::RmtRX;
        cfg.rx.tick_ns              = 1000;
        cfg.rx.mem_blocks           = 1;
        cfg.rx.ring_buffer_size     = 256;
        cfg.rx.idle_ticks_threshold = 1000;

        std::vector<std::unique_ptr<AdapterGPIO>> adapters;
        for (int i = 0; i < SOC_RMT_CHANNELS_PER_GROUP; ++i) {
            std::unique_ptr<AdapterGPIO> a(new AdapterGPIO(pin, -1));
            if (!a->begin(cfg)) {
                break;
            }
            const int ch = a->impl()->rmtRxChannel();
            EXPECT_GE(ch, rx_first);
            EXPECT_LT(ch, SOC_RMT_CHANNELS_PER_GROUP);
            adapters.emplace_back(std::move(a));
        }
        EXPECT_EQ(adapters.size(), static_cast<size_t>(SOC_RMT_RX_CANDIDATES_PER_GROUP));
    }
}

// RMT v1: a channel with mem_block_num k occupies blocks [ch, ch + k), so channels must not share blocks
TEST(ChipCaps, RmtV1MemoryBlocksDoNotOverlap)
{
    const int pin = rmt_test_pin();
    if (!valid_pin(pin)) {
        GTEST_SKIP() << "No free GPIO port on this board";
    }
    constexpr uint8_t blocks = 2;

    gpio::adapter_config_t cfg{};
    cfg.mode          = gpio::Mode::RmtTX;
    cfg.tx.tick_ns    = 1000;
    cfg.tx.mem_blocks = blocks;

    std::vector<std::unique_ptr<AdapterGPIO>> adapters;
    std::vector<int> channels;
    for (int i = 0; i < SOC_RMT_CHANNELS_PER_GROUP; ++i) {
        std::unique_ptr<AdapterGPIO> a(new AdapterGPIO(-1, pin));
        if (!a->begin(cfg)) {
            break;
        }
        channels.push_back(a->impl()->rmtTxChannel());
        adapters.emplace_back(std::move(a));
    }
    // TX channels [0, TX candidates) with 2 blocks each: 0, 2, 4, ... (ESP32: 4, S3: 2, C3: 1)
    EXPECT_EQ(channels.size(), static_cast<size_t>((SOC_RMT_TX_CANDIDATES_PER_GROUP + blocks - 1) / blocks));
    for (size_t i = 1; i < channels.size(); ++i) {
        EXPECT_GE(channels[i] - channels[i - 1], static_cast<int>(blocks)) << "channel " << channels[i];
    }
    for (auto&& ch : channels) {
        EXPECT_LE(ch + blocks, SOC_RMT_CHANNELS_PER_GROUP) << "channel " << ch;
    }
}
#endif

#if defined(M5_UNIT_UNIFIED_USING_RMT_V2)
// RMT v2 (ESP-IDF 5+): channels are allocated by the driver up to the TX/RX-capable counts
TEST(ChipCaps, RmtV2ChannelAllocation)
{
    const int pin = rmt_test_pin();
    if (!valid_pin(pin)) {
        GTEST_SKIP() << "No free GPIO port on this board";
    }

    // TX
    {
        gpio::adapter_config_t cfg{};
        cfg.mode          = gpio::Mode::RmtTX;
        cfg.tx.tick_ns    = 1000;
        cfg.tx.mem_blocks = 1;

        std::vector<std::unique_ptr<AdapterGPIO>> adapters;
        for (int i = 0; i < SOC_RMT_CHANNELS_PER_GROUP; ++i) {
            std::unique_ptr<AdapterGPIO> a(new AdapterGPIO(-1, pin));
            if (!a->begin(cfg)) {
                break;
            }
            EXPECT_NE(a->impl()->rmtTxHandle(), nullptr);
            adapters.emplace_back(std::move(a));
        }
        EXPECT_FALSE(adapters.empty());
#if defined(SOC_RMT_TX_CANDIDATES_PER_GROUP)
        EXPECT_EQ(adapters.size(), static_cast<size_t>(SOC_RMT_TX_CANDIDATES_PER_GROUP));
#endif
    }
    // RX
    {
        gpio::adapter_config_t cfg{};
        cfg.mode                    = gpio::Mode::RmtRX;
        cfg.rx.tick_ns              = 1000;
        cfg.rx.mem_blocks           = 1;
        cfg.rx.ring_buffer_size     = 256;
        cfg.rx.idle_ticks_threshold = 10000;

        std::vector<std::unique_ptr<AdapterGPIO>> adapters;
        for (int i = 0; i < SOC_RMT_CHANNELS_PER_GROUP; ++i) {
            std::unique_ptr<AdapterGPIO> a(new AdapterGPIO(pin, -1));
            if (!a->begin(cfg)) {
                break;
            }
            EXPECT_NE(a->impl()->rmtRxHandle(), nullptr);
            adapters.emplace_back(std::move(a));
        }
        EXPECT_FALSE(adapters.empty());
#if defined(SOC_RMT_RX_CANDIDATES_PER_GROUP)
        EXPECT_EQ(adapters.size(), static_cast<size_t>(SOC_RMT_RX_CANDIDATES_PER_GROUP));
#endif
    }
}
#endif

#if M5_UNIT_UNIFIED_HAS_RMT
// RxPull is applied at begin() whatever the RMT driver did to the pin (ESP-IDF 5.1-5.5 enables the pull-up itself)
TEST(ChipCaps, RmtRxPull)
{
    const int pin = rmt_test_pin();
    if (!valid_pin(pin) || !GPIO_IS_VALID_OUTPUT_GPIO(pin)) {
        GTEST_SKIP() << "No free GPIO with an internal pull on this board";
    }

    auto level_with = [pin](const gpio::RxPull pull) {
        gpio::adapter_config_t cfg{};
        cfg.mode                    = gpio::Mode::RmtRX;
        cfg.rx.tick_ns              = 1000;
        cfg.rx.mem_blocks           = 1;
        cfg.rx.ring_buffer_size     = 256;
        cfg.rx.idle_ticks_threshold = 10000;
        cfg.rx.pull                 = pull;
        AdapterGPIO a(pin, -1);
        EXPECT_TRUE(a.begin(cfg));
        m5::utility::delay(5);
        return gpio_get_level(static_cast<gpio_num_t>(pin));
    };

    const int up   = level_with(gpio::RxPull::Up);
    const int down = level_with(gpio::RxPull::Down);
    if (up == down) {
        GTEST_SKIP() << "GPIO" << pin << " is driven externally (level " << up << " with both pulls)";
    }
    EXPECT_EQ(up, 1);
    EXPECT_EQ(down, 0);

#if defined(CONFIG_IDF_TARGET_ESP32)
    // Input-only pads have no internal pull
    EXPECT_FALSE(gpio::apply_rx_pull(GPIO_NUM_36, gpio::RxPull::Up));
    EXPECT_TRUE(gpio::apply_rx_pull(GPIO_NUM_36, gpio::RxPull::None));
#endif
}
#endif

#if defined(M5_UNIT_UNIFIED_USING_RMT_V2) && M5_UNIT_UNIFIED_HAS_RMT
#include <driver/rmt_types.h>

namespace {
// RX and TX on the same pin: the RX channel reads back what the TX channel drives, so no unit is needed
gpio::adapter_config_t loopback_config(const uint16_t ring_buffer_size)
{
    gpio::adapter_config_t cfg{};
    cfg.mode                    = gpio::Mode::RmtRXTX;
    cfg.tx.tick_ns              = 1000;
    cfg.tx.mem_blocks           = 1;
    cfg.tx.idle_output_enabled  = true;
    cfg.rx.tick_ns              = 1000;
    cfg.rx.ring_buffer_size     = ring_buffer_size;
    cfg.rx.idle_ticks_threshold = 200;  // 200us of no edge ends a frame
#if defined(SOC_RMT_SUPPORT_RX_PINGPONG) && SOC_RMT_SUPPORT_RX_PINGPONG
    cfg.rx.mem_blocks = 1;
#else
    cfg.rx.mem_blocks = 4;  // Without RX ping-pong a frame must fit in the channel memory
#endif
    return cfg;
}

std::vector<rmt_symbol_word_t> make_frame(const size_t count)
{
    std::vector<rmt_symbol_word_t> v(count);
    for (size_t i = 0; i < count; ++i) {
        v[i].level0    = 1;
        v[i].duration0 = 30 + (i % 7) * 5;  // us
        v[i].level1    = 0;
        v[i].duration1 = 40 + (i % 5) * 5;
    }
    return v;
}

// Read one frame (2-byte length + symbols). Returns the number of symbols
size_t read_frame(AdapterGPIO& a, std::vector<uint8_t>& buf)
{
    for (int i = 0; i < 10; ++i) {
        if (a.readWithTransaction(buf.data(), buf.size()) == m5::hal::error::error_t::OK) {
            uint16_t len{};
            memcpy(&len, buf.data(), sizeof(len));
            return len / sizeof(rmt_symbol_word_t);
        }
    }
    return 0;
}
}  // namespace

// Frames go through the receive ISR into the ringbuffer, including one larger than half of ring_buffer_size
TEST(ChipCaps, RmtV2LoopbackReceive)
{
    const int pin = rmt_test_pin();
    if (!valid_pin(pin) || !GPIO_IS_VALID_OUTPUT_GPIO(pin)) {
        GTEST_SKIP() << "No free output-capable GPIO on this board";
    }
    constexpr uint16_t ring_buffer_size = 1024;
    AdapterGPIO a(pin, pin);
    if (!a.begin(loopback_config(ring_buffer_size))) {
        ADD_FAILURE() << "begin failed";
        return;
    }

    std::vector<uint8_t> buf(ring_buffer_size + 2);
    for (auto&& count : {8u, 64u, 200u}) {  // 200 symbols = 800 bytes > ring_buffer_size / 2
        SCOPED_TRACE(count);
        const auto frame = make_frame(count);
        EXPECT_EQ(a.writeWithTransaction(reinterpret_cast<const uint8_t*>(frame.data()),
                                         frame.size() * sizeof(rmt_symbol_word_t), 1000),
                  m5::hal::error::error_t::OK);

        const size_t received = read_frame(a, buf);
        EXPECT_EQ(received, count);
        if (received != count) {
            continue;
        }
        const auto* sym = reinterpret_cast<const rmt_symbol_word_t*>(buf.data() + 2);
        for (size_t i = 0; i < received; ++i) {
            EXPECT_EQ(sym[i].level0, 1u) << i;
            EXPECT_NEAR(sym[i].duration0, frame[i].duration0, 2) << i;
            if (i + 1 < received) {  // The last low lasts until the idle threshold ends the frame
                EXPECT_NEAR(sym[i].duration1, frame[i].duration1, 2) << i;
            }
        }
    }
}

// Destroy adapters while frames are still in flight: the receive ISR must never touch a freed adapter
TEST(ChipCaps, RmtV2RecreateWhileReceiving)
{
    const int pin = rmt_test_pin();
    if (!valid_pin(pin) || !GPIO_IS_VALID_OUTPUT_GPIO(pin)) {
        GTEST_SKIP() << "No free output-capable GPIO on this board";
    }
    const auto frame = make_frame(16);
    for (int i = 0; i < 100; ++i) {
        AdapterGPIO a(pin, pin);
        if (!a.begin(loopback_config(256))) {
            ADD_FAILURE() << "begin failed at " << i;
            return;
        }
        for (int j = 0; j < 3; ++j) {
            a.writeWithTransaction(reinterpret_cast<const uint8_t*>(frame.data()),
                                   frame.size() * sizeof(rmt_symbol_word_t), 0);
        }
        m5::utility::delay(i % 4);  // Destroy at different points of the transmission / reception
    }

    // A fresh adapter still works
    AdapterGPIO a(pin, pin);
    if (!a.begin(loopback_config(256))) {
        ADD_FAILURE() << "begin failed after recreating";
        return;
    }
    std::vector<uint8_t> buf(256 + 2);
    EXPECT_EQ(a.writeWithTransaction(reinterpret_cast<const uint8_t*>(frame.data()),
                                     frame.size() * sizeof(rmt_symbol_word_t), 1000),
              m5::hal::error::error_t::OK);
    EXPECT_EQ(read_frame(a, buf), frame.size());
}
#endif
