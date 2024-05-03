//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria/test/test.hpp>

#include <fitoria/test/memfn.hpp>

#include <fitoria/core/expected.hpp>

#include <vector>

using namespace fitoria;
using namespace fitoria::test;

TEST_SUITE_BEGIN("[fitoria.core.expected.void]");

TEST_CASE("trivially")
{
  {
    using type = expected<void, int>;
    static_assert(std::is_trivially_copy_constructible_v<type>);
    static_assert(std::is_trivially_copy_assignable_v<type>);
    static_assert(std::is_trivially_move_constructible_v<type>);
    static_assert(std::is_trivially_move_assignable_v<type>);
    static_assert(std::is_trivially_destructible_v<type>);
  }
}

TEST_CASE("default constructor")
{
  static_assert(std::is_default_constructible_v<expected<void, int>>);
  {
    auto exp = expected<void, int>();
    CHECK(exp);
  }
}

TEST_CASE("copy constructor")
{
  {
    auto s = expected<void, int>(std::in_place);
    expected<void, int> d(s);
    CHECK(s);
    CHECK(d);
  }
  {
    auto s = expected<void, int>(unexpect, 1);
    expected<void, int> d(s);
    CHECK_EQ(s.error(), 1);
    CHECK_EQ(d.error(), 1);
  }
  {
    auto s = expected<void, std::vector<int>>(std::in_place);
    expected<void, std::vector<int>> d(s);
    CHECK(s);
    CHECK(d);
  }
  {
    auto s = expected<void, std::vector<int>>(unexpect, { 0, 1, 2 });
    expected<void, std::vector<int>> d(s);
    CHECK_EQ(s.error(), std::vector<int>({ 0, 1, 2 }));
    CHECK_EQ(d.error(), std::vector<int>({ 0, 1, 2 }));
  }
}

TEST_CASE("move constructor")
{
  {
    auto s = expected<void, int>(std::in_place);
    expected<void, int> d(std::move(s));
    CHECK(s);
    CHECK(d);
  }
  {
    auto s = expected<void, int>(unexpect, 1);
    expected<void, int> d(std::move(s));
    CHECK_EQ(s.error(), 1);
    CHECK_EQ(d.error(), 1);
  }
  {
    auto s = expected<void, std::vector<int>>(std::in_place);
    expected<void, std::vector<int>> d(std::move(s));
    CHECK(s);
    CHECK(d);
  }
  {
    auto s = expected<void, std::vector<int>>(unexpect, { 0, 1, 2 });
    expected<void, std::vector<int>> d(std::move(s));
    CHECK_EQ(s.error(), std::vector<int>());
    CHECK_EQ(d.error(), std::vector<int>({ 0, 1, 2 }));
  }
}

TEST_CASE("in-place constructor")
{
  {
    auto exp = expected<void, int>(std::in_place);
    CHECK(exp);
  }
  {
    auto exp = expected<void, int>(unexpect, 1);
    CHECK_EQ(exp.error(), 1);
  }
  {
    auto exp = expected<void, std::vector<int>>(unexpect, { 1, 2, 3 });
    CHECK_EQ(exp.error(), std::vector<int> { 1, 2, 3 });
  }
}

