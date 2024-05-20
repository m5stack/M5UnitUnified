/*
  UnitTest for M5Utility

  SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD

  SPDX-License-Identifier: MIT
*/
#include <gtest/gtest.h>
#include <random>
#include <M5Utility.hpp>
#include "MurmurHash3.h"  // from https://github.com/rurban/smhasher/tree/master (public domain)

#if defined(ARDUINO)
#include <WString.h>
using string_t = String;
#else
#include <cstring>
using string_t = std::string;
#endif

namespace {

auto rng = std::default_random_engine{};

string_t make_random_str() {
    constexpr char dic[] =
        "abcdefghijklmnopqrstuvwxyz"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "0123456789"
        "!#$%&()=-~^|@[{]}:;+*,.<>/?_";

    string_t s;
    size_t len = 1 + (rng() & 31);
    while (len--) {
        s += dic[rng() % (sizeof(dic) / sizeof(dic[0]) - 1)];
    }
    return s;
}

using test_pair_t = std::pair<const char*, uint32_t>;

test_pair_t test_pairs[] = {
    {"", 0},
    {"M5Stack", 0x8c97d1e0U},
    {"M5Stack is a leading provider of IoT solutions.", 0x1a1eca6dU},
};

}  // namespace

TEST(MurmurHash3, endianness) {
    using namespace m5::utility::mmh3;

    constexpr char tst[] = "M5ST";
    auto big             = str2uint32<false>(tst);
    auto little          = str2uint32<true>(tst);

    EXPECT_EQ(big, 0x4d355354U);
    EXPECT_EQ(little, 0x5453354dU);
}

// User-defined literals "_mmh3"
TEST(MurmurHash3, user_defined_literals) {
    using namespace m5::utility::mmh3;

    constexpr auto h0 = ""_mmh3;
    constexpr auto h1 = "M5Stack"_mmh3;
    constexpr auto h2 = "M5Stack is a leading provider of IoT solutions."_mmh3;

    EXPECT_EQ(h0, 0U);
    EXPECT_EQ(h1, 0x8c97d1e0U);
    EXPECT_EQ(h2, 0x1a1eca6dU);
}

// Verification of value correctness
TEST(MurmurHash3, verify) {
    for (auto&& e : test_pairs) {
        auto& s = e.first;
        auto h  = m5::utility::mmh3::calculate(s);
        uint32_t h2{};
        MurmurHash3_x86_32(s, strlen(s), 0, &h2);

        EXPECT_EQ(e.second, h) << '[' << s << "] len:" << strlen(s);
        EXPECT_EQ(h, h2) << '[' << s << "] len:" << strlen(s);
    }

    constexpr int count = 10000;
    for (int i = 0; i < count; ++i) {
        string_t s = make_random_str();
        size_t len = strlen(s.c_str());
        uint32_t h1{};
        MurmurHash3_x86_32(s.c_str(), len, 0, &h1);
        uint32_t h2 = m5::utility::mmh3::calculate(s.c_str());

        EXPECT_EQ(h1, h2) << '[' << s.c_str() << "] len:" << strlen(s.c_str());
    }
}
