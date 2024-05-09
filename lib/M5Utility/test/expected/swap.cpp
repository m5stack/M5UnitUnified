#include <gtest/gtest.h>
#include <m5_utility/stl/expected.hpp>

struct no_throw {
  no_throw(std::string i) : i(i) {}
  std::string i;
};
struct canthrow_move {
  canthrow_move(std::string i) : i(i) {}
  canthrow_move(canthrow_move const &) = default;
  canthrow_move(canthrow_move &&other) noexcept(false) : i(other.i) {}
  canthrow_move &operator=(canthrow_move &&) = default;
  std::string i;
};

bool should_throw = false;

#ifdef TL_EXPECTED_EXCEPTIONS_ENABLED
struct willthrow_move {
  willthrow_move(std::string i) : i(i) {}
  willthrow_move(willthrow_move const &) = default;
  willthrow_move(willthrow_move &&other) : i(other.i) {
    if (should_throw)
      throw 0;
  }
  willthrow_move &operator=(willthrow_move &&) = default;
  std::string i;
};
#endif // TL_EXPECTED_EXCEPTIONS_ENABLED

static_assert(m5::stl::detail::is_swappable<no_throw>::value, "");

template <class T1, class T2> void swap_test() {
  std::string s1 = "abcdefghijklmnopqrstuvwxyz";
  std::string s2 = "zyxwvutsrqponmlkjihgfedcba";

  m5::stl::expected<T1, T2> a{s1};
  m5::stl::expected<T1, T2> b{s2};
  swap(a, b);
  EXPECT_TRUE(a->i == s2);
  EXPECT_TRUE(b->i == s1);

  a = s1;
  b = m5::stl::unexpected<T2>(s2);
  swap(a, b);
  EXPECT_TRUE(a.error().i == s2);
  EXPECT_TRUE(b->i == s1);

  a = m5::stl::unexpected<T2>(s1);
  b = s2;
  swap(a, b);
  EXPECT_TRUE(a->i == s2);
  EXPECT_TRUE(b.error().i == s1);

  a = m5::stl::unexpected<T2>(s1);
  b = m5::stl::unexpected<T2>(s2);
  swap(a, b);
  EXPECT_TRUE(a.error().i == s2);
  EXPECT_TRUE(b.error().i == s1);

  a = s1;
  b = s2;
  a.swap(b);
  EXPECT_TRUE(a->i == s2);
  EXPECT_TRUE(b->i == s1);

  a = s1;
  b = m5::stl::unexpected<T2>(s2);
  a.swap(b);
  EXPECT_TRUE(a.error().i == s2);
  EXPECT_TRUE(b->i == s1);

  a = m5::stl::unexpected<T2>(s1);
  b = s2;
  a.swap(b);
  EXPECT_TRUE(a->i == s2);
  EXPECT_TRUE(b.error().i == s1);

  a = m5::stl::unexpected<T2>(s1);
  b = m5::stl::unexpected<T2>(s2);
  a.swap(b);
  EXPECT_TRUE(a.error().i == s2);
  EXPECT_TRUE(b.error().i == s1);
}

#ifdef TL_EXPECTED_EXCEPTIONS_ENABLED
TEST(Expected, swap) {

  {
    SCOPED_TRACE("no_throw no-throw");
    swap_test<no_throw, no_throw>();
  }
  {
    SCOPED_TRACE("no_throw canthrow");
    swap_test<no_throw, canthrow_move>();
  }
  {
    SCOPED_TRACE("canthrow no_throw");
    swap_test<canthrow_move, no_throw>();
  }

  std::string s1 = "abcdefghijklmnopqrstuvwxyz";
  std::string s2 = "zyxwvutsrqponmlkjihgfedcbaxxx";
  m5::stl::expected<no_throw, willthrow_move> a{s1};
  m5::stl::expected<no_throw, willthrow_move> b{m5::stl::unexpect, s2};
  should_throw = 1;

  #ifdef _MSC_VER
  //this seems to break catch on GCC and Clang
  EXPECT_TRUE_THROWS(swap(a, b));
  #endif

  EXPECT_TRUE(a->i == s1);
  EXPECT_TRUE(b.error().i == s2);
}
#endif // TL_EXPECTED_EXCEPTIONS_ENABLED