TEST_CASE("copy assignment operator")
{
  {
    auto s = expected<void, int>(std::in_place);
    auto d = expected<void, int>(std::in_place);
    d = s;
    CHECK(s);
    CHECK(d);
  }
  {
    auto s = expected<void, int>(unexpect, 1);
    auto d = expected<void, int>(std::in_place);
    d = s;
    CHECK_EQ(s.error(), 1);
    CHECK_EQ(d.error(), 1);
  }
  {
    auto s = expected<void, int>(std::in_place);
    auto d = expected<void, int>(unexpect, 1);
    d = s;
    CHECK(s);
    CHECK(d);
  }
  {
    auto s = expected<void, int>(unexpect, 1);
    auto d = expected<void, int>(unexpect, 1);
    d = s;
    CHECK_EQ(s.error(), 1);
    CHECK_EQ(d.error(), 1);
  }
  {
    auto s = expected<void, std::vector<int>>(std::in_place);
    auto d = expected<void, std::vector<int>>(std::in_place);
    d = s;
    CHECK(s);
    CHECK(d);
  }
  {
    auto s = expected<void, std::vector<int>>(unexpect, { 0, 1, 2 });
    auto d = expected<void, std::vector<int>>(std::in_place);
    d = s;
    CHECK_EQ(s.error(), std::vector<int>({ 0, 1, 2 }));
    CHECK_EQ(d.error(), std::vector<int>({ 0, 1, 2 }));
  }
  {
    auto s = expected<void, std::vector<int>>(std::in_place);
    auto d = expected<void, std::vector<int>>(unexpect, { 3, 4, 5 });
    d = s;
    CHECK(s);
    CHECK(d);
  }
  {
    auto s = expected<void, std::vector<int>>(unexpect, { 0, 1, 2 });
    auto d = expected<void, std::vector<int>>(unexpect, { 3, 4, 5 });
    d = s;
    CHECK_EQ(s.error(), std::vector<int>({ 0, 1, 2 }));
    CHECK_EQ(d.error(), std::vector<int>({ 0, 1, 2 }));
  }
}

TEST_CASE("move assignment operator")
{
  {
    auto s = expected<void, int>(std::in_place);
    auto d = expected<void, int>(std::in_place);
    d = std::move(s);
    CHECK(s);
    CHECK(d);
  }
  {
    auto s = expected<void, int>(unexpect, 1);
    auto d = expected<void, int>(std::in_place);
    d = std::move(s);
    CHECK_EQ(s.error(), 1);
    CHECK_EQ(d.error(), 1);
  }
  {
    auto s = expected<void, int>(std::in_place);
    auto d = expected<void, int>(unexpect, 1);
    d = std::move(s);
    CHECK(s);
    CHECK(d);
  }
  {
    auto s = expected<void, int>(unexpect, 1);
    auto d = expected<void, int>(unexpect, 1);
    d = std::move(s);
    CHECK_EQ(s.error(), 1);
    CHECK_EQ(d.error(), 1);
  }
  {
    auto s = expected<void, std::vector<int>>(std::in_place);
    auto d = expected<void, std::vector<int>>(std::in_place);
    d = std::move(s);
    CHECK(s);
    CHECK(d);
  }
  {
    auto s = expected<void, std::vector<int>>(unexpect, { 0, 1, 2 });
    auto d = expected<void, std::vector<int>>(std::in_place);
    d = std::move(s);
    CHECK_EQ(s.error(), std::vector<int>());
    CHECK_EQ(d.error(), std::vector<int>({ 0, 1, 2 }));
  }
  {
    auto s = expected<void, std::vector<int>>(std::in_place);
    auto d = expected<void, std::vector<int>>(unexpect, { 3, 4, 5 });
    d = std::move(s);
    CHECK(s);
    CHECK(d);
  }
  {
    auto s = expected<void, std::vector<int>>(unexpect, { 0, 1, 2 });
    auto d = expected<void, std::vector<int>>(unexpect, { 3, 4, 5 });
    d = std::move(s);
    CHECK_EQ(s.error(), std::vector<int>());
    CHECK_EQ(d.error(), std::vector<int>({ 0, 1, 2 }));
  }
}

TEST_CASE("converting assignment operator")
{
  {
    auto exp = expected<void, int>();
    auto une = fitoria::unexpected<int>(2);
    exp = une;
    CHECK_EQ(exp.error(), 2);
  }
  {
    auto exp = expected<void, int>(unexpect);
    auto une = fitoria::unexpected<int>(2);
    exp = une;
    CHECK_EQ(exp.error(), 2);
  }
  {
    auto exp = expected<void, int>();
    auto une = fitoria::unexpected<int>(2);
    exp = std::move(une);
    CHECK_EQ(exp.error(), 2);
  }
  {
    auto exp = expected<void, int>(unexpect);
    auto une = fitoria::unexpected<int>(2);
    exp = std::move(une);
    CHECK_EQ(exp.error(), 2);
  }
  {
    auto exp = expected<int, int>();
    auto une = fitoria::unexpected<int>(2);
    exp = une;
    CHECK_EQ(exp.error(), 2);
  }
}

