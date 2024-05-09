#include <gtest/gtest.h>
#include <m5_utility/stl/expected.hpp>

TEST(Expected, Assignment) {
  m5::stl::expected<int, int> e1 = 42;
  m5::stl::expected<int, int> e2 = 17;
  m5::stl::expected<int, int> e3 = 21;
  m5::stl::expected<int, int> e4 = m5::stl::make_unexpected(42);
  m5::stl::expected<int, int> e5 = m5::stl::make_unexpected(17);
  m5::stl::expected<int, int> e6 = m5::stl::make_unexpected(21);

  e1 = e2;
  EXPECT_TRUE(e1);
  EXPECT_TRUE(*e1 == 17);
  EXPECT_TRUE(e2);
  EXPECT_TRUE(*e2 == 17);

  e1 = std::move(e2);
  EXPECT_TRUE(e1);
  EXPECT_TRUE(*e1 == 17);
  EXPECT_TRUE(e2);
  EXPECT_TRUE(*e2 == 17);

  e1 = 42;
  EXPECT_TRUE(e1);
  EXPECT_TRUE(*e1 == 42);

  auto unex = m5::stl::make_unexpected(12);
  e1 = unex;
  EXPECT_TRUE(!e1);
  EXPECT_TRUE(e1.error() == 12);

  e1 = m5::stl::make_unexpected(42);
  EXPECT_TRUE(!e1);
  EXPECT_TRUE(e1.error() == 42);

  e1 = e3;
  EXPECT_TRUE(e1);
  EXPECT_TRUE(*e1 == 21);

  e4 = e5;
  EXPECT_TRUE(!e4);
  EXPECT_TRUE(e4.error() == 17);

  e4 = std::move(e6);
  EXPECT_TRUE(!e4);
  EXPECT_TRUE(e4.error() == 21);

  e4 = e1;
  EXPECT_TRUE(e4);
  EXPECT_TRUE(*e4 == 21);
}

TEST(Expected,AssignmentDeletion) {
  struct has_all {
    has_all() = default;
    has_all(const has_all &) = default;
    has_all(has_all &&) noexcept = default;
    has_all &operator=(const has_all &) = default;
  };

  m5::stl::expected<has_all, has_all> e1 = {};
  m5::stl::expected<has_all, has_all> e2 = {};
  e1 = e2;

  struct except_move {
    except_move() = default;
    except_move(const except_move &) = default;
    except_move(except_move &&) noexcept(false){};
    except_move &operator=(const except_move &) = default;
  };

  //m5::stl::expected<except_move, except_move> e3 = {};
  //m5::stl::expected<except_move, except_move> e4 = {};
  //e3 = e4; //should not compile
}
