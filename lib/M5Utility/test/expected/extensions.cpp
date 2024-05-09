#include <gtest/gtest.h>
#include <m5_utility/stl/expected.hpp>

#define TOKENPASTE(x, y) x##y
#define TOKENPASTE2(x, y) TOKENPASTE(x, y)
#undef STATIC_REQUIRE
#define STATIC_REQUIRE(e)                                                      \
  constexpr bool TOKENPASTE2(rqure, __LINE__) = e;                             \
  (void)TOKENPASTE2(rqure, __LINE__);                                          \
  EXPECT_TRUE(e);

TEST(Expected, MapExtensions) {
  auto mul2 = [](int a) { return a * 2; };
  auto ret_void = [](int a) { (void)a; };

  {
    m5::stl::expected<int, int> e = 21;
    auto ret = e.map(mul2);
    EXPECT_TRUE(ret);
    EXPECT_TRUE(*ret == 42);
  }

  {
    const m5::stl::expected<int, int> e = 21;
    auto ret = e.map(mul2);
    EXPECT_TRUE(ret);
    EXPECT_TRUE(*ret == 42);
  }

  {
    m5::stl::expected<int, int> e = 21;
    auto ret = std::move(e).map(mul2);
    EXPECT_TRUE(ret);
    EXPECT_TRUE(*ret == 42);
  }

  {
    const m5::stl::expected<int, int> e = 21;
    auto ret = std::move(e).map(mul2);
    EXPECT_TRUE(ret);
    EXPECT_TRUE(*ret == 42);
  }

  {
    m5::stl::expected<int, int> e(m5::stl::unexpect, 21);
    auto ret = e.map(mul2);
    EXPECT_TRUE(!ret);
    EXPECT_TRUE(ret.error() == 21);
  }

  {
    const m5::stl::expected<int, int> e(m5::stl::unexpect, 21);
    auto ret = e.map(mul2);
    EXPECT_TRUE(!ret);
    EXPECT_TRUE(ret.error() == 21);
  }

  {
    m5::stl::expected<int, int> e(m5::stl::unexpect, 21);
    auto ret = std::move(e).map(mul2);
    EXPECT_TRUE(!ret);
    EXPECT_TRUE(ret.error() == 21);
  }

  {
    const m5::stl::expected<int, int> e(m5::stl::unexpect, 21);
    auto ret = std::move(e).map(mul2);
    EXPECT_TRUE(!ret);
    EXPECT_TRUE(ret.error() == 21);
  }

  {
    m5::stl::expected<int, int> e = 21;
    auto ret = e.map(ret_void);
    EXPECT_TRUE(ret);
    STATIC_REQUIRE(
        (std::is_same<decltype(ret), m5::stl::expected<void, int>>::value));
  }

  {
    const m5::stl::expected<int, int> e = 21;
    auto ret = e.map(ret_void);
    EXPECT_TRUE(ret);
    STATIC_REQUIRE(
        (std::is_same<decltype(ret), m5::stl::expected<void, int>>::value));
  }

  {
    m5::stl::expected<int, int> e = 21;
    auto ret = std::move(e).map(ret_void);
    EXPECT_TRUE(ret);
    STATIC_REQUIRE(
        (std::is_same<decltype(ret), m5::stl::expected<void, int>>::value));
  }

  {
    const m5::stl::expected<int, int> e = 21;
    auto ret = std::move(e).map(ret_void);
    EXPECT_TRUE(ret);
    STATIC_REQUIRE(
        (std::is_same<decltype(ret), m5::stl::expected<void, int>>::value));
  }

  {
    m5::stl::expected<int, int> e(m5::stl::unexpect, 21);
    auto ret = e.map(ret_void);
    EXPECT_TRUE(!ret);
    STATIC_REQUIRE(
        (std::is_same<decltype(ret), m5::stl::expected<void, int>>::value));
  }

  {
    const m5::stl::expected<int, int> e(m5::stl::unexpect, 21);
    auto ret = e.map(ret_void);
    EXPECT_TRUE(!ret);
    STATIC_REQUIRE(
        (std::is_same<decltype(ret), m5::stl::expected<void, int>>::value));
  }

  {
    m5::stl::expected<int, int> e(m5::stl::unexpect, 21);
    auto ret = std::move(e).map(ret_void);
    EXPECT_TRUE(!ret);
    STATIC_REQUIRE(
        (std::is_same<decltype(ret), m5::stl::expected<void, int>>::value));
  }

  {
    const m5::stl::expected<int, int> e(m5::stl::unexpect, 21);
    auto ret = std::move(e).map(ret_void);
    EXPECT_TRUE(!ret);
    STATIC_REQUIRE(
        (std::is_same<decltype(ret), m5::stl::expected<void, int>>::value));
  }


  // mapping functions which return references
  {
    m5::stl::expected<int, int> e(42);
    auto ret = e.map([](int& i) -> int& { return i; });
    EXPECT_TRUE(ret);
    EXPECT_TRUE(ret == 42);
  }
}