TEST_CASE("operator->")
{
  static_assert(std::is_same_v<memop_mem_of<expected<void, int>&>, void>);
  static_assert(std::is_same_v<memop_mem_of<const expected<void, int>&>, void>);
  static_assert(std::is_same_v<memop_mem_of<expected<void, int>>, void>);
  static_assert(std::is_same_v<memop_mem_of<const expected<void, int>>, void>);
}

TEST_CASE("operator*")
{
  static_assert(std::is_same_v<memop_deref<expected<void, int>&>, void>);
  static_assert(std::is_same_v<memop_deref<const expected<void, int>&>, void>);
  static_assert(std::is_same_v<memop_deref<expected<void, int>>, void>);
  static_assert(std::is_same_v<memop_deref<const expected<void, int>>, void>);
  {
    auto exp = expected<void, int>(std::in_place);
    *exp;
  }
  {
    const auto exp = expected<void, int>(std::in_place);
    *exp;
  }
  {
    auto exp = expected<void, int>(std::in_place);
    *std::move(exp);
  }
  {
    const auto exp = expected<void, int>(std::in_place);
    *std::move(exp);
  }
}

TEST_CASE("has_value")
{
  CHECK(expected<void, int>().has_value());
  CHECK(!expected<void, int>(unexpect, 1).has_value());
}

TEST_CASE("value")
{
  static_assert(std::is_same_v<memfn_value<expected<void, int>&>, void>);
  static_assert(std::is_same_v<memfn_value<const expected<void, int>&>, void>);
  static_assert(std::is_same_v<memfn_value<expected<void, int>>, void>);
  static_assert(std::is_same_v<memfn_value<const expected<void, int>>, void>);
  {
    auto exp = expected<void, int>(std::in_place);
    exp.value();
  }
  {
    const auto exp = expected<void, int>(std::in_place);
    exp.value();
  }
  {
    auto exp = expected<void, int>(std::in_place);
    std::move(exp).value();
  }
  {
    const auto exp = expected<void, int>(std::in_place);
    std::move(exp).value();
  }
#if !FITORIA_NO_EXCEPTIONS
  {
    auto exp = expected<void, int>(unexpect);
    CHECK_THROWS_AS(exp.value(), bad_expected_access<int>);
  }
  {
    const auto exp = expected<void, int>(unexpect);
    CHECK_THROWS_AS(exp.value(), bad_expected_access<int>);
  }
  {
    auto exp = expected<void, int>(unexpect);
    CHECK_THROWS_AS(std::move(exp).value(), bad_expected_access<int>);
  }
  {
    const auto exp = expected<void, int>(unexpect);
    CHECK_THROWS_AS(std::move(exp).value(), bad_expected_access<int>);
  }
#endif
}

TEST_CASE("error")
{
  static_assert(std::is_same_v<memfn_error<expected<void, int>&>, int&>);
  static_assert(
      std::is_same_v<memfn_error<const expected<void, int>&>, const int&>);
  static_assert(std::is_same_v<memfn_error<expected<void, int>>, int&&>);
  static_assert(
      std::is_same_v<memfn_error<const expected<void, int>>, const int&&>);
  {
    auto exp = expected<void, int>(unexpect, 1);
    CHECK_EQ(exp.error(), 1);
  }
  {
    const auto exp = expected<void, int>(unexpect, 1);
    CHECK_EQ(exp.error(), 1);
  }
  {
    auto exp = expected<void, int>(unexpect, 1);
    CHECK_EQ(std::move(exp).error(), 1);
  }
  {
    const auto exp = expected<void, int>(unexpect, 1);
    CHECK_EQ(std::move(exp).error(), 1);
  }
}

TEST_CASE("emplace")
{
  {
    auto e = expected<void, int>();
    e.emplace();
    CHECK(e);
  }
  {
    auto e = expected<void, int>(unexpect, 0);
    e.emplace();
    CHECK(e);
  }
}

