//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria/test/test.hpp>

#include <fitoria/test/memfn.hpp>

#include <fitoria/core/optional.hpp>

#include <vector>

using namespace fitoria;
using namespace fitoria::test;

TEST_SUITE_BEGIN("[fitoria.core.optional]");

namespace {

template <typename T>
class test_vector : public std::vector<T> {
public:
  test_vector(std::initializer_list<T> list)
      : std::vector<T> { list }
  {
  }
};

}

TEST_CASE("trivial")
{
  {
    using type = optional<int>;
    static_assert(std::is_trivially_copy_constructible_v<type>);
    static_assert(std::is_trivially_copy_assignable_v<type>);
    static_assert(std::is_trivially_move_constructible_v<type>);
    static_assert(std::is_trivially_move_assignable_v<type>);
    static_assert(std::is_trivially_destructible_v<type>);
  }
  {
    using type = optional<std::vector<int>>;
    static_assert(!std::is_trivially_copy_constructible_v<type>);
    static_assert(!std::is_trivially_copy_assignable_v<type>);
    static_assert(!std::is_trivially_move_constructible_v<type>);
    static_assert(!std::is_trivially_move_assignable_v<type>);
    static_assert(!std::is_trivially_destructible_v<type>);
  }
}

TEST_CASE("has_value")
{
  CHECK(!optional<int>().has_value());
  CHECK(optional<int>(1).has_value());
}

TEST_CASE("nullopt")
{
  {
    optional<int> o(nullopt);
    CHECK(!o);
  }
  {
    auto o = optional<int>(1);
    CHECK(o);
    o = nullopt;
    CHECK(!o);
  }
}

TEST_CASE("in-place constructor")
{
  {
    auto o = optional<int>(std::in_place, 1);
    CHECK_EQ(o.value(), 1);
  }
  {
    auto o = optional<std::vector<int>>(std::in_place, { 0, 1, 2 });
    CHECK_EQ(o.value(), std::vector<int> { 0, 1, 2 });
  }
}

TEST_CASE("perfect forwarding constructor")
{
  {
    auto o = optional<std::vector<int>>(std::vector<int>(1));
    CHECK_EQ(o.value(), std::vector<int>(1));
  }
}

TEST_CASE("copy constructor")
{
  {
    auto s = optional<std::vector<int>>();
    auto d = optional<std::vector<int>>(s);
    CHECK(!s);
    CHECK(!d);
  }
  {
    auto s = optional<std::vector<int>>(1);
    auto d = optional<std::vector<int>>(s);
    CHECK_EQ(s, std::vector<int>(1));
    CHECK_EQ(d, std::vector<int>(1));
  }
}

TEST_CASE("move constructor")
{
  {
    auto s = optional<std::vector<int>>();
    auto d = optional<std::vector<int>>(std::move(s));
    CHECK(!s);
    CHECK(!d);
  }
  {
    auto s = optional<std::vector<int>>(1);
    auto d = optional<std::vector<int>>(std::move(s));
    CHECK_EQ(s, std::vector<int>());
    CHECK_EQ(d, std::vector<int>(1));
  }
}

TEST_CASE("converting constructor")
{
  {
    auto s = optional<test_vector<int>>();
    auto d = optional<std::vector<int>>(s);
    CHECK(!s);
    CHECK(!d);
  }
  {
    auto s = optional<test_vector<int>>();
    auto d = optional<std::vector<int>>(std::move(s));
    CHECK(!s);
    CHECK(!d);
  }
  {
    auto s = optional<test_vector<int>>(std::in_place, { 0, 1, 2 });
    auto d = optional<std::vector<int>>(s);
    CHECK(s);
    CHECK_EQ(d.value(), std::vector<int> { 0, 1, 2 });
  }
  {
    auto s = optional<test_vector<int>>(std::in_place, { 0, 1, 2 });
    auto d = optional<std::vector<int>>(std::move(s));
    CHECK(s);
    CHECK_EQ(d.value(), std::vector<int> { 0, 1, 2 });
  }
}

TEST_CASE("perfect forwarding assignment operator")
{
  {
    auto o = optional<std::vector<int>>();
    CHECK(!o);
    o = std::vector<int>(1);
    CHECK_EQ(o.value(), std::vector<int>(1));
  }
  {
    auto o = optional<std::vector<int>>(1);
    CHECK(o);
    o = std::vector<int>(1);
    CHECK_EQ(o.value(), std::vector<int>(1));
  }
}