TEST(Expected, MapErrorExtensions) {
  auto mul2 = [](int a) { return a * 2; };
  auto ret_void = [](int a) { (void)a; };

  {
    m5::stl::expected<int, int> e = 21;
    auto ret = e.map_error(mul2);
    EXPECT_TRUE(ret);
    EXPECT_TRUE(*ret == 21);
  }

  {
    const m5::stl::expected<int, int> e = 21;
    auto ret = e.map_error(mul2);
    EXPECT_TRUE(ret);
    EXPECT_TRUE(*ret == 21);
  }

  {
    m5::stl::expected<int, int> e = 21;
    auto ret = std::move(e).map_error(mul2);
    EXPECT_TRUE(ret);
    EXPECT_TRUE(*ret == 21);
  }

  {
    const m5::stl::expected<int, int> e = 21;
    auto ret = std::move(e).map_error(mul2);
    EXPECT_TRUE(ret);
    EXPECT_TRUE(*ret == 21);
  }

  {
    m5::stl::expected<int, int> e(m5::stl::unexpect, 21);
    auto ret = e.map_error(mul2);
    EXPECT_TRUE(!ret);
    EXPECT_TRUE(ret.error() == 42);
  }

  {
    const m5::stl::expected<int, int> e(m5::stl::unexpect, 21);
    auto ret = e.map_error(mul2);
    EXPECT_TRUE(!ret);
    EXPECT_TRUE(ret.error() == 42);
  }

  {
    m5::stl::expected<int, int> e(m5::stl::unexpect, 21);
    auto ret = std::move(e).map_error(mul2);
    EXPECT_TRUE(!ret);
    EXPECT_TRUE(ret.error() == 42);
  }

  {
    const m5::stl::expected<int, int> e(m5::stl::unexpect, 21);
    auto ret = std::move(e).map_error(mul2);
    EXPECT_TRUE(!ret);
    EXPECT_TRUE(ret.error() == 42);
  }

  {
    m5::stl::expected<int, int> e = 21;
    auto ret = e.map_error(ret_void);
    EXPECT_TRUE(ret);
  }

  {
    const m5::stl::expected<int, int> e = 21;
    auto ret = e.map_error(ret_void);
    EXPECT_TRUE(ret);
  }

  {
    m5::stl::expected<int, int> e = 21;
    auto ret = std::move(e).map_error(ret_void);
    EXPECT_TRUE(ret);
  }

  {
    const m5::stl::expected<int, int> e = 21;
    auto ret = std::move(e).map_error(ret_void);
    EXPECT_TRUE(ret);
  }

  {
    m5::stl::expected<int, int> e(m5::stl::unexpect, 21);
    auto ret = e.map_error(ret_void);
    EXPECT_TRUE(!ret);
  }

  {
    const m5::stl::expected<int, int> e(m5::stl::unexpect, 21);
    auto ret = e.map_error(ret_void);
    EXPECT_TRUE(!ret);
  }

  {
    m5::stl::expected<int, int> e(m5::stl::unexpect, 21);
    auto ret = std::move(e).map_error(ret_void);
    EXPECT_TRUE(!ret);
  }

  {
    const m5::stl::expected<int, int> e(m5::stl::unexpect, 21);
    auto ret = std::move(e).map_error(ret_void);
    EXPECT_TRUE(!ret);
  }

}

