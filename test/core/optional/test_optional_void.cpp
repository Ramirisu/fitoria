//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria/test/test.hpp>

#include <fitoria/test/memfn.hpp>

#include <fitoria/core/optional.hpp>

using namespace fitoria;
using namespace fitoria::test;

TEST_SUITE_BEGIN("[fitoria.core.optional.void]");

TEST_CASE("trivial")
{
  using type = optional<void>;
  static_assert(std::is_trivially_copy_constructible_v<type>);
  static_assert(std::is_trivially_copy_assignable_v<type>);
  static_assert(std::is_trivially_move_constructible_v<type>);
  static_assert(std::is_trivially_move_assignable_v<type>);
  static_assert(std::is_trivially_destructible_v<type>);
}

TEST_CASE("has_value")
{
  CHECK(!optional<void>().has_value());
  CHECK(optional<void>(std::in_place).has_value());
}

TEST_CASE("nullopt")
{
  {
    optional<void> o(nullopt);
    CHECK(!o);
  }
  {
    auto o = optional<void>(std::in_place);
    CHECK(o);
    o = nullopt;
    CHECK(!o);
  }
}

TEST_CASE("in-place constructor")
{
  {
    auto o = optional<void>(std::in_place);
    CHECK(o);
  }
}

TEST_CASE("copy constructor")
{
  {
    auto s = optional<void>();
    auto d = optional<void>(s);
    CHECK(!s);
    CHECK(!d);
  }
  {
    auto s = optional<void>(std::in_place);
    auto d = optional<void>(s);
    CHECK(s);
    CHECK(d);
  }
}

TEST_CASE("move constructor")
{
  {
    auto s = optional<void>();
    auto d = optional<void>(std::move(s));
    CHECK(!s);
    CHECK(!d);
  }
  {
    auto s = optional<void>(std::in_place);
    auto d = optional<void>(std::move(s));
    CHECK(s);
    CHECK(d);
  }
}

TEST_CASE("copy assignment operator")
{
  {
    auto s = optional<void>();
    auto d = optional<void>();
    d = s;
    CHECK(!s);
    CHECK(!d);
  }
  {
    auto s = optional<void>(std::in_place);
    auto d = optional<void>();
    d = s;
    CHECK(s);
    CHECK(d);
  }
}

TEST_CASE("move assignment operator")
{
  {
    auto s = optional<void>();
    auto d = optional<void>();
    d = std::move(s);
    CHECK(!s);
    CHECK(!d);
  }
  {
    auto s = optional<void>(std::in_place);
    auto d = optional<void>();
    d = std::move(s);
    CHECK(s);
    CHECK(d);
  }
  {
    auto s = optional<void>();
    auto d = optional<void>();
    d = std::move(s);
    CHECK(!s);
    CHECK(!d);
  }
  {
    auto s = optional<void>(std::in_place);
    auto d = optional<void>();
    d = std::move(s);
    CHECK(s);
    CHECK(d);
  }
}

TEST_CASE("operator->")
{
  static_assert(std::is_same_v<memop_mem_of<optional<void>&>, void>);
  static_assert(std::is_same_v<memop_mem_of<const optional<void>&>, void>);
  static_assert(std::is_same_v<memop_mem_of<optional<void>>, void>);
  static_assert(std::is_same_v<memop_mem_of<const optional<void>>, void>);
}

TEST_CASE("operator*")
{
  static_assert(std::is_same_v<memop_deref<optional<void>&>, void>);
  static_assert(std::is_same_v<memop_deref<const optional<void>&>, void>);
  static_assert(std::is_same_v<memop_deref<optional<void>>, void>);
  static_assert(std::is_same_v<memop_deref<const optional<void>>, void>);
  {
    auto o = optional<void>(std::in_place);
    *o;
  }
  {
    const auto o = optional<void>(std::in_place);
    *o;
  }
  {
    auto o = optional<void>(std::in_place);
    *std::move(o);
  }
  {
    const auto o = optional<void>(std::in_place);
    *std::move(o);
  }
}

TEST_CASE("value")
{
  static_assert(std::is_same_v<memfn_value<optional<void>&>, void>);
  static_assert(std::is_same_v<memfn_value<const optional<void>&>, void>);
  static_assert(std::is_same_v<memfn_value<optional<void>>, void>);
  static_assert(std::is_same_v<memfn_value<const optional<void>>, void>);
  {
    auto o = optional<void>(std::in_place);
    o.value();
  }
  {
    const auto o = optional<void>(std::in_place);
    o.value();
  }
  {
    auto o = optional<void>(std::in_place);
    std::move(o).value();
  }
  {
    const auto o = optional<void>(std::in_place);
    std::move(o).value();
  }
#if !FITORIA_NO_EXCEPTIONS
  {
    auto o = optional<void>();
    CHECK_THROWS_AS(o.value(), bad_optional_access);
  }
  {
    const auto o = optional<void>();
    CHECK_THROWS_AS(o.value(), bad_optional_access);
  }
  {
    auto o = optional<void>();
    CHECK_THROWS_AS(std::move(o).value(), bad_optional_access);
  }
  {
    const auto o = optional<void>();
    CHECK_THROWS_AS(std::move(o).value(), bad_optional_access);
  }
#endif
}

