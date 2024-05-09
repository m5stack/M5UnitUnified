#include <gtest/gtest.h>
#include <m5_utility/stl/expected.hpp>

#include <string>
#include <memory>

using std::string;

m5::stl::expected<int, string> getInt3(int val) { return val; }

m5::stl::expected<int, string> getInt2(int val) { return val; }

m5::stl::expected<int, string> getInt1() { return getInt2(5).and_then(getInt3); }

TEST(Expected, Issue1) {getInt1(); }

m5::stl::expected<int, int> operation1() { return 42; }

m5::stl::expected<std::string, int> operation2(int const val) { (void)val; return "Bananas"; }

TEST(Expected, Issue17) {
  auto const intermediate_result = operation1();

  intermediate_result.and_then(operation2);
}

struct a {};
struct b : a {};

auto doit() -> m5::stl::expected<std::unique_ptr<b>, int> {
    return m5::stl::make_unexpected(0);
}

TEST(Expected, Issue23) {
    m5::stl::expected<std::unique_ptr<a>, int> msg = doit();
    EXPECT_TRUE(!msg.has_value());    
}

TEST(Expected, Issue26) {
  m5::stl::expected<a, int> exp = m5::stl::expected<b, int>(m5::stl::unexpect, 0);
  EXPECT_TRUE(!exp.has_value());
}

struct foo {
  foo() = default;
  foo(foo &) = delete;
  foo(foo &&){};
};

TEST(Expected, Issue29) {
  std::vector<foo> v;
  v.emplace_back();
  m5::stl::expected<std::vector<foo>, int> ov = std::move(v);
  EXPECT_TRUE(ov->size() == 1);
}

m5::stl::expected<int, std::string> error() {
  return m5::stl::make_unexpected(std::string("error1 "));
}
std::string maperror(std::string s) { return s + "maperror "; }

TEST(Expected, Issue30) {
  error().map_error(maperror);
}

struct i31{
  int i;
};
TEST(Expected, Issue31) {
    const m5::stl::expected<i31, int> a = i31{42};
    (void)a->i;

    m5::stl::expected< void, std::string > result;
    m5::stl::expected< void, std::string > result2 = result;
    result2 = result;
}

TEST(Expected, Issue33) {
    m5::stl::expected<void, int> res {m5::stl::unexpect, 0};
    EXPECT_TRUE(!res);    
    res = res.map_error([](int i) { (void)i; return 42; });
    EXPECT_TRUE(res.error() == 42);
}


m5::stl::expected<void, std::string> voidWork() { return {}; }
m5::stl::expected<int, std::string> work2() { return 42; }
void errorhandling(std::string){}

TEST(Expected, Issue34) {
  m5::stl::expected <int, std::string> result = voidWork ()
      .and_then (work2);
  result.map_error ([&] (std::string result) {errorhandling (result);});
}

struct non_copyable {
	non_copyable(non_copyable&&) = default;
	non_copyable(non_copyable const&) = delete;
	non_copyable() = default;
};

TEST(Expected, Issue42) {
	m5::stl::expected<non_copyable,int>{}.map([](non_copyable) {});
}

TEST(Expected, Issue43) {
	auto result = m5::stl::expected<void, std::string>{};
	result = m5::stl::make_unexpected(std::string{ "foo" });
}

#if !(__GNUC__ <= 5)
#include <memory>

using MaybeDataPtr = m5::stl::expected<int, std::unique_ptr<int>>;

MaybeDataPtr test(int i) noexcept
{
  return std::move(i);
}

MaybeDataPtr test2(int i) noexcept
{
  return std::move(i);
}

TEST(Expected, Issue49) {
  auto m = test(10)
    .and_then(test2);
}
#endif

m5::stl::expected<int, std::unique_ptr<std::string>> func()
{
  return 1;
}

TEST(Expected, Issue61) {
  EXPECT_TRUE(func().value() == 1);
}

struct move_tracker {
        int moved = 0;

        move_tracker() = default;

        move_tracker(move_tracker const &other) noexcept {};
        move_tracker(move_tracker &&orig) noexcept
            : moved(orig.moved + 1) {}

        move_tracker &
        operator=(move_tracker const &other) noexcept {};
        
        move_tracker &operator=(move_tracker &&orig) noexcept {
          moved = orig.moved + 1;
          return *this;
        }
};

TEST(Expected, Issue122) {
     m5::stl::expected<move_tracker, int> res;
     res.emplace();
     EXPECT_TRUE(res.value().moved == 0);
}

#ifdef __cpp_deduction_guides
TEST(Expected, Issue89) {
    auto s = m5::stl::unexpected("Some string");
    EXPECT_TRUE(s.value() == std::string("Some string"));
}
#endif

namespace {
struct SS {
    int i = 0;
    int j = 0;
    SS(int i) : i(i) {}
    SS(int i, int j) : i(i), j(j) {}
};
}

TEST(Expected, Issue107) {
    m5::stl::expected<int, SS> ex1(m5::stl::unexpect, 2); 
    m5::stl::expected<int, SS> ex2(m5::stl::unexpect, 2, 2);

    EXPECT_TRUE(ex1.error().i == 2);
    EXPECT_TRUE(ex1.error().j == 0);
    EXPECT_TRUE(ex2.error().i == 2);
    EXPECT_TRUE(ex2.error().j == 2);
}

TEST(Expected, Issue129) {
  m5::stl::expected<std::unique_ptr<int>, int> x1 {std::unique_ptr<int>(new int(4))};
  m5::stl::expected<std::unique_ptr<int>, int> y1 {std::unique_ptr<int>(new int(2))};
  x1 = std::move(y1);

  EXPECT_TRUE(**x1 == 2);
}
