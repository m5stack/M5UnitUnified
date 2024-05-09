#include <gtest/gtest.h>
#include <m5_utility/stl/expected.hpp>

#include <type_traits>
#include <vector>
#include <string>

struct takes_init_and_variadic {
    std::vector<int> v;
    std::tuple<int, int> t;
    template <class... Args>
    takes_init_and_variadic(std::initializer_list<int> l, Args &&... args)
        : v(l), t(std::forward<Args>(args)...) {}
};

TEST(Expected, Constructors) {
    {
        m5::stl::expected<int,int> e;
        EXPECT_TRUE(e);
        EXPECT_TRUE(e == 0);
    }

    {
        m5::stl::expected<int,int> e = m5::stl::make_unexpected(0);
        EXPECT_TRUE(!e);
        EXPECT_TRUE(e.error() == 0);
    }

    {
        m5::stl::expected<int,int> e (m5::stl::unexpect, 0);
        EXPECT_TRUE(!e);
        EXPECT_TRUE(e.error() == 0);
    }

    {
        m5::stl::expected<int,int> e (m5::stl::in_place, 42);
        EXPECT_TRUE(e);
        EXPECT_TRUE(e == 42);
    }

    {
        m5::stl::expected<std::vector<int>,int> e (m5::stl::in_place, {0,1});
        EXPECT_TRUE(e);
        EXPECT_TRUE((*e)[0] == 0);
        EXPECT_TRUE((*e)[1] == 1);
    }

    {
        m5::stl::expected<std::tuple<int,int>,int> e (m5::stl::in_place, 0, 1);
        EXPECT_TRUE(e);
        EXPECT_TRUE(std::get<0>(*e) == 0);
        EXPECT_TRUE(std::get<1>(*e) == 1);
    }

    {
        m5::stl::expected<takes_init_and_variadic,int> e (m5::stl::in_place, {0,1}, 2, 3);
        EXPECT_TRUE(e);
        EXPECT_TRUE(e->v[0] == 0);
        EXPECT_TRUE(e->v[1] == 1);
        EXPECT_TRUE(std::get<0>(e->t) == 2);
        EXPECT_TRUE(std::get<1>(e->t) == 3);
    }

	{
		m5::stl::expected<int, int> e;
		EXPECT_TRUE(std::is_default_constructible<decltype(e)>::value);
		EXPECT_TRUE(std::is_copy_constructible<decltype(e)>::value);
		EXPECT_TRUE(std::is_move_constructible<decltype(e)>::value);
		EXPECT_TRUE(std::is_copy_assignable<decltype(e)>::value);
		EXPECT_TRUE(std::is_move_assignable<decltype(e)>::value);
		EXPECT_TRUE(TL_EXPECTED_IS_TRIVIALLY_COPY_CONSTRUCTIBLE(decltype(e))::value);
		EXPECT_TRUE(TL_EXPECTED_IS_TRIVIALLY_COPY_ASSIGNABLE(decltype(e))::value);
#	if !defined(TL_EXPECTED_GCC49)
		EXPECT_TRUE(std::is_trivially_move_constructible<decltype(e)>::value);
		EXPECT_TRUE(std::is_trivially_move_assignable<decltype(e)>::value);
#	endif
	}

	{
		m5::stl::expected<int, std::string> e;
		EXPECT_TRUE(std::is_default_constructible<decltype(e)>::value);
		EXPECT_TRUE(std::is_copy_constructible<decltype(e)>::value);
		EXPECT_TRUE(std::is_move_constructible<decltype(e)>::value);
		EXPECT_TRUE(std::is_copy_assignable<decltype(e)>::value);
		EXPECT_TRUE(std::is_move_assignable<decltype(e)>::value);
		EXPECT_TRUE(!TL_EXPECTED_IS_TRIVIALLY_COPY_CONSTRUCTIBLE(decltype(e))::value);
		EXPECT_TRUE(!TL_EXPECTED_IS_TRIVIALLY_COPY_ASSIGNABLE(decltype(e))::value);
#	if !defined(TL_EXPECTED_GCC49)
		EXPECT_TRUE(!std::is_trivially_move_constructible<decltype(e)>::value);
		EXPECT_TRUE(!std::is_trivially_move_assignable<decltype(e)>::value);
#	endif
	}

	{
		m5::stl::expected<std::string, int> e;
		EXPECT_TRUE(std::is_default_constructible<decltype(e)>::value);
		EXPECT_TRUE(std::is_copy_constructible<decltype(e)>::value);
		EXPECT_TRUE(std::is_move_constructible<decltype(e)>::value);
		EXPECT_TRUE(std::is_copy_assignable<decltype(e)>::value);
		EXPECT_TRUE(std::is_move_assignable<decltype(e)>::value);
		EXPECT_TRUE(!TL_EXPECTED_IS_TRIVIALLY_COPY_CONSTRUCTIBLE(decltype(e))::value);
		EXPECT_TRUE(!TL_EXPECTED_IS_TRIVIALLY_COPY_ASSIGNABLE(decltype(e))::value);
#	if !defined(TL_EXPECTED_GCC49)
		EXPECT_TRUE(!std::is_trivially_move_constructible<decltype(e)>::value);
		EXPECT_TRUE(!std::is_trivially_move_assignable<decltype(e)>::value);
#	endif
	}

	{
		m5::stl::expected<std::string, std::string> e;
		EXPECT_TRUE(std::is_default_constructible<decltype(e)>::value);
		EXPECT_TRUE(std::is_copy_constructible<decltype(e)>::value);
		EXPECT_TRUE(std::is_move_constructible<decltype(e)>::value);
		EXPECT_TRUE(std::is_copy_assignable<decltype(e)>::value);
		EXPECT_TRUE(std::is_move_assignable<decltype(e)>::value);
		EXPECT_TRUE(!TL_EXPECTED_IS_TRIVIALLY_COPY_CONSTRUCTIBLE(decltype(e))::value);
		EXPECT_TRUE(!TL_EXPECTED_IS_TRIVIALLY_COPY_ASSIGNABLE(decltype(e))::value);
#	if !defined(TL_EXPECTED_GCC49)
		EXPECT_TRUE(!std::is_trivially_move_constructible<decltype(e)>::value);
		EXPECT_TRUE(!std::is_trivially_move_assignable<decltype(e)>::value);
#	endif
	}

    {
        m5::stl::expected<void,int> e;
        EXPECT_TRUE(e);
    }

    {
        m5::stl::expected<void,int> e (m5::stl::unexpect, 42);
        EXPECT_TRUE(!e);
        EXPECT_TRUE(e.error() == 42);
    }
}