TEST_CASE("copy assignment operator")
{
  {
    auto s = optional<std::vector<int>>();
    auto d = optional<std::vector<int>>();
    d = s;
    CHECK(!s);
    CHECK(!d);
  }
  {
    auto s = optional<std::vector<int>>(1);
    auto d = optional<std::vector<int>>();
    d = s;
    CHECK_EQ(s, std::vector<int>(1));
    CHECK_EQ(d, std::vector<int>(1));
  }
  {
    auto s = optional<std::vector<int>>();
    auto d = optional<std::vector<int>>(1);
    d = s;
    CHECK(!s);
    CHECK(!d);
  }
  {
    auto s = optional<std::vector<int>>(1);
    auto d = optional<std::vector<int>>(1);
    d = s;
    CHECK_EQ(s, std::vector<int>(1));
    CHECK_EQ(d, std::vector<int>(1));
  }
}

TEST_CASE("move assignment operator")
{
  {
    auto s = optional<std::vector<int>>();
    auto d = optional<std::vector<int>>();
    d = std::move(s);
    CHECK(!s);
    CHECK(!d);
  }
  {
    auto s = optional<std::vector<int>>(1);
    auto d = optional<std::vector<int>>();
    d = std::move(s);
    CHECK_EQ(s, std::vector<int>());
    CHECK_EQ(d, std::vector<int>(1));
  }
  {
    auto s = optional<std::vector<int>>();
    auto d = optional<std::vector<int>>(1);
    d = std::move(s);
    CHECK(!s);
    CHECK(!d);
  }
  {
    auto s = optional<std::vector<int>>(1);
    auto d = optional<std::vector<int>>(1);
    d = std::move(s);
    CHECK_EQ(s, std::vector<int>());
    CHECK_EQ(d, std::vector<int>(1));
  }
}

TEST_CASE("converting assignment operator")
{
  {
    auto s = optional<test_vector<int>>();
    auto d = optional<std::vector<int>>();
    d = s;
    CHECK(!s);
    CHECK(!d);
  }
  {
    auto s = optional<test_vector<int>>(std::in_place, { 0, 1, 2 });
    auto d = optional<std::vector<int>>();
    d = s;
    CHECK(s);
    CHECK_EQ(d.value(), std::vector<int> { 0, 1, 2 });
  }
  {
    auto s = optional<test_vector<int>>();
    auto d = optional<std::vector<int>>();
    d = std::move(s);
    CHECK(!s);
    CHECK(!d);
  }
  {
    auto s = optional<test_vector<int>>(std::in_place, { 0, 1, 2 });
    auto d = optional<std::vector<int>>();
    d = std::move(s);
    CHECK(s);
    CHECK_EQ(d.value(), std::vector<int> { 0, 1, 2 });
  }
}

TEST_CASE("operator->")
{
  static_assert(std::is_same_v<memop_mem_of<optional<int>&>, int*>);
  static_assert(std::is_same_v<memop_mem_of<const optional<int>&>, const int*>);
  static_assert(std::is_same_v<memop_mem_of<optional<int>>, int*>);
  static_assert(std::is_same_v<memop_mem_of<const optional<int>>, const int*>);
  {
    auto exp = optional<std::vector<int>>(std::in_place, { 0, 1, 2 });
    CHECK_EQ(exp->size(), 3);
  }
  {
    const auto exp = optional<std::vector<int>>(std::in_place, { 0, 1, 2 });
    CHECK_EQ(exp->size(), 3);
  }
}

TEST_CASE("operator*")
{
  static_assert(std::is_same_v<memop_deref<optional<int>&>, int&>);
  static_assert(std::is_same_v<memop_deref<const optional<int>&>, const int&>);
  static_assert(std::is_same_v<memop_deref<optional<int>>, int&&>);
  static_assert(std::is_same_v<memop_deref<const optional<int>>, const int&&>);
  {
    auto o = optional<int>(1);
    CHECK_EQ(*o, 1);
  }
  {
    const auto o = optional<int>(1);
    CHECK_EQ(*o, 1);
  }
  {
    auto o = optional<int>(1);
    CHECK_EQ(*std::move(o), 1);
  }
  {
    const auto o = optional<int>(1);
    CHECK_EQ(*std::move(o), 1);
  }
}