TEST_CASE("and_then")
{
  {
    auto e = expected<void, int>();
    CHECK_EQ(e.and_then([]() -> expected<int, int> { return 1; }), 1);
  }
  {
    const auto e = expected<void, int>();
    CHECK_EQ(e.and_then([]() -> expected<int, int> { return 1; }), 1);
  }
  {
    auto e = expected<void, int>();
    CHECK_EQ(std::move(e).and_then([]() -> expected<int, int> { return 1; }),
             1);
  }
  {
    const auto e = expected<void, int>();
    CHECK_EQ(std::move(e).and_then([]() -> expected<int, int> { return 1; }),
             1);
  }
  {
    auto e = expected<void, int>(unexpect, -1);
    CHECK_EQ(e.and_then([]() -> expected<int, int> { return 1; }),
             fitoria::unexpected<int>(-1));
  }
  {
    const auto e = expected<void, int>(unexpect, -1);
    CHECK_EQ(e.and_then([]() -> expected<int, int> { return 1; }),
             fitoria::unexpected<int>(-1));
  }
  {
    auto e = expected<void, int>(unexpect, -1);
    CHECK_EQ(std::move(e).and_then([]() -> expected<int, int> { return 1; }),
             fitoria::unexpected<int>(-1));
  }
  {
    const auto e = expected<void, int>(unexpect, -1);
    CHECK_EQ(std::move(e).and_then([]() -> expected<int, int> { return 1; }),
             fitoria::unexpected<int>(-1));
  }
}

TEST_CASE("or_else")
{
  {
    auto e = expected<void, int>();
    CHECK_EQ(e.or_else([](int err) -> expected<void, int> {
      return fitoria::unexpected<int>(err - 1);
    }),
             expected<void, int>());
  }
  {
    const auto e = expected<void, int>();
    CHECK_EQ(e.or_else([](int err) -> expected<void, int> {
      return fitoria::unexpected<int>(err - 1);
    }),
             expected<void, int>());
  }
  {
    auto e = expected<void, int>();
    CHECK_EQ(std::move(e).or_else([](int err) -> expected<void, int> {
      return fitoria::unexpected<int>(err - 1);
    }),
             expected<void, int>());
  }
  {
    const auto e = expected<void, int>();
    CHECK_EQ(std::move(e).or_else([](int err) -> expected<void, int> {
      return fitoria::unexpected<int>(err - 1);
    }),
             expected<void, int>());
  }
  {
    auto e = expected<void, int>(unexpect, -1);
    CHECK_EQ(e.or_else([](int err) -> expected<void, int> {
      return fitoria::unexpected<int>(err - 1);
    }),
             fitoria::unexpected<int>(-2));
  }
  {
    const auto e = expected<void, int>(unexpect, -1);
    CHECK_EQ(e.or_else([](int err) -> expected<void, int> {
      return fitoria::unexpected<int>(err - 1);
    }),
             fitoria::unexpected<int>(-2));
  }
  {
    auto e = expected<void, int>(unexpect, -1);
    CHECK_EQ(std::move(e).or_else([](int err) -> expected<void, int> {
      return fitoria::unexpected<int>(err - 1);
    }),
             fitoria::unexpected<int>(-2));
  }
  {
    const auto e = expected<void, int>(unexpect, -1);
    CHECK_EQ(std::move(e).or_else([](int err) -> expected<void, int> {
      return fitoria::unexpected<int>(err - 1);
    }),
             fitoria::unexpected<int>(-2));
  }
}

