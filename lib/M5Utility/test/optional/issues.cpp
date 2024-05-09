#include <gtest/gtest.h>
#include <m5_utility/stl/optional.hpp>
#include <type_traits>

namespace issues {
struct foo{
    int& v() { return i; }
    int i = 0;
};
}
int& x(int& i) { i = 42; return i;}

TEST(Optional, issue14) {
    m5::stl::optional<issues::foo> f = issues::foo{};
    auto v = f.map(&issues::foo::v).map(x);
    static_assert(std::is_same<decltype(v), m5::stl::optional<int&>>::value, "Must return a reference");
    EXPECT_TRUE(f->i == 42);
    EXPECT_TRUE(*v == 42);
    EXPECT_TRUE((&f->i) == (&*v));
}

struct fail_on_copy_self {
    int value;
    fail_on_copy_self(int v) : value(v) {}
    fail_on_copy_self(const fail_on_copy_self& other) : value(other.value) {
        EXPECT_TRUE(&other != this);
    }
};

TEST(Optional,issue15) {
    m5::stl::optional<fail_on_copy_self> o = fail_on_copy_self(42);

    o = o;
    EXPECT_TRUE(o->value == 42);
}

TEST(Optional, issue33) {
    int i = 0;
    int j = 0;
    m5::stl::optional<int&> a = i;
    a.emplace(j);
    *a = 42;
    EXPECT_TRUE(j == 42);
    EXPECT_TRUE(*a == 42);
    EXPECT_TRUE(a.has_value());
}