TEST(Expected, AndThenExtensions) {
  auto succeed = [](int a) { (void)a; return m5::stl::expected<int, int>(21 * 2); };
  auto fail = [](int a) { (void)a; return m5::stl::expected<int, int>(m5::stl::unexpect, 17); };

  {
    m5::stl::expected<int, int> e = 21;
    auto ret = e.and_then(succeed);
    EXPECT_TRUE(ret);
    EXPECT_TRUE(*ret == 42);
  }

  {
    const m5::stl::expected<int, int> e = 21;
    auto ret = e.and_then(succeed);
    EXPECT_TRUE(ret);
    EXPECT_TRUE(*ret == 42);
  }

  {
    m5::stl::expected<int, int> e = 21;
    auto ret = std::move(e).and_then(succeed);
    EXPECT_TRUE(ret);
    EXPECT_TRUE(*ret == 42);
  }

  {
    const m5::stl::expected<int, int> e = 21;
    auto ret = std::move(e).and_then(succeed);
    EXPECT_TRUE(ret);
    EXPECT_TRUE(*ret == 42);
  }

  {
    m5::stl::expected<int, int> e = 21;
    auto ret = e.and_then(fail);
    EXPECT_TRUE(!ret);
    EXPECT_TRUE(ret.error() == 17);
  }

  {
    const m5::stl::expected<int, int> e = 21;
    auto ret = e.and_then(fail);
    EXPECT_TRUE(!ret);
    EXPECT_TRUE(ret.error() == 17);
  }

  {
    m5::stl::expected<int, int> e = 21;
    auto ret = std::move(e).and_then(fail);
    EXPECT_TRUE(!ret);
    EXPECT_TRUE(ret.error() == 17);
  }

  {
    const m5::stl::expected<int, int> e = 21;
    auto ret = std::move(e).and_then(fail);
    EXPECT_TRUE(!ret);
    EXPECT_TRUE(ret.error() == 17);
  }

  {
    m5::stl::expected<int, int> e(m5::stl::unexpect, 21);
    auto ret = e.and_then(succeed);
    EXPECT_TRUE(!ret);
    EXPECT_TRUE(ret.error() == 21);
  }

  {
    const m5::stl::expected<int, int> e(m5::stl::unexpect, 21);
    auto ret = e.and_then(succeed);
    EXPECT_TRUE(!ret);
    EXPECT_TRUE(ret.error() == 21);
  }

  {
    m5::stl::expected<int, int> e(m5::stl::unexpect, 21);
    auto ret = std::move(e).and_then(succeed);
    EXPECT_TRUE(!ret);
    EXPECT_TRUE(ret.error() == 21);
  }

  {
    const m5::stl::expected<int, int> e(m5::stl::unexpect, 21);
    auto ret = std::move(e).and_then(succeed);
    EXPECT_TRUE(!ret);
    EXPECT_TRUE(ret.error() == 21);
  }

  {
    m5::stl::expected<int, int> e(m5::stl::unexpect, 21);
    auto ret = e.and_then(fail);
    EXPECT_TRUE(!ret);
    EXPECT_TRUE(ret.error() == 21);
  }

  {
    const m5::stl::expected<int, int> e(m5::stl::unexpect, 21);
    auto ret = e.and_then(fail);
    EXPECT_TRUE(!ret);
    EXPECT_TRUE(ret.error() == 21);
  }

  {
    m5::stl::expected<int, int> e(m5::stl::unexpect, 21);
    auto ret = std::move(e).and_then(fail);
    EXPECT_TRUE(!ret);
    EXPECT_TRUE(ret.error() == 21);
  }

  {
    const m5::stl::expected<int, int> e(m5::stl::unexpect, 21);
    auto ret = std::move(e).and_then(fail);
    EXPECT_TRUE(!ret);
    EXPECT_TRUE(ret.error() == 21);
  }
}