TEST_CASE("value")
{
  static_assert(std::is_same_v<memfn_value<optional<int>&>, int&>);
  static_assert(std::is_same_v<memfn_value<const optional<int>&>, const int&>);
  static_assert(std::is_same_v<memfn_value<optional<int>>, int&&>);
  static_assert(std::is_same_v<memfn_value<const optional<int>>, const int&&>);
  {
    auto o = optional<int>(1);
    CHECK_EQ(o.value(), 1);
  }
  {
    const auto o = optional<int>(1);
    CHECK_EQ(o.value(), 1);
  }
  {
    auto o = optional<int>(1);
    CHECK_EQ(std::move(o).value(), 1);
  }
  {
    const auto o = optional<int>(1);
    CHECK_EQ(std::move(o).value(), 1);
  }
#if !FITORIA_NO_EXCEPTIONS
  {
    auto o = optional<int>();
    CHECK_THROWS_AS(o.value(), bad_optional_access);
  }
  {
    const auto o = optional<int>();
    CHECK_THROWS_AS(o.value(), bad_optional_access);
  }
  {
    auto o = optional<int>();
    CHECK_THROWS_AS(std::move(o).value(), bad_optional_access);
  }
  {
    const auto o = optional<int>();
    CHECK_THROWS_AS(std::move(o).value(), bad_optional_access);
  }
#endif
}

TEST_CASE("value_or")
{
  {
    optional<int> o;
    CHECK_EQ(o.value_or(2LL), 2);
  }
  {
    optional<int> o(1);
    CHECK_EQ(o.value_or(2LL), 1);
  }
  {
    optional<int> o;
    CHECK_EQ(std::move(o).value_or(2LL), 2);
  }
  {
    optional<int> o(1);
    CHECK_EQ(std::move(o).value_or(2LL), 1);
  }
}

TEST_CASE("emplace")
{
  {
    optional<int> o;
    CHECK(!o);
    o.emplace(1);
    CHECK_EQ(o.value(), 1);
  }
  {
    optional<int> o(1);
    CHECK(o);
    o.emplace(1);
    CHECK_EQ(o.value(), 1);
  }
  {
    optional<std::vector<int>> o;
    CHECK(!o);
    o.emplace({ 3, 4, 5 });
    CHECK_EQ(o.value(), std::vector<int> { 3, 4, 5 });
  }
  {
    optional<std::vector<int>> o(std::in_place, { 0, 1, 2 });
    CHECK(o);
    o.emplace({ 3, 4, 5 });
    CHECK_EQ(o.value(), std::vector<int> { 3, 4, 5 });
  }
}

TEST_CASE("reset")
{
  {
    optional<int> o(1);
    CHECK(o);
    o.reset();
    CHECK(!o);
  }
}

TEST_CASE("and_then")
{
  {
    auto f = [](int i) -> optional<std::string> { return std::to_string(i); };
    {
      auto o = optional<int>();
      CHECK(!o.and_then(f));
    }
    {
      auto o = optional<int>(1);
      CHECK_EQ(o.and_then(f), "1");
    }
    {
      const auto o = optional<int>();
      CHECK(!o.and_then(f));
    }
    {
      const auto o = optional<int>(1);
      CHECK_EQ(o.and_then(f), "1");
    }
    {
      auto o = optional<int>();
      CHECK(!std::move(o).and_then(f));
    }
    {
      auto o = optional<int>(1);
      CHECK_EQ(std::move(o).and_then(f), "1");
    }
    {
      const auto o = optional<int>();
      CHECK(!std::move(o).and_then(f));
    }
    {
      const auto o = optional<int>(1);
      CHECK_EQ(std::move(o).and_then(f), "1");
    }
  }
}

TEST_CASE("transform")
{
  {
    auto f = [](int i) -> std::string { return std::to_string(i); };
    {
      auto o = optional<int>();
      CHECK(!o.transform(f));
    }
    {
      auto o = optional<int>(1);
      CHECK_EQ(o.transform(f), "1");
    }
    {
      const auto o = optional<int>();
      CHECK(!o.transform(f));
    }
    {
      const auto o = optional<int>(1);
      CHECK_EQ(o.transform(f), "1");
    }
    {
      auto o = optional<int>();
      CHECK(!std::move(o).transform(f));
    }
    {
      auto o = optional<int>(1);
      CHECK_EQ(std::move(o).transform(f), "1");
    }
    {
      const auto o = optional<int>();
      CHECK(!std::move(o).transform(f));
    }
    {
      const auto o = optional<int>(1);
      CHECK_EQ(std::move(o).transform(f), "1");
    }
  }
}

TEST_CASE("or_else")
{
  {
    auto f = []() -> optional<int> { return { 2 }; };
    {
      auto o = optional<int>();
      CHECK_EQ(o.or_else(f), 2);
    }
    {
      auto o = optional<int>(1);
      CHECK_EQ(o.or_else(f), 1);
    }
    {
      const auto o = optional<int>();
      CHECK_EQ(o.or_else(f), 2);
    }
    {
      const auto o = optional<int>(1);
      CHECK_EQ(o.or_else(f), 1);
    }
    {
      auto o = optional<int>();
      CHECK_EQ(std::move(o).or_else(f), 2);
    }
    {
      auto o = optional<int>(1);
      CHECK_EQ(std::move(o).or_else(f), 1);
    }
    {
      const auto o = optional<int>();
      CHECK_EQ(std::move(o).or_else(f), 2);
    }
    {
      const auto o = optional<int>(1);
      CHECK_EQ(std::move(o).or_else(f), 1);
    }
  }
}

