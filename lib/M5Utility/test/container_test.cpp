/*
  UnitTest for M5Utility

  SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD

  SPDX-License-Identifier: MIT
*/
#include <gtest/gtest.h>
#include <M5Utility.hpp>

namespace {

using namespace m5::container;

void cb_basic_test() {
    SCOPED_TRACE("Basic");

    CircularBuffer<int, 4> rbuf;

    EXPECT_TRUE(rbuf.empty());
    EXPECT_FALSE(rbuf.full());
    EXPECT_EQ(rbuf.capacity(), 4U);
    EXPECT_EQ(rbuf.size(), 0U);
    EXPECT_FALSE(rbuf.front());
    EXPECT_FALSE(rbuf.back());
    EXPECT_FALSE(rbuf.at(0));
    EXPECT_FALSE(rbuf.at(1));
    EXPECT_FALSE(rbuf.at(2));
    EXPECT_FALSE(rbuf.at(3));

    // H
    // |   |   |   | 1 |
    //             T
    rbuf.push_front(1);
    EXPECT_FALSE(rbuf.empty());
    EXPECT_FALSE(rbuf.full());
    EXPECT_EQ(rbuf.size(), 1U);
    EXPECT_EQ(rbuf[0], 1);

    // H
    // |   |   |   |   |
    // T
    auto r = rbuf.front();
    EXPECT_TRUE(r);
    int v = *r;
    rbuf.pop_front();
    EXPECT_EQ(v, 1);
    EXPECT_TRUE(rbuf.empty());
    EXPECT_FALSE(rbuf.full());
    EXPECT_EQ(rbuf.size(), 0U);

    //     H
    // | 2 |   |   |   |
    // T
    rbuf.push_back(2);
    EXPECT_FALSE(rbuf.empty());
    EXPECT_FALSE(rbuf.full());
    EXPECT_EQ(rbuf.size(), 1U);
    EXPECT_EQ(rbuf[0], 2);

    // H
    // |   |   |   |   |
    // T
    r = rbuf.back();
    EXPECT_TRUE(r);
    v = *r;
    rbuf.pop_back();
    EXPECT_EQ(v, 2);
    EXPECT_TRUE(rbuf.empty());
    EXPECT_FALSE(rbuf.full());
    EXPECT_EQ(rbuf.size(), 0U);

    //         H
    // | 4 | 5 | 6 | 3 |
    //         T
    rbuf.push_front(3);
    EXPECT_FALSE(rbuf.empty());
    EXPECT_FALSE(rbuf.full());
    EXPECT_EQ(rbuf.size(), 1U);

    rbuf.push_back(4);
    EXPECT_FALSE(rbuf.empty());
    EXPECT_FALSE(rbuf.full());
    EXPECT_EQ(rbuf.size(), 2U);

    rbuf.push_back(5);
    EXPECT_FALSE(rbuf.empty());
    EXPECT_FALSE(rbuf.full());
    EXPECT_EQ(rbuf.size(), 3U);

    rbuf.push_front(6);
    EXPECT_FALSE(rbuf.empty());
    EXPECT_TRUE(rbuf.full());
    EXPECT_EQ(rbuf.size(), 4U);
    EXPECT_EQ(rbuf[0], 6);
    EXPECT_EQ(rbuf[1], 3);
    EXPECT_EQ(rbuf[2], 4);
    EXPECT_EQ(rbuf[3], 5);

    //     H
    // | 4 | 7 | 6 | 3 |
    //     T
    rbuf.push_front(7);
    EXPECT_FALSE(rbuf.empty());
    EXPECT_TRUE(rbuf.full());
    EXPECT_EQ(rbuf.size(), 4U);
    EXPECT_EQ(rbuf[0], 7);

    //         H
    // | 4 | 8 | 6 | 3 |
    //         T
    rbuf.push_back(8);
    EXPECT_FALSE(rbuf.empty());
    EXPECT_TRUE(rbuf.full());
    EXPECT_EQ(rbuf.size(), 4U);
    EXPECT_EQ(rbuf[0], 6);
    EXPECT_EQ(rbuf[1], 3);
    EXPECT_EQ(rbuf[2], 4);
    EXPECT_EQ(rbuf[3], 8);

    //         H
    // |   |   |   |   |
    //         T
    rbuf.pop_back();
    EXPECT_EQ(rbuf.back(), 4);
    rbuf.pop_back();
    EXPECT_EQ(rbuf.back(), 3);
    rbuf.pop_back();
    EXPECT_EQ(rbuf.back(), 6);
    rbuf.pop_back();
    EXPECT_TRUE(rbuf.empty());
    EXPECT_FALSE(rbuf.full());
    EXPECT_EQ(rbuf.size(), 0U);

    //             H
    // | 11| 12| 13| 10 |
    //             T
    rbuf.push_back(9);
    rbuf.push_back(10);
    rbuf.push_back(11);
    rbuf.push_back(12);
    rbuf.push_back(13);
    EXPECT_FALSE(rbuf.empty());
    EXPECT_TRUE(rbuf.full());
    EXPECT_EQ(rbuf.size(), 4U);
    EXPECT_EQ(rbuf[0], 10);
    EXPECT_EQ(rbuf[1], 11);
    EXPECT_EQ(rbuf[2], 12);
    EXPECT_EQ(rbuf[3], 13);

    //             H
    // |   |   | 13|   |
    //         T
    EXPECT_EQ(rbuf.front(), 10);
    rbuf.pop_front();
    EXPECT_EQ(rbuf.front(), 11);
    rbuf.pop_front();
    EXPECT_EQ(rbuf.front(), 12);
    rbuf.pop_front();
    EXPECT_FALSE(rbuf.empty());
    EXPECT_FALSE(rbuf.full());
    EXPECT_EQ(rbuf.size(), 1U);

    //         H
    // |   |   |   |   |
    //         T
    rbuf.clear();
    EXPECT_TRUE(rbuf.empty());
    EXPECT_FALSE(rbuf.full());
    EXPECT_EQ(rbuf.size(), 0U);

    // H
    // |111|111|111|111|
    // T
    rbuf.fill(111);
    EXPECT_TRUE(rbuf.full());
    EXPECT_EQ(rbuf.size(), 4U);
    EXPECT_EQ(rbuf.front(), 111);
    EXPECT_EQ(rbuf.back(), 111);
}

void cb_constructor_test() {
    SCOPED_TRACE("Constructor");

    std::vector<int> table(100);
    for (int i = 0; i < 100; ++i) {
        table[i] = i;
    }

    {
        CircularBuffer<int, 8> rbuf(2U, 52);
        EXPECT_FALSE(rbuf.empty());
        EXPECT_FALSE(rbuf.full());
        EXPECT_EQ(rbuf.capacity(), 8U);
        EXPECT_EQ(rbuf.size(), 2U);
        EXPECT_EQ(rbuf[0], 52);
        EXPECT_EQ(rbuf[1], 52);
    }
    {
        CircularBuffer<int, 8> rbuf(100U, 52);
        EXPECT_FALSE(rbuf.empty());
        EXPECT_TRUE(rbuf.full());
        EXPECT_EQ(rbuf.capacity(), 8U);
        EXPECT_EQ(rbuf.size(), 8U);
        EXPECT_EQ(rbuf.front(), 52);
        EXPECT_EQ(rbuf.back(), 52);
    }

    {
        CircularBuffer<int, 10> rbuf2({9, 8, 7, 6, 5});
        EXPECT_FALSE(rbuf2.empty());
        EXPECT_FALSE(rbuf2.full());
        EXPECT_EQ(rbuf2.capacity(), 10U);
        EXPECT_EQ(rbuf2.size(), 5U);
        EXPECT_EQ(rbuf2[0], 9);
        EXPECT_EQ(rbuf2[1], 8);
        EXPECT_EQ(rbuf2[2], 7);
        EXPECT_EQ(rbuf2[3], 6);
        EXPECT_EQ(rbuf2[4], 5);
    }
    {
        CircularBuffer<int, 8> rbuf2(table.begin(), table.end());
        EXPECT_FALSE(rbuf2.empty());
        EXPECT_TRUE(rbuf2.full());
        EXPECT_EQ(rbuf2.capacity(), 8U);
        EXPECT_EQ(rbuf2.size(), 8U);
        EXPECT_EQ(rbuf2.front(), 100 - 8);
        EXPECT_EQ(rbuf2.back(), 99);
    }

    CircularBuffer<float, 3> rbuf3 = {1.1f, 2.2f, 3.3f};
    EXPECT_FALSE(rbuf3.empty());
    EXPECT_TRUE(rbuf3.full());
    EXPECT_EQ(rbuf3.capacity(), 3U);
    EXPECT_EQ(rbuf3.size(), 3U);
    EXPECT_FLOAT_EQ(rbuf3[0], 1.1f);
    EXPECT_FLOAT_EQ(rbuf3[1], 2.2f);
    EXPECT_FLOAT_EQ(rbuf3[2], 3.3f);
}

void cb_read() {
    SCOPED_TRACE("Read");

    CircularBuffer<int, 128> rb;
    int buf[128] = {};
    size_t rcnt  = 0;

    // empty
    rcnt = rb.read(buf, 64);
    EXPECT_EQ(rcnt, 0U);

    // fill
    for (size_t i = 0; i < rb.capacity(); ++i) {
        rb.push_back(i);
    }
    EXPECT_EQ(rb.size(), 128U);

    // read half
    rcnt = rb.read(buf, 64);
    EXPECT_EQ(rcnt, 64U);
    for (int i = 0; i < 64; ++i) {
        EXPECT_EQ(buf[i], i) << i;
    }

    EXPECT_EQ(rb.size(), 128U);
    EXPECT_EQ(rb.front(), 0);
    EXPECT_EQ(rb.back(), 127);

    // pop half
    for (int i = 0; i < 64; ++i) {
        rb.pop_front();
    }

    // read
    rcnt = rb.read(buf, 64);
    EXPECT_EQ(rcnt, 64U);
    for (int i = 0; i < 64; ++i) {
        EXPECT_EQ(buf[i], i + 64) << (i + 64);
    }

    EXPECT_EQ(rb.size(), 64U);
    EXPECT_EQ(rb.front(), 64);
    EXPECT_EQ(rb.back(), 127);

    // pop half
    for (int i = 0; i < 32; ++i) {
        rb.pop_front();
    }

    // read
    rcnt = rb.read(buf, 64);
    EXPECT_EQ(rcnt, 32U);
    for (int i = 0; i < 32; ++i) {
        EXPECT_EQ(buf[i], i + 96) << (i + 96);
    }

    EXPECT_EQ(rb.size(), 32U);
    EXPECT_EQ(rb.front(), 96);
    EXPECT_EQ(rb.back(), 127);

    // push
    for (int i = 0; i < 100; ++i) {
        rb.push_back(i + 128);
    }

    // read
    rcnt = rb.read(buf, 64);
    EXPECT_EQ(rcnt, 64U);
    for (int i = 0; i < 64; ++i) {
        EXPECT_EQ(buf[i], i + 100) << (i + 100);
    }

    EXPECT_EQ(rb.size(), 128U);
    EXPECT_EQ(*rb.front(), 227 - 127);
    EXPECT_EQ(*rb.back(), 227);
}

}  // namespace

TEST(Utility, CircularBuffer) {
    cb_basic_test();
    cb_constructor_test();
    cb_read();
}