TEST(Expected, or_else) {
  using eptr = std::unique_ptr<int>;
  auto succeed = [](int a) { (void)a; return m5::stl::expected<int, int>(21 * 2); };
  auto succeedptr = [](eptr e) { (void)e; return m5::stl::expected<int,eptr>(21*2);};
  auto fail =    [](int a) { (void)a; return m5::stl::expected<int,int>(m5::stl::unexpect, 17);};
  auto failptr = [](eptr e) { *e = 17;return m5::stl::expected<int,eptr>(m5::stl::unexpect, std::move(e));};
  auto failvoid = [](int) {};
  auto failvoidptr = [](const eptr&) { /* don't consume */};
  auto consumeptr = [](eptr) {};
  auto make_u_int = [](int n) { return std::unique_ptr<int>(new int(n));};

  {
    m5::stl::expected<int, int> e = 21;
    auto ret = e.or_else(succeed);
    EXPECT_TRUE(ret);
    EXPECT_TRUE(*ret == 21);
  }

  {
    const m5::stl::expected<int, int> e = 21;
    auto ret = e.or_else(succeed);
    EXPECT_TRUE(ret);
    EXPECT_TRUE(*ret == 21);
  }

  {
    m5::stl::expected<int, int> e = 21;
    auto ret = std::move(e).or_else(succeed);
    EXPECT_TRUE(ret);
    EXPECT_TRUE(*ret == 21);
  }

  {
    m5::stl::expected<int, eptr> e = 21;
    auto ret = std::move(e).or_else(succeedptr);
    EXPECT_TRUE(ret);
    EXPECT_TRUE(*ret == 21);
  }

  {
    const m5::stl::expected<int, int> e = 21;
    auto ret = std::move(e).or_else(succeed);
    EXPECT_TRUE(ret);
    EXPECT_TRUE(*ret == 21);
  }

  {
    m5::stl::expected<int, int> e = 21;
    auto ret = e.or_else(fail);
    EXPECT_TRUE(ret);
    EXPECT_TRUE(*ret == 21);
  }

  {
    const m5::stl::expected<int, int> e = 21;
    auto ret = e.or_else(fail);
    EXPECT_TRUE(ret);
    EXPECT_TRUE(*ret == 21);
  }

  {
    m5::stl::expected<int, int> e = 21;
    auto ret = std::move(e).or_else(fail);
    EXPECT_TRUE(ret);
    EXPECT_TRUE(ret == 21);
  }


  {
    m5::stl::expected<int, eptr> e = 21;
    auto ret = std::move(e).or_else(failptr);
    EXPECT_TRUE(ret);
    EXPECT_TRUE(ret == 21);
  }

  {
    const m5::stl::expected<int, int> e = 21;
    auto ret = std::move(e).or_else(fail);
    EXPECT_TRUE(ret);
    EXPECT_TRUE(*ret == 21);
  }

  {
    m5::stl::expected<int, int> e(m5::stl::unexpect, 21);
    auto ret = e.or_else(succeed);
    EXPECT_TRUE(ret);
    EXPECT_TRUE(*ret == 42);
  }

  {
    const m5::stl::expected<int, int> e(m5::stl::unexpect, 21);
    auto ret = e.or_else(succeed);
    EXPECT_TRUE(ret);
    EXPECT_TRUE(*ret == 42);
  }

  {
    m5::stl::expected<int, int> e(m5::stl::unexpect, 21);
    auto ret = std::move(e).or_else(succeed);
    EXPECT_TRUE(ret);
    EXPECT_TRUE(*ret == 42);
  }

  {
    m5::stl::expected<int, eptr> e(m5::stl::unexpect, make_u_int(21));
    auto ret = std::move(e).or_else(succeedptr);
    EXPECT_TRUE(ret);
    EXPECT_TRUE(*ret == 42);
  }

  {
    const m5::stl::expected<int, int> e(m5::stl::unexpect, 21);
    auto ret = std::move(e).or_else(succeed);
    EXPECT_TRUE(ret);
    EXPECT_TRUE(*ret == 42);
  }

  {
    m5::stl::expected<int, int> e(m5::stl::unexpect, 21);
    auto ret = e.or_else(fail);
    EXPECT_TRUE(!ret);
    EXPECT_TRUE(ret.error() == 17);
  }

  {
    m5::stl::expected<int, int> e(m5::stl::unexpect, 21);
    auto ret = e.or_else(failvoid);
    EXPECT_TRUE(!ret);
    EXPECT_TRUE(ret.error() == 21);
  }

  {
    const m5::stl::expected<int, int> e(m5::stl::unexpect, 21);
    auto ret = e.or_else(fail);
    EXPECT_TRUE(!ret);
    EXPECT_TRUE(ret.error() == 17);
  }

  {
    const m5::stl::expected<int, int> e(m5::stl::unexpect, 21);
    auto ret = e.or_else(failvoid);
    EXPECT_TRUE(!ret);
    EXPECT_TRUE(ret.error() == 21);
  }

  {
    m5::stl::expected<int, int> e(m5::stl::unexpect, 21);
    auto ret = std::move(e).or_else(fail);
    EXPECT_TRUE(!ret);
    EXPECT_TRUE(ret.error() == 17);
  }

  {
    m5::stl::expected<int, int> e(m5::stl::unexpect, 21);
    auto ret = std::move(e).or_else(failvoid);
    EXPECT_TRUE(!ret);
    EXPECT_TRUE(ret.error() == 21);
  }

  {
    m5::stl::expected<int, eptr> e(m5::stl::unexpect, make_u_int(21));
    auto ret = std::move(e).or_else(failvoidptr);
    EXPECT_TRUE(!ret);
    EXPECT_TRUE(*ret.error() == 21);
  }

  {
    m5::stl::expected<int, eptr> e(m5::stl::unexpect, make_u_int(21));
    auto ret = std::move(e).or_else(consumeptr);
    EXPECT_TRUE(!ret);
    EXPECT_TRUE(ret.error() == nullptr);
  }

  {
    const m5::stl::expected<int, int> e(m5::stl::unexpect, 21);
    auto ret = std::move(e).or_else(fail);
    EXPECT_TRUE(!ret);
    EXPECT_TRUE(ret.error() == 17);
  }

  {
    const m5::stl::expected<int, int> e(m5::stl::unexpect, 21);
    auto ret = std::move(e).or_else(failvoid);
    EXPECT_TRUE(!ret);
    EXPECT_TRUE(ret.error() == 21);
  }

}