TEST_CASE("swap")
{
  {
    auto a = optional<int>();
    auto b = optional<int>();
    swap(a, b);
    CHECK(!a);
    CHECK(!b);
  }
  {
    auto a = optional<int>();
    auto b = optional<int>(1);
    swap(a, b);
    CHECK_EQ(a.value(), 1);
    CHECK(!b);
  }
  {
    auto a = optional<int>(1);
    auto b = optional<int>(2);
    swap(a, b);
    CHECK_EQ(a.value(), 2);
    CHECK_EQ(b.value(), 1);
  }
}

TEST_CASE("compare")
{
  {
    const auto n = optional<int>();
    const auto v = optional<int64_t>(1);
    CHECK(n == n);
    CHECK_FALSE(n == v);
    CHECK_FALSE(v == n);
    CHECK(v == v);
    CHECK_FALSE(n != n);
    CHECK(n != v);
    CHECK(v != n);
    CHECK_FALSE(v != v);
    CHECK_FALSE(n < n);
    CHECK(n < v);
    CHECK_FALSE(v < n);
    CHECK_FALSE(v < v);
    CHECK(n <= n);
    CHECK(n <= v);
    CHECK_FALSE(v <= n);
    CHECK(v <= v);
    CHECK_FALSE(n > n);
    CHECK_FALSE(n > v);
    CHECK(v > n);
    CHECK_FALSE(v > v);
    CHECK(n >= n);
    CHECK_FALSE(n >= v);
    CHECK(v >= n);
    CHECK(v >= v);
  }
  {
    const auto n = optional<int>();
    const auto v = optional<int>(1);
    CHECK(n == nullopt);
    CHECK(nullopt == n);
    CHECK_FALSE(v == nullopt);
    CHECK_FALSE(nullopt == v);
    CHECK_FALSE(n != nullopt);
    CHECK_FALSE(nullopt != n);
    CHECK(v != nullopt);
    CHECK(nullopt != v);
    CHECK_FALSE(n < nullopt);
    CHECK_FALSE(nullopt < n);
    CHECK_FALSE(v < nullopt);
    CHECK(nullopt < v);
    CHECK(n <= nullopt);
    CHECK(nullopt <= n);
    CHECK_FALSE(v <= nullopt);
    CHECK(nullopt <= v);
    CHECK_FALSE(n > nullopt);
    CHECK_FALSE(nullopt > n);
    CHECK(v > nullopt);
    CHECK_FALSE(nullopt > v);
    CHECK(n >= nullopt);
    CHECK(nullopt >= n);
    CHECK(v >= nullopt);
    CHECK_FALSE(nullopt >= v);
  }
  {
    const auto n = optional<int>();
    const auto v = optional<int>(1);
    CHECK_FALSE(n == 1);
    CHECK_FALSE(1 == n);
    CHECK(v == 1);
    CHECK(1 == v);
    CHECK(n != 1);
    CHECK(1 != n);
    CHECK_FALSE(v != 1);
    CHECK_FALSE(1 != v);
    CHECK(n < 1);
    CHECK_FALSE(1 < n);
    CHECK_FALSE(v < 1);
    CHECK_FALSE(1 < v);
    CHECK(n <= 1);
    CHECK_FALSE(1 <= n);
    CHECK(v <= 1);
    CHECK(1 <= v);
    CHECK_FALSE(n > 1);
    CHECK(1 > n);
    CHECK_FALSE(v > 1);
    CHECK_FALSE(1 > v);
    CHECK_FALSE(n >= 1);
    CHECK(1 >= n);
    CHECK(v >= 1);
    CHECK(1 >= v);
  }
}

TEST_CASE("make_optional")
{
  auto o = make_optional(1);
  CHECK_EQ(o.value(), 1);

  auto i = make_optional<std::vector<int>>(1);
  CHECK_EQ(i.value(), std::vector<int>(1));

  auto vec = make_optional<std::vector<int>>({ 0, 1, 2 });
  CHECK_EQ(vec.value(), std::vector<int> { 0, 1, 2 });
}

TEST_CASE("format")
{
  {
    CHECK_EQ(fmt::format("{}", optional<int>(1)), "1");
    CHECK_EQ(fmt::format("{}", optional<int>()), "{nullopt}");
  }
}

TEST_SUITE_END();
