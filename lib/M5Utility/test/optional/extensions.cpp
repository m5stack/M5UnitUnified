#include <gtest/gtest.h>
#include <m5_utility/stl/optional.hpp>
#include <string>
#include "helper.hpp"

constexpr int get_int(int) { return 42; }
TL_OPTIONAL_11_CONSTEXPR m5::stl::optional<int> get_opt_int(int) { return 42; }

// What is Clang Format up to?!
TEST(Optional,Monadic) {
  { // lhs is empty
    SCOPED_TRACE("map");
    m5::stl::optional<int> o1;
    auto o1r = o1.map([](int i) { return i + 2; });
    STATIC_EXPECT_TRUE((std::is_same<decltype(o1r), m5::stl::optional<int>>::value));
    EXPECT_TRUE(!o1r);

    // lhs has value
    m5::stl::optional<int> o2 = 40;
    auto o2r = o2.map([](int i) { return i + 2; });
    STATIC_EXPECT_TRUE((std::is_same<decltype(o2r), m5::stl::optional<int>>::value));
    EXPECT_TRUE(o2r.value() == 42);

    struct rval_call_map {
      double operator()(int) && { return 42.0; };
    };

    // ensure that function object is forwarded
    m5::stl::optional<int> o3 = 42;
    auto o3r = o3.map(rval_call_map{});
    STATIC_EXPECT_TRUE((std::is_same<decltype(o3r), m5::stl::optional<double>>::value));
    EXPECT_TRUE(o3r.value() == 42);

    // ensure that lhs is forwarded
    m5::stl::optional<int> o4 = 40;
    auto o4r = std::move(o4).map([](int &&i) { return i + 2; });
    STATIC_EXPECT_TRUE((std::is_same<decltype(o4r), m5::stl::optional<int>>::value));
    EXPECT_TRUE(o4r.value() == 42);

    // ensure that lhs is const-propagated
    const m5::stl::optional<int> o5 = 40;
    auto o5r = o5.map([](const int &i) { return i + 2; });
    STATIC_EXPECT_TRUE((std::is_same<decltype(o5r), m5::stl::optional<int>>::value));
    EXPECT_TRUE(o5r.value() == 42);

    // test void return
    m5::stl::optional<int> o7 = 40;
    auto f7 = [](const int &) { return; };
    auto o7r = o7.map(f7);
    STATIC_EXPECT_TRUE(
        (std::is_same<decltype(o7r), m5::stl::optional<m5::stl::monostate>>::value));
    EXPECT_TRUE(o7r.has_value());

    // test each overload in turn
    m5::stl::optional<int> o8 = 42;
    auto o8r = o8.map([](int) { return 42; });
    EXPECT_TRUE(*o8r == 42);

    m5::stl::optional<int> o9 = 42;
    auto o9r = o9.map([](int) { return; });
    EXPECT_TRUE(o9r);

    m5::stl::optional<int> o12 = 42;
    auto o12r = std::move(o12).map([](int) { return 42; });
    EXPECT_TRUE(*o12r == 42);

    m5::stl::optional<int> o13 = 42;
    auto o13r = std::move(o13).map([](int) { return; });
    EXPECT_TRUE(o13r);

    const m5::stl::optional<int> o16 = 42;
    auto o16r = o16.map([](int) { return 42; });
    EXPECT_TRUE(*o16r == 42);

    const m5::stl::optional<int> o17 = 42;
    auto o17r = o17.map([](int) { return; });
    EXPECT_TRUE(o17r);

    const m5::stl::optional<int> o20 = 42;
    auto o20r = std::move(o20).map([](int) { return 42; });
    EXPECT_TRUE(*o20r == 42);

    const m5::stl::optional<int> o21 = 42;
    auto o21r = std::move(o21).map([](int) { return; });
    EXPECT_TRUE(o21r);

    m5::stl::optional<int> o24 = m5::stl::nullopt;
    auto o24r = o24.map([](int) { return 42; });
    EXPECT_TRUE(!o24r);

    m5::stl::optional<int> o25 = m5::stl::nullopt;
    auto o25r = o25.map([](int) { return; });
    EXPECT_TRUE(!o25r);

    m5::stl::optional<int> o28 = m5::stl::nullopt;
    auto o28r = std::move(o28).map([](int) { return 42; });
    EXPECT_TRUE(!o28r);

    m5::stl::optional<int> o29 = m5::stl::nullopt;
    auto o29r = std::move(o29).map([](int) { return; });
    EXPECT_TRUE(!o29r);

    const m5::stl::optional<int> o32 = m5::stl::nullopt;
    auto o32r = o32.map([](int) { return 42; });
    EXPECT_TRUE(!o32r);

    const m5::stl::optional<int> o33 = m5::stl::nullopt;
    auto o33r = o33.map([](int) { return; });
    EXPECT_TRUE(!o33r);

    const m5::stl::optional<int> o36 = m5::stl::nullopt;
    auto o36r = std::move(o36).map([](int) { return 42; });
    EXPECT_TRUE(!o36r);

    const m5::stl::optional<int> o37 = m5::stl::nullopt;
    auto o37r = std::move(o37).map([](int) { return; });
    EXPECT_TRUE(!o37r);

    // callable which returns a reference
    m5::stl::optional<int> o38 = 42;
    auto o38r = o38.map([](int &i) -> const int & { return i; });
    EXPECT_TRUE(o38r);
    EXPECT_TRUE(*o38r == 42);

    int i = 42;
    m5::stl::optional<int&> o39 = i;
    o39.map([](int& x){x = 12;});
    EXPECT_TRUE(i == 12);
  }
  {
    SCOPED_TRACE("map constexpr");
#if !defined(_MSC_VER) && defined(TL_OPTIONAL_CXX14)
    // test each overload in turn
    constexpr m5::stl::optional<int> o16 = 42;
    constexpr auto o16r = o16.map(get_int);
    STATIC_EXPECT_TRUE(*o16r == 42);

    constexpr m5::stl::optional<int> o20 = 42;
    constexpr auto o20r = std::move(o20).map(get_int);
    STATIC_EXPECT_TRUE(*o20r == 42);

    constexpr m5::stl::optional<int> o32 = m5::stl::nullopt;
    constexpr auto o32r = o32.map(get_int);
    STATIC_EXPECT_TRUE(!o32r);
    constexpr m5::stl::optional<int> o36 = m5::stl::nullopt;
    constexpr auto o36r = std::move(o36).map(get_int);
    STATIC_EXPECT_TRUE(!o36r);
#endif
  }
  {// lhs is empty
    SCOPED_TRACE("transform");
    m5::stl::optional<int> o1;
    auto o1r = o1.transform([](int i) { return i + 2; });
    STATIC_EXPECT_TRUE((std::is_same<decltype(o1r), m5::stl::optional<int>>::value));
    EXPECT_TRUE(!o1r);

    // lhs has value
    m5::stl::optional<int> o2 = 40;
    auto o2r = o2.transform([](int i) { return i + 2; });
    STATIC_EXPECT_TRUE((std::is_same<decltype(o2r), m5::stl::optional<int>>::value));
    EXPECT_TRUE(o2r.value() == 42);

    struct rval_call_transform {
      double operator()(int) && { return 42.0; };
    };

    // ensure that function object is forwarded
    m5::stl::optional<int> o3 = 42;
    auto o3r = o3.transform(rval_call_transform{});
    STATIC_EXPECT_TRUE((std::is_same<decltype(o3r), m5::stl::optional<double>>::value));
    EXPECT_TRUE(o3r.value() == 42);

    // ensure that lhs is forwarded
    m5::stl::optional<int> o4 = 40;
    auto o4r = std::move(o4).transform([](int&& i) { return i + 2; });
    STATIC_EXPECT_TRUE((std::is_same<decltype(o4r), m5::stl::optional<int>>::value));
    EXPECT_TRUE(o4r.value() == 42);

    // ensure that lhs is const-propagated
    const m5::stl::optional<int> o5 = 40;
    auto o5r = o5.transform([](const int& i) { return i + 2; });
    STATIC_EXPECT_TRUE((std::is_same<decltype(o5r), m5::stl::optional<int>>::value));
    EXPECT_TRUE(o5r.value() == 42);

    // test void return
    m5::stl::optional<int> o7 = 40;
    auto f7 = [](const int&) { return; };
    auto o7r = o7.transform(f7);
    STATIC_EXPECT_TRUE(
      (std::is_same<decltype(o7r), m5::stl::optional<m5::stl::monostate>>::value));
    EXPECT_TRUE(o7r.has_value());

    // test each overload in turn
    m5::stl::optional<int> o8 = 42;
    auto o8r = o8.transform([](int) { return 42; });
    EXPECT_TRUE(*o8r == 42);

    m5::stl::optional<int> o9 = 42;
    auto o9r = o9.transform([](int) { return; });
    EXPECT_TRUE(o9r);

    m5::stl::optional<int> o12 = 42;
    auto o12r = std::move(o12).transform([](int) { return 42; });
    EXPECT_TRUE(*o12r == 42);

    m5::stl::optional<int> o13 = 42;
    auto o13r = std::move(o13).transform([](int) { return; });
    EXPECT_TRUE(o13r);

    const m5::stl::optional<int> o16 = 42;
    auto o16r = o16.transform([](int) { return 42; });
    EXPECT_TRUE(*o16r == 42);

    const m5::stl::optional<int> o17 = 42;
    auto o17r = o17.transform([](int) { return; });
    EXPECT_TRUE(o17r);

    const m5::stl::optional<int> o20 = 42;
    auto o20r = std::move(o20).transform([](int) { return 42; });
    EXPECT_TRUE(*o20r == 42);

    const m5::stl::optional<int> o21 = 42;
    auto o21r = std::move(o21).transform([](int) { return; });
    EXPECT_TRUE(o21r);

    m5::stl::optional<int> o24 = m5::stl::nullopt;
    auto o24r = o24.transform([](int) { return 42; });
    EXPECT_TRUE(!o24r);

    m5::stl::optional<int> o25 = m5::stl::nullopt;
    auto o25r = o25.transform([](int) { return; });
    EXPECT_TRUE(!o25r);

    m5::stl::optional<int> o28 = m5::stl::nullopt;
    auto o28r = std::move(o28).transform([](int) { return 42; });
    EXPECT_TRUE(!o28r);

    m5::stl::optional<int> o29 = m5::stl::nullopt;
    auto o29r = std::move(o29).transform([](int) { return; });
    EXPECT_TRUE(!o29r);

    const m5::stl::optional<int> o32 = m5::stl::nullopt;
    auto o32r = o32.transform([](int) { return 42; });
    EXPECT_TRUE(!o32r);

    const m5::stl::optional<int> o33 = m5::stl::nullopt;
    auto o33r = o33.transform([](int) { return; });
    EXPECT_TRUE(!o33r);

    const m5::stl::optional<int> o36 = m5::stl::nullopt;
    auto o36r = std::move(o36).transform([](int) { return 42; });
    EXPECT_TRUE(!o36r);

    const m5::stl::optional<int> o37 = m5::stl::nullopt;
    auto o37r = std::move(o37).transform([](int) { return; });
    EXPECT_TRUE(!o37r);

    // callable which returns a reference
    m5::stl::optional<int> o38 = 42;
    auto o38r = o38.transform([](int& i) -> const int& { return i; });
    EXPECT_TRUE(o38r);
    EXPECT_TRUE(*o38r == 42);

    int i = 42;
    m5::stl::optional<int&> o39 = i;
    o39.transform([](int& x) {x = 12; });
    EXPECT_TRUE(i == 12);
  }
  {
    SCOPED_TRACE("transform constexpr");
#if !defined(_MSC_VER) && defined(TL_OPTIONAL_CXX14)
    // test each overload in turn
    constexpr m5::stl::optional<int> o16 = 42;
    constexpr auto o16r = o16.transform(get_int);
    STATIC_EXPECT_TRUE(*o16r == 42);

    constexpr m5::stl::optional<int> o20 = 42;
    constexpr auto o20r = std::move(o20).transform(get_int);
    STATIC_EXPECT_TRUE(*o20r == 42);

    constexpr m5::stl::optional<int> o32 = m5::stl::nullopt;
    constexpr auto o32r = o32.transform(get_int);
    STATIC_EXPECT_TRUE(!o32r);
    constexpr m5::stl::optional<int> o36 = m5::stl::nullopt;
    constexpr auto o36r = std::move(o36).transform(get_int);
    STATIC_EXPECT_TRUE(!o36r);
#endif
  }
  {
    SCOPED_TRACE("and_then");

    // lhs is empty
    m5::stl::optional<int> o1;
    auto o1r = o1.and_then([](int) { return m5::stl::optional<float>{42}; });
    STATIC_EXPECT_TRUE((std::is_same<decltype(o1r), m5::stl::optional<float>>::value));
    EXPECT_TRUE(!o1r);

    // lhs has value
    m5::stl::optional<int> o2 = 12;
    auto o2r = o2.and_then([](int) { return m5::stl::optional<float>{42}; });
    STATIC_EXPECT_TRUE((std::is_same<decltype(o2r), m5::stl::optional<float>>::value));
    EXPECT_TRUE(o2r.value() == 42.f);

    // lhs is empty, rhs returns empty
    m5::stl::optional<int> o3;
    auto o3r = o3.and_then([](int) { return m5::stl::optional<float>{}; });
    STATIC_EXPECT_TRUE((std::is_same<decltype(o3r), m5::stl::optional<float>>::value));
    EXPECT_TRUE(!o3r);

    // rhs returns empty
    m5::stl::optional<int> o4 = 12;
    auto o4r = o4.and_then([](int) { return m5::stl::optional<float>{}; });
    STATIC_EXPECT_TRUE((std::is_same<decltype(o4r), m5::stl::optional<float>>::value));
    EXPECT_TRUE(!o4r);

    struct rval_call_and_then {
      m5::stl::optional<double> operator()(int) && {
        return m5::stl::optional<double>(42.0);
      };
    };

    // ensure that function object is forwarded
    m5::stl::optional<int> o5 = 42;
    auto o5r = o5.and_then(rval_call_and_then{});
    STATIC_EXPECT_TRUE((std::is_same<decltype(o5r), m5::stl::optional<double>>::value));
    EXPECT_TRUE(o5r.value() == 42);

    // ensure that lhs is forwarded
    m5::stl::optional<int> o6 = 42;
    auto o6r =
        std::move(o6).and_then([](int &&i) { return m5::stl::optional<double>(i); });
    STATIC_EXPECT_TRUE((std::is_same<decltype(o6r), m5::stl::optional<double>>::value));
    EXPECT_TRUE(o6r.value() == 42);

    // ensure that function object is const-propagated
    const m5::stl::optional<int> o7 = 42;
    auto o7r =
        o7.and_then([](const int &i) { return m5::stl::optional<double>(i); });
    STATIC_EXPECT_TRUE((std::is_same<decltype(o7r), m5::stl::optional<double>>::value));
    EXPECT_TRUE(o7r.value() == 42);

    // test each overload in turn
    m5::stl::optional<int> o8 = 42;
    auto o8r = o8.and_then([](int) { return m5::stl::make_optional(42); });
    EXPECT_TRUE(*o8r == 42);

    m5::stl::optional<int> o9 = 42;
    auto o9r =
        std::move(o9).and_then([](int) { return m5::stl::make_optional(42); });
    EXPECT_TRUE(*o9r == 42);

    const m5::stl::optional<int> o10 = 42;
    auto o10r = o10.and_then([](int) { return m5::stl::make_optional(42); });
    EXPECT_TRUE(*o10r == 42);

    const m5::stl::optional<int> o11 = 42;
    auto o11r =
        std::move(o11).and_then([](int) { return m5::stl::make_optional(42); });
    EXPECT_TRUE(*o11r == 42);

    m5::stl::optional<int> o16 = m5::stl::nullopt;
    auto o16r = o16.and_then([](int) { return m5::stl::make_optional(42); });
    EXPECT_TRUE(!o16r);

    m5::stl::optional<int> o17 = m5::stl::nullopt;
    auto o17r =
        std::move(o17).and_then([](int) { return m5::stl::make_optional(42); });
    EXPECT_TRUE(!o17r);

    const m5::stl::optional<int> o18 = m5::stl::nullopt;
    auto o18r = o18.and_then([](int) { return m5::stl::make_optional(42); });
    EXPECT_TRUE(!o18r);

    const m5::stl::optional<int> o19 = m5::stl::nullopt;
    auto o19r = std::move(o19).and_then([](int) { return m5::stl::make_optional(42); });
    EXPECT_TRUE(!o19r);

    int i = 3;
    m5::stl::optional<int&> o20{i};
    std::move(o20).and_then([](int& r){return m5::stl::optional<int&>{++r};});
    EXPECT_TRUE(o20);
    EXPECT_TRUE(i == 4);
  }
  {
    SCOPED_TRACE("constexpr and_then");
#if !defined(_MSC_VER) && defined(TL_OPTIONAL_CXX14)

    constexpr m5::stl::optional<int> o10 = 42;
    constexpr auto o10r = o10.and_then(get_opt_int);
    EXPECT_TRUE(*o10r == 42);

    constexpr m5::stl::optional<int> o11 = 42;
    constexpr auto o11r = std::move(o11).and_then(get_opt_int);
    EXPECT_TRUE(*o11r == 42);

    constexpr m5::stl::optional<int> o18 = m5::stl::nullopt;
    constexpr auto o18r = o18.and_then(get_opt_int);
    EXPECT_TRUE(!o18r);

    constexpr m5::stl::optional<int> o19 = m5::stl::nullopt;
    constexpr auto o19r = std::move(o19).and_then(get_opt_int);
    EXPECT_TRUE(!o19r);
#endif
  }
  {
    SCOPED_TRACE("or else");
    m5::stl::optional<int> o1 = 42;
    EXPECT_TRUE(*(o1.or_else([] { return m5::stl::make_optional(13); })) == 42);

    m5::stl::optional<int> o2;
    EXPECT_TRUE(*(o2.or_else([] { return m5::stl::make_optional(13); })) == 13);
  }
  {
    SCOPED_TRACE("disjunction");
    m5::stl::optional<int> o1 = 42;
    m5::stl::optional<int> o2 = 12;
    m5::stl::optional<int> o3;

    EXPECT_TRUE(*o1.disjunction(o2) == 42);
    EXPECT_TRUE(*o1.disjunction(o3) == 42);
    EXPECT_TRUE(*o2.disjunction(o1) == 12);
    EXPECT_TRUE(*o2.disjunction(o3) == 12);
    EXPECT_TRUE(*o3.disjunction(o1) == 42);
    EXPECT_TRUE(*o3.disjunction(o2) == 12);
  }
  {
    SCOPED_TRACE("conjunction");
    m5::stl::optional<int> o1 = 42;
    EXPECT_TRUE(*o1.conjunction(42.0) == 42.0);
    EXPECT_TRUE(*o1.conjunction(std::string{"hello"}) == std::string{"hello"});

    m5::stl::optional<int> o2;
    EXPECT_TRUE(!o2.conjunction(42.0));
    EXPECT_TRUE(!o2.conjunction(std::string{"hello"}));
  }
  {
    SCOPED_TRACE("map_or");
    m5::stl::optional<int> o1 = 21;
    EXPECT_TRUE((o1.map_or([](int x) { return x * 2; }, 13)) == 42);

    m5::stl::optional<int> o2;
    EXPECT_TRUE((o2.map_or([](int x) { return x * 2; }, 13)) == 13);
  }
  {
    SCOPED_TRACE("map_or_else");
    m5::stl::optional<int> o1 = 21;
    EXPECT_TRUE((o1.map_or_else([](int x) { return x * 2; }, [] { return 13; })) ==
            42);

    m5::stl::optional<int> o2;
    EXPECT_TRUE((o2.map_or_else([](int x) { return x * 2; }, [] { return 13; })) ==
            13);
  }
  {
    SCOPED_TRACE("take");
    m5::stl::optional<int> o1 = 42;
    EXPECT_TRUE(*o1.take() == 42);
    EXPECT_TRUE(!o1);

    m5::stl::optional<int> o2;
    EXPECT_TRUE(!o2.take());
    EXPECT_TRUE(!o2);
  }

  struct foo {
    void non_const() {}
  };

#if defined(TL_OPTIONAL_CXX14) && !defined(TL_OPTIONAL_GCC49) &&               \
    !defined(TL_OPTIONAL_GCC54) && !defined(TL_OPTIONAL_GCC55)
  {
    SCOPED_TRACE("Issue #1");
    m5::stl::optional<foo> f = foo{};
    auto l = [](auto &&x) { x.non_const(); };
    f.map(l);
  }
#endif

  struct overloaded {
    m5::stl::optional<int> operator()(foo &) { return 0; }
    m5::stl::optional<std::string> operator()(const foo &) { return ""; }
  };
  {
    SCOPED_TRACE("Issue #2");
    m5::stl::optional<foo> f = foo{};
    auto x = f.and_then(overloaded{});
  }
};