TEST_CASE("emplace")
{
  {
    optional<void> o;
    CHECK(!o);
    o.emplace();
    CHECK(o);
  }
  {
    optional<void> o(std::in_place);
    CHECK(o);
    o.emplace();
    CHECK(o);
  }
}

TEST_CASE("reset")
{
  {
    optional<void> o(std::in_place);
    CHECK(o);
    o.reset();
    CHECK(!o);
  }
}

TEST_CASE("and_then")
{
  {
    int val = 0;
    auto f = [&]() -> optional<void> {
      val = 1;
      return optional<void>(std::in_place);
    };
    SUBCASE("")
    {
      auto o = optional<void>();
      CHECK(!o.and_then(f));
      CHECK_EQ(val, 0);
    }
    SUBCASE("")
    {
      auto o = optional<void>(std::in_place);
      CHECK(o.and_then(f));
      CHECK_EQ(val, 1);
    }
    SUBCASE("")
    {
      const auto o = optional<void>();
      CHECK(!o.and_then(f));
      CHECK_EQ(val, 0);
    }
    SUBCASE("")
    {
      const auto o = optional<void>(std::in_place);
      CHECK(o.and_then(f));
      CHECK_EQ(val, 1);
    }
    SUBCASE("")
    {
      auto o = optional<void>();
      CHECK(!std::move(o).and_then(f));
      CHECK_EQ(val, 0);
    }
    SUBCASE("")
    {
      auto o = optional<void>(std::in_place);
      CHECK(std::move(o).and_then(f));
      CHECK_EQ(val, 1);
    }
    SUBCASE("")
    {
      const auto o = optional<void>();
      CHECK(!std::move(o).and_then(f));
      CHECK_EQ(val, 0);
    }
    SUBCASE("")
    {
      const auto o = optional<void>(std::in_place);
      CHECK(std::move(o).and_then(f));
      CHECK_EQ(val, 1);
    }
  }
}

TEST_CASE("transform")
{
  {
    int val = 0;
    auto f = [&]() { val = 1; };
    SUBCASE("")
    {
      auto o = optional<void>();
      CHECK(!o.transform(f));
      CHECK_EQ(val, 0);
    }
    SUBCASE("")
    {
      auto o = optional<void>(std::in_place);
      CHECK(o.transform(f));
      CHECK_EQ(val, 1);
    }
    SUBCASE("")
    {
      const auto o = optional<void>();
      CHECK(!o.transform(f));
      CHECK_EQ(val, 0);
    }
    SUBCASE("")
    {
      const auto o = optional<void>(std::in_place);
      CHECK(o.transform(f));
      CHECK_EQ(val, 1);
    }
    SUBCASE("")
    {
      auto o = optional<void>();
      CHECK(!std::move(o).transform(f));
      CHECK_EQ(val, 0);
    }
    SUBCASE("")
    {
      auto o = optional<void>(std::in_place);
      CHECK(std::move(o).transform(f));
      CHECK_EQ(val, 1);
    }
    SUBCASE("")
    {
      const auto o = optional<void>();
      CHECK(!std::move(o).transform(f));
      CHECK_EQ(val, 0);
    }
    SUBCASE("")
    {
      const auto o = optional<void>(std::in_place);
      CHECK(std::move(o).transform(f));
      CHECK_EQ(val, 1);
    }
  }
}

TEST_CASE("or_else")
{
  {
    auto f = []() -> optional<void> { return optional<void>(std::in_place); };
    {
      auto o = optional<void>();
      CHECK(o.or_else(f));
    }
    {
      auto o = optional<void>(std::in_place);
      CHECK(o.or_else(f));
    }
    {
      const auto o = optional<void>();
      CHECK(o.or_else(f));
    }
    {
      const auto o = optional<void>(std::in_place);
      CHECK(o.or_else(f));
    }
    {
      auto o = optional<void>();
      CHECK(std::move(o).or_else(f));
    }
    {
      auto o = optional<void>(std::in_place);
      CHECK(std::move(o).or_else(f));
    }
    {
      const auto o = optional<void>();
      CHECK(std::move(o).or_else(f));
    }
    {
      const auto o = optional<void>(std::in_place);
      CHECK(std::move(o).or_else(f));
    }
  }
}

TEST_CASE("swap")
{
  {
    auto a = optional<void>();
    auto b = optional<void>();
    swap(a, b);
    CHECK(!a);
    CHECK(!b);
  }
  {
    auto a = optional<void>();
    auto b = optional<void>(std::in_place);
    swap(a, b);
    CHECK(a);
    CHECK(!b);
  }
  {
    auto a = optional<void>(std::in_place);
    auto b = optional<void>();
    swap(a, b);
    CHECK(!a);
    CHECK(b);
  }
  {
    auto a = optional<void>(std::in_place);
    auto b = optional<void>(std::in_place);
    swap(a, b);
    CHECK(a);
    CHECK(b);
  }
}

TEST_CASE("compare")
{
  {
    const auto n = optional<void>();
    const auto v = optional<void>(std::in_place);
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
    const auto n = optional<void>();
    const auto v = optional<void>(std::in_place);
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
}

TEST_SUITE_END();
