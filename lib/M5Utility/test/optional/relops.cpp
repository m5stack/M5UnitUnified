#include <gtest/gtest.h>
#include <m5_utility/stl/optional.hpp>

TEST(Optional, RelationalOps) {
  m5::stl::optional<int> o1{4};
  m5::stl::optional<int> o2{42};
  m5::stl::optional<int> o3{};
  {
    SCOPED_TRACE("self simple");
    EXPECT_TRUE(!(o1 == o2));
    EXPECT_TRUE(o1 == o1);
    EXPECT_TRUE(o1 != o2);
    EXPECT_TRUE(!(o1 != o1));
    EXPECT_TRUE(o1 < o2);
    EXPECT_TRUE(!(o1 < o1));
    EXPECT_TRUE(!(o1 > o2));
    EXPECT_TRUE(!(o1 > o1));
    EXPECT_TRUE(o1 <= o2);
    EXPECT_TRUE(o1 <= o1);
    EXPECT_TRUE(!(o1 >= o2));
    EXPECT_TRUE(o1 >= o1);
  }
  {
    SCOPED_TRACE("nullopt simple");
    EXPECT_TRUE(!(o1 == m5::stl::nullopt));
    EXPECT_TRUE(!(m5::stl::nullopt == o1));
    EXPECT_TRUE(o1 != m5::stl::nullopt);
    EXPECT_TRUE(m5::stl::nullopt != o1);
    EXPECT_TRUE(!(o1 < m5::stl::nullopt));
    EXPECT_TRUE(m5::stl::nullopt < o1);
    EXPECT_TRUE(o1 > m5::stl::nullopt);
    EXPECT_TRUE(!(m5::stl::nullopt > o1));
    EXPECT_TRUE(!(o1 <= m5::stl::nullopt));
    EXPECT_TRUE(m5::stl::nullopt <= o1);
    EXPECT_TRUE(o1 >= m5::stl::nullopt);
    EXPECT_TRUE(!(m5::stl::nullopt >= o1));

    EXPECT_TRUE(o3 == m5::stl::nullopt);
    EXPECT_TRUE(m5::stl::nullopt == o3);
    EXPECT_TRUE(!(o3 != m5::stl::nullopt));
    EXPECT_TRUE(!(m5::stl::nullopt != o3));
    EXPECT_TRUE(!(o3 < m5::stl::nullopt));
    EXPECT_TRUE(!(m5::stl::nullopt < o3));
    EXPECT_TRUE(!(o3 > m5::stl::nullopt));
    EXPECT_TRUE(!(m5::stl::nullopt > o3));
    EXPECT_TRUE(o3 <= m5::stl::nullopt);
    EXPECT_TRUE(m5::stl::nullopt <= o3);
    EXPECT_TRUE(o3 >= m5::stl::nullopt);
    EXPECT_TRUE(m5::stl::nullopt >= o3);
  }
  {
    SCOPED_TRACE("with T simple");
    EXPECT_TRUE(!(o1 == 1));
    EXPECT_TRUE(!(1 == o1));
    EXPECT_TRUE(o1 != 1);
    EXPECT_TRUE(1 != o1);
    EXPECT_TRUE(!(o1 < 1));
    EXPECT_TRUE(1 < o1);
    EXPECT_TRUE(o1 > 1);
    EXPECT_TRUE(!(1 > o1));
    EXPECT_TRUE(!(o1 <= 1));
    EXPECT_TRUE(1 <= o1);
    EXPECT_TRUE(o1 >= 1);
    EXPECT_TRUE(!(1 >= o1));

    EXPECT_TRUE(o1 == 4);
    EXPECT_TRUE(4 == o1);
    EXPECT_TRUE(!(o1 != 4));
    EXPECT_TRUE(!(4 != o1));
    EXPECT_TRUE(!(o1 < 4));
    EXPECT_TRUE(!(4 < o1));
    EXPECT_TRUE(!(o1 > 4));
    EXPECT_TRUE(!(4 > o1));
    EXPECT_TRUE(o1 <= 4);
    EXPECT_TRUE(4 <= o1);
    EXPECT_TRUE(o1 >= 4);
    EXPECT_TRUE(4 >= o1);
  }

  m5::stl::optional<std::string> o4{"hello"};
  m5::stl::optional<std::string> o5{"xyz"};
  {
    SCOPED_TRACE("self complex");
    EXPECT_TRUE(!(o4 == o5));
    EXPECT_TRUE(o4 == o4);
    EXPECT_TRUE(o4 != o5);
    EXPECT_TRUE(!(o4 != o4));
    EXPECT_TRUE(o4 < o5);
    EXPECT_TRUE(!(o4 < o4));
    EXPECT_TRUE(!(o4 > o5));
    EXPECT_TRUE(!(o4 > o4));
    EXPECT_TRUE(o4 <= o5);
    EXPECT_TRUE(o4 <= o4);
    EXPECT_TRUE(!(o4 >= o5));
    EXPECT_TRUE(o4 >= o4);
  }
  {
    SCOPED_TRACE("nullopt complex");
    EXPECT_TRUE(!(o4 == m5::stl::nullopt));
    EXPECT_TRUE(!(m5::stl::nullopt == o4));
    EXPECT_TRUE(o4 != m5::stl::nullopt);
    EXPECT_TRUE(m5::stl::nullopt != o4);
    EXPECT_TRUE(!(o4 < m5::stl::nullopt));
    EXPECT_TRUE(m5::stl::nullopt < o4);
    EXPECT_TRUE(o4 > m5::stl::nullopt);
    EXPECT_TRUE(!(m5::stl::nullopt > o4));
    EXPECT_TRUE(!(o4 <= m5::stl::nullopt));
    EXPECT_TRUE(m5::stl::nullopt <= o4);
    EXPECT_TRUE(o4 >= m5::stl::nullopt);
    EXPECT_TRUE(!(m5::stl::nullopt >= o4));

    EXPECT_TRUE(o3 == m5::stl::nullopt);
    EXPECT_TRUE(m5::stl::nullopt == o3);
    EXPECT_TRUE(!(o3 != m5::stl::nullopt));
    EXPECT_TRUE(!(m5::stl::nullopt != o3));
    EXPECT_TRUE(!(o3 < m5::stl::nullopt));
    EXPECT_TRUE(!(m5::stl::nullopt < o3));
    EXPECT_TRUE(!(o3 > m5::stl::nullopt));
    EXPECT_TRUE(!(m5::stl::nullopt > o3));
    EXPECT_TRUE(o3 <= m5::stl::nullopt);
    EXPECT_TRUE(m5::stl::nullopt <= o3);
    EXPECT_TRUE(o3 >= m5::stl::nullopt);
    EXPECT_TRUE(m5::stl::nullopt >= o3);
  }
  {
    SCOPED_TRACE("with T complex");
    EXPECT_TRUE(!(o4 == "a"));
    EXPECT_TRUE(!("a" == o4));
    EXPECT_TRUE(o4 != "a");
    EXPECT_TRUE("a" != o4);
    EXPECT_TRUE(!(o4 < "a"));
    EXPECT_TRUE("a" < o4);
    EXPECT_TRUE(o4 > "a");
    EXPECT_TRUE(!("a" > o4));
    EXPECT_TRUE(!(o4 <= "a"));
    EXPECT_TRUE("a" <= o4);
    EXPECT_TRUE(o4 >= "a");
    EXPECT_TRUE(!("a" >= o4));

    EXPECT_TRUE(o4 == "hello");
    EXPECT_TRUE("hello" == o4);
    EXPECT_TRUE(!(o4 != "hello"));
    EXPECT_TRUE(!("hello" != o4));
    EXPECT_TRUE(!(o4 < "hello"));
    EXPECT_TRUE(!("hello" < o4));
    EXPECT_TRUE(!(o4 > "hello"));
    EXPECT_TRUE(!("hello" > o4));
    EXPECT_TRUE(o4 <= "hello");
    EXPECT_TRUE("hello" <= o4);
    EXPECT_TRUE(o4 >= "hello");
    EXPECT_TRUE("hello" >= o4);
  }
}
