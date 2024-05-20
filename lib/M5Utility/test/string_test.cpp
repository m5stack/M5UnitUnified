/*
  UnitTest for M5Utility


  SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD

  SPDX-License-Identifier: MIT
*/
#include <gtest/gtest.h>
#include <M5Utility.hpp>

TEST(Utility, String) {
    std::string org = "\t\r\n\v STRING \v\n\r\t";
    std::string s   = org;
    s               = m5::utility::trim(s);  // Call trimRight/Left in it
    EXPECT_STREQ(s.c_str(), "STRING");
}

TEST(Utility, HexString) {
    {
        std::pair<uint8_t, char> table_upper[] = {
            {0, '0'},  {1, '1'},  {2, '2'},  {3, '3'},  {4, '4'},   {5, '5'},
            {6, '6'},  {7, '7'},  {8, '8'},  {9, '9'},  {10, 'A'},  {11, 'B'},
            {12, 'C'}, {13, 'D'}, {14, 'E'}, {15, 'F'}, {100, '4'},
        };
        std::pair<uint8_t, char> table_lower[] = {
            {0, '0'},  {1, '1'},  {2, '2'},  {3, '3'},  {4, '4'},   {5, '5'},
            {6, '6'},  {7, '7'},  {8, '8'},  {9, '9'},  {10, 'a'},  {11, 'b'},
            {12, 'c'}, {13, 'd'}, {14, 'e'}, {15, 'f'}, {100, '4'},
        };

        for (auto&& e : table_upper) {
            EXPECT_EQ(m5::utility::uintToHexChar<true>(e.first), e.second);
        }
        for (auto&& e : table_lower) {
            EXPECT_EQ(m5::utility::uintToHexChar<false>(e.first), e.second);
        }
    }

    std::string s;
    {
        uint8_t zero = 0;
        uint8_t v    = 0xA2;
        s            = m5::utility::unsignedToHexString(zero);
        EXPECT_STREQ(s.c_str(), "00");
        s = m5::utility::unsignedToHexString(v);
        EXPECT_STREQ(s.c_str(), "A2");
    }
    {
        uint16_t zero = 0;
        uint16_t v    = 0x0D51;
        s             = m5::utility::unsignedToHexString(zero);
        EXPECT_STREQ(s.c_str(), "0000");
        s = m5::utility::unsignedToHexString(v);
        EXPECT_STREQ(s.c_str(), "0D51");
    }
    {
        uint32_t zero = 0;
        uint32_t v    = 0xBEAF1234;
        s             = m5::utility::unsignedToHexString(zero);
        EXPECT_STREQ(s.c_str(), "00000000");
        s = m5::utility::unsignedToHexString(v);
        EXPECT_STREQ(s.c_str(), "BEAF1234");
    }
    {
        uint64_t zero = 0;
        uint64_t v    = 0x5252DEADBEAF0303;
        s             = m5::utility::unsignedToHexString(zero);
        EXPECT_STREQ(s.c_str(), "0000000000000000");
        s = m5::utility::unsignedToHexString(v);
        EXPECT_STREQ(s.c_str(), "5252DEADBEAF0303");
    }
}