TEST(Expected, TransformExtensions) {
  auto mul2 = [](int a) { return a * 2; };
  auto ret_void = [](int a) { (void)a; };

  {
    m5::stl::expected<int, int> e = 21;
    auto ret = e.transform(mul2);
    EXPECT_TRUE(ret);
    EXPECT_TRUE(*ret == 42);
  }

  {
    const m5::stl::expected<int, int> e = 21;
    auto ret = e.transform(mul2);
    EXPECT_TRUE(ret);
    EXPECT_TRUE(*ret == 42);
  }

  {
    m5::stl::expected<int, int> e = 21;
    auto ret = std::move(e).transform(mul2);
    EXPECT_TRUE(ret);
    EXPECT_TRUE(*ret == 42);
  }

  {
    const m5::stl::expected<int, int> e = 21;
    auto ret = std::move(e).transform(mul2);
    EXPECT_TRUE(ret);
    EXPECT_TRUE(*ret == 42);
  }

  {
    m5::stl::expected<int, int> e(m5::stl::unexpect, 21);
    auto ret = e.transform(mul2);
    EXPECT_TRUE(!ret);
    EXPECT_TRUE(ret.error() == 21);
  }

  {
    const m5::stl::expected<int, int> e(m5::stl::unexpect, 21);
    auto ret = e.transform(mul2);
    EXPECT_TRUE(!ret);
    EXPECT_TRUE(ret.error() == 21);
  }

  {
    m5::stl::expected<int, int> e(m5::stl::unexpect, 21);
    auto ret = std::move(e).transform(mul2);
    EXPECT_TRUE(!ret);
    EXPECT_TRUE(ret.error() == 21);
  }

  {
    const m5::stl::expected<int, int> e(m5::stl::unexpect, 21);
    auto ret = std::move(e).transform(mul2);
    EXPECT_TRUE(!ret);
    EXPECT_TRUE(ret.error() == 21);
  }

  {
    m5::stl::expected<int, int> e = 21;
    auto ret = e.transform(ret_void);
    EXPECT_TRUE(ret);
    STATIC_REQUIRE(
        (std::is_same<decltype(ret), m5::stl::expected<void, int>>::value));
  }

  {
    const m5::stl::expected<int, int> e = 21;
    auto ret = e.transform(ret_void);
    EXPECT_TRUE(ret);
    STATIC_REQUIRE(
        (std::is_same<decltype(ret), m5::stl::expected<void, int>>::value));
  }

  {
    m5::stl::expected<int, int> e = 21;
    auto ret = std::move(e).transform(ret_void);
    EXPECT_TRUE(ret);
    STATIC_REQUIRE(
        (std::is_same<decltype(ret), m5::stl::expected<void, int>>::value));
  }

  {
    const m5::stl::expected<int, int> e = 21;
    auto ret = std::move(e).transform(ret_void);
    EXPECT_TRUE(ret);
    STATIC_REQUIRE(
        (std::is_same<decltype(ret), m5::stl::expected<void, int>>::value));
  }

  {
    m5::stl::expected<int, int> e(m5::stl::unexpect, 21);
    auto ret = e.transform(ret_void);
    EXPECT_TRUE(!ret);
    STATIC_REQUIRE(
        (std::is_same<decltype(ret), m5::stl::expected<void, int>>::value));
  }

  {
    const m5::stl::expected<int, int> e(m5::stl::unexpect, 21);
    auto ret = e.transform(ret_void);
    EXPECT_TRUE(!ret);
    STATIC_REQUIRE(
        (std::is_same<decltype(ret), m5::stl::expected<void, int>>::value));
  }

  {
    m5::stl::expected<int, int> e(m5::stl::unexpect, 21);
    auto ret = std::move(e).transform(ret_void);
    EXPECT_TRUE(!ret);
    STATIC_REQUIRE(
        (std::is_same<decltype(ret), m5::stl::expected<void, int>>::value));
  }

  {
    const m5::stl::expected<int, int> e(m5::stl::unexpect, 21);
    auto ret = std::move(e).transform(ret_void);
    EXPECT_TRUE(!ret);
    STATIC_REQUIRE(
        (std::is_same<decltype(ret), m5::stl::expected<void, int>>::value));
  }


  // mapping functions which return references
  {
    m5::stl::expected<int, int> e(42);
    auto ret = e.transform([](int& i) -> int& { return i; });
    EXPECT_TRUE(ret);
    EXPECT_TRUE(ret == 42);
  }
}