TEST_CASE("transform")
{
  {
    auto e = expected<void, int>();
    CHECK_EQ(e.transform([]() -> expected<int, int> { return 1; }), 1);
  }
  {
    const auto e = expected<void, int>();
    CHECK_EQ(e.transform([]() -> expected<int, int> { return 1; }), 1);
  }
  {
    auto e = expected<void, int>();
    CHECK_EQ(std::move(e).transform([]() -> expected<int, int> { return 1; }),
             1);
  }
  {
    const auto e = expected<void, int>();
    CHECK_EQ(std::move(e).transform([]() -> expected<int, int> { return 1; }),
             1);
  }
  {
    auto e = expected<void, int>(unexpect, -1);
    CHECK_EQ(e.transform([]() -> expected<int, int> { return -1; }),
             fitoria::unexpected<int>(-1));
  }
  {
    const auto e = expected<void, int>(unexpect, -1);
    CHECK_EQ(e.transform([]() -> expected<int, int> { return -1; }),
             fitoria::unexpected<int>(-1));
  }
  {
    auto e = expected<void, int>(unexpect, -1);
    CHECK_EQ(std::move(e).transform([]() -> expected<int, int> { return -1; }),
             fitoria::unexpected<int>(-1));
  }
  {
    const auto e = expected<void, int>(unexpect, -1);
    CHECK_EQ(std::move(e).transform([]() -> expected<int, int> { return -1; }),
             fitoria::unexpected<int>(-1));
  }
}

TEST_CASE("transform_error")
{
  {
    auto e = expected<void, int>();
    CHECK_EQ(e.transform_error([](int err) -> int { return err - 1; }),
             expected<void, int>());
  }
  {
    const auto e = expected<void, int>();
    CHECK_EQ(e.transform_error([](int err) -> int { return err - 1; }),
             expected<void, int>());
  }
  {
    auto e = expected<void, int>();
    CHECK_EQ(
        std::move(e).transform_error([](int err) -> int { return err - 1; }),
        expected<void, int>());
  }
  {
    const auto e = expected<void, int>();
    CHECK_EQ(
        std::move(e).transform_error([](int err) -> int { return err - 1; }),
        expected<void, int>());
  }
  {
    auto e = expected<void, int>(unexpect, -1);
    CHECK_EQ(e.transform_error([](int err) -> int { return err - 1; }),
             fitoria::unexpected<int>(-2));
  }
  {
    const auto e = expected<void, int>(unexpect, -1);
    CHECK_EQ(e.transform_error([](int err) -> int { return err - 1; }),
             fitoria::unexpected<int>(-2));
  }
  {
    auto e = expected<void, int>(unexpect, -1);
    CHECK_EQ(
        std::move(e).transform_error([](int err) -> int { return err - 1; }),
        fitoria::unexpected<int>(-2));
  }
  {
    const auto e = expected<void, int>(unexpect, -1);
    CHECK_EQ(
        std::move(e).transform_error([](int err) -> int { return err - 1; }),
        fitoria::unexpected<int>(-2));
  }
}

TEST_CASE("swap")
{
  {
    auto a = expected<void, int>(unexpect);
    auto b = expected<void, int>(unexpect);
    swap(a, b);
    CHECK_EQ(a.error(), 0);
    CHECK_EQ(b.error(), 0);
  }
  {
    auto a = expected<void, int>(unexpect);
    auto b = expected<void, int>();
    swap(a, b);
    CHECK(a);
    CHECK_EQ(b.error(), 0);
  }
  {
    auto a = expected<void, int>();
    auto b = expected<void, int>(unexpect);
    swap(a, b);
    CHECK_EQ(a.error(), 0);
    CHECK(b);
  }
  {
    auto a = expected<void, int>();
    auto b = expected<void, int>();
    swap(a, b);
    CHECK(a);
    CHECK(b);
  }
}

TEST_CASE("compare")
{
  CHECK(expected<void, int>() == expected<void, int>());
  CHECK(!(expected<void, int>() != expected<void, int>()));
  CHECK(expected<void, int>(unexpect) == expected<void, int>(unexpect));
  CHECK(!(expected<void, int>(unexpect) != expected<void, int>(unexpect)));
  CHECK(!(expected<void, int>() == expected<void, int>(unexpect)));
  CHECK(expected<void, int>() != expected<void, int>(unexpect));
  CHECK(!(expected<void, int>(unexpect) == expected<void, int>()));
  CHECK(expected<void, int>(unexpect) != expected<void, int>());
}

TEST_SUITE_END();