TEST(Expected, TransformErrorExtensions) {
  auto mul2 = [](int a) { return a * 2; };
  auto ret_void = [](int a) { (void)a; };

  {
    m5::stl::expected<int, int> e = 21;
    auto ret = e.transform_error(mul2);
    EXPECT_TRUE(ret);
    EXPECT_TRUE(*ret == 21);
  }

  {
    const m5::stl::expected<int, int> e = 21;
    auto ret = e.transform_error(mul2);
    EXPECT_TRUE(ret);
    EXPECT_TRUE(*ret == 21);
  }

  {
    m5::stl::expected<int, int> e = 21;
    auto ret = std::move(e).transform_error(mul2);
    EXPECT_TRUE(ret);
    EXPECT_TRUE(*ret == 21);
  }

  {
    const m5::stl::expected<int, int> e = 21;
    auto ret = std::move(e).transform_error(mul2);
    EXPECT_TRUE(ret);
    EXPECT_TRUE(*ret == 21);
  }

  {
    m5::stl::expected<int, int> e(m5::stl::unexpect, 21);
    auto ret = e.transform_error(mul2);
    EXPECT_TRUE(!ret);
    EXPECT_TRUE(ret.error() == 42);
  }

  {
    const m5::stl::expected<int, int> e(m5::stl::unexpect, 21);
    auto ret = e.transform_error(mul2);
    EXPECT_TRUE(!ret);
    EXPECT_TRUE(ret.error() == 42);
  }

  {
    m5::stl::expected<int, int> e(m5::stl::unexpect, 21);
    auto ret = std::move(e).transform_error(mul2);
    EXPECT_TRUE(!ret);
    EXPECT_TRUE(ret.error() == 42);
  }

  {
    const m5::stl::expected<int, int> e(m5::stl::unexpect, 21);
    auto ret = std::move(e).transform_error(mul2);
    EXPECT_TRUE(!ret);
    EXPECT_TRUE(ret.error() == 42);
  }

  {
    m5::stl::expected<int, int> e = 21;
    auto ret = e.transform_error(ret_void);
    EXPECT_TRUE(ret);
  }

  {
    const m5::stl::expected<int, int> e = 21;
    auto ret = e.transform_error(ret_void);
    EXPECT_TRUE(ret);
  }

  {
    m5::stl::expected<int, int> e = 21;
    auto ret = std::move(e).transform_error(ret_void);
    EXPECT_TRUE(ret);
  }

  {
    const m5::stl::expected<int, int> e = 21;
    auto ret = std::move(e).transform_error(ret_void);
    EXPECT_TRUE(ret);
  }

  {
    m5::stl::expected<int, int> e(m5::stl::unexpect, 21);
    auto ret = e.transform_error(ret_void);
    EXPECT_TRUE(!ret);
  }

  {
    const m5::stl::expected<int, int> e(m5::stl::unexpect, 21);
    auto ret = e.transform_error(ret_void);
    EXPECT_TRUE(!ret);
  }

  {
    m5::stl::expected<int, int> e(m5::stl::unexpect, 21);
    auto ret = std::move(e).transform_error(ret_void);
    EXPECT_TRUE(!ret);
  }

  {
    const m5::stl::expected<int, int> e(m5::stl::unexpect, 21);
    auto ret = std::move(e).transform_error(ret_void);
    EXPECT_TRUE(!ret);
  }

}

struct S {
    int x;
};

struct F {
    int x;
};

TEST(Expected, issue14) {
    auto res = m5::stl::expected<S,F>{m5::stl::unexpect, F{}};

    res.map_error([](F f) {
        (void)f;
    });
}

TEST(Expected, issue32) {
    int i = 0;
    m5::stl::expected<void, int> a;
    a.map([&i]{i = 42;});
    EXPECT_TRUE(i == 42);

    auto x = a.map([]{return 42;});
    EXPECT_TRUE(*x == 42);
}
