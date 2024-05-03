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

TEST_SUITE_BEGIN("[fitoria.core.expected]");

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

TEST_CASE("unexpected")
{
  {
    auto s = fitoria::unexpected<int>(1);
    CHECK_EQ(s.error(), 1);
  }
  {
    auto s = fitoria::unexpected<std::vector<int>>(std::in_place, 1);
    CHECK_EQ(s.error(), std::vector<int>(1));
  }
  {
    auto s = fitoria::unexpected<std::vector<int>>(std::in_place, { 0, 1, 2 });
    CHECK_EQ(s.error(), std::vector<int>({ 0, 1, 2 }));
  }
  {
    auto s = fitoria::unexpected<std::vector<int>>(1);
    fitoria::unexpected<std::vector<int>> d(s);
    CHECK_EQ(s.error(), std::vector<int>(1));
    CHECK_EQ(d.error(), std::vector<int>(1));
  }
  {
    const auto s = fitoria::unexpected<std::vector<int>>(1);
    fitoria::unexpected<std::vector<int>> d(s);
    CHECK_EQ(s.error(), std::vector<int>(1));
    CHECK_EQ(d.error(), std::vector<int>(1));
  }
  {
    auto s = fitoria::unexpected<std::vector<int>>(1);
    fitoria::unexpected<std::vector<int>> d(std::move(s));
    CHECK_EQ(s.error(), std::vector<int>());
    CHECK_EQ(d.error(), std::vector<int>(1));
  }
  {
    const auto s = fitoria::unexpected<std::vector<int>>(1);
    fitoria::unexpected<std::vector<int>> d(std::move(s));
    CHECK_EQ(s.error(), std::vector<int>(1));
    CHECK_EQ(d.error(), std::vector<int>(1));
  }
  {
    auto s = fitoria::unexpected<std::vector<int>>(1);
    auto d = s;
    CHECK_EQ(s.error(), std::vector<int>(1));
    CHECK_EQ(d.error(), std::vector<int>(1));
  }
  {
    const auto s = fitoria::unexpected<std::vector<int>>(1);
    auto d = s;
    CHECK_EQ(s.error(), std::vector<int>(1));
    CHECK_EQ(d.error(), std::vector<int>(1));
  }
  {
    auto s = fitoria::unexpected<std::vector<int>>(1);
    auto d = std::move(s);
    CHECK_EQ(s.error(), std::vector<int>());
    CHECK_EQ(d.error(), std::vector<int>(1));
  }
  {
    const auto s = fitoria::unexpected<std::vector<int>>(1);
    auto d = std::move(s);
    CHECK_EQ(s.error(), std::vector<int>(1));
    CHECK_EQ(d.error(), std::vector<int>(1));
  }
  {
    auto s = fitoria::unexpected<std::vector<int>>(1);
    CHECK_EQ(s.error(), std::vector<int>(1));
  }
  {
    const auto s = fitoria::unexpected<std::vector<int>>(1);
    CHECK_EQ(s.error(), std::vector<int>(1));
  }
  {
    auto s = fitoria::unexpected<std::vector<int>>(1);
    CHECK_EQ(std::move(s).error(), std::vector<int>(1));
  }
  {
    const auto s = fitoria::unexpected<std::vector<int>>(1);
    CHECK_EQ(std::move(s).error(), std::vector<int>(1));
  }
  {
    auto lhs = fitoria::unexpected<std::vector<int>>(1);
    auto rhs = fitoria::unexpected<std::vector<int>>(2);
    swap(lhs, rhs);
    CHECK_EQ(lhs.error(), std::vector<int>(2));
    CHECK_EQ(rhs.error(), std::vector<int>(1));
  }
  {
    CHECK(fitoria::unexpected<int>(1) == fitoria::unexpected<int>(1));
    CHECK(!(fitoria::unexpected<int>(1) == fitoria::unexpected<int>(2)));
    CHECK(!(fitoria::unexpected<int>(1) != fitoria::unexpected<int>(1)));
    CHECK(fitoria::unexpected<int>(1) != fitoria::unexpected<int>(2));
  }
}

TEST_CASE("bad_expected_access")
{
#if !FITORIA_NO_EXCEPTIONS
  {
    auto ex = bad_expected_access<int>(1);
    CHECK_EQ(ex.error(), 1);
  }
  {
    const auto ex = bad_expected_access<int>(1);
    CHECK_EQ(ex.error(), 1);
  }
  {
    auto ex = bad_expected_access<int>(1);
    CHECK_EQ(std::move(ex).error(), 1);
  }
  {
    const auto ex = bad_expected_access<int>(1);
    CHECK_EQ(std::move(ex).error(), 1);
  }
  {
    auto ex = bad_expected_access<int>(1);
    CHECK(strcmp(ex.what(), "bad expected access") == 0);
  }
#endif
}

TEST_CASE("trivially")
{
  {
    using type = expected<int, int>;
    static_assert(std::is_trivially_copy_constructible_v<type>);
    static_assert(std::is_trivially_copy_assignable_v<type>);
    static_assert(std::is_trivially_move_constructible_v<type>);
    static_assert(std::is_trivially_move_assignable_v<type>);
    static_assert(std::is_trivially_destructible_v<type>);
  }
  {
    using type = expected<int, std::vector<int>>;
    static_assert(!std::is_trivially_copy_constructible_v<type>);
    static_assert(!std::is_trivially_copy_assignable_v<type>);
    static_assert(!std::is_trivially_move_constructible_v<type>);
    static_assert(!std::is_trivially_move_assignable_v<type>);
    static_assert(!std::is_trivially_destructible_v<type>);
  }
  {
    using type = expected<std::vector<int>, int>;
    static_assert(!std::is_trivially_copy_constructible_v<type>);
    static_assert(!std::is_trivially_copy_assignable_v<type>);
    static_assert(!std::is_trivially_move_constructible_v<type>);
    static_assert(!std::is_trivially_move_assignable_v<type>);
    static_assert(!std::is_trivially_destructible_v<type>);
  }
  {
    using type = expected<std::vector<int>, std::vector<int>>;
    static_assert(!std::is_trivially_copy_constructible_v<type>);
    static_assert(!std::is_trivially_copy_assignable_v<type>);
    static_assert(!std::is_trivially_move_constructible_v<type>);
    static_assert(!std::is_trivially_move_assignable_v<type>);
    static_assert(!std::is_trivially_destructible_v<type>);
  }
}

TEST_CASE("default constructor")
{
  static_assert(std::is_default_constructible_v<expected<void, int>>);
  static_assert(std::is_default_constructible_v<expected<int, int>>);
  static_assert(
      std::is_default_constructible_v<expected<std::vector<int>, int>>);
  static_assert(
      !std::is_default_constructible_v<expected<test_vector<int>, int>>);
  static_assert(!std::is_default_constructible_v<expected<int&, int>>);
  {
    auto exp = expected<int, int>();
    CHECK(exp);
    CHECK_EQ(exp.value(), 0);
  }
  {
    auto exp = expected<std::vector<int>, int>();
    CHECK(exp);
    CHECK_EQ(exp.value(), std::vector<int>());
  }
  {
    auto exp = expected<test_vector<int>, int>({ 1, 2, 3 });
    CHECK(exp);
    CHECK_EQ(exp.value(), test_vector<int>({ 1, 2, 3 }));
  }
}

TEST_CASE("copy constructor")
{
  {
    auto s = expected<int, int>(1);
    auto d = expected<int, int>(s);
    CHECK_EQ(s, 1);
    CHECK_EQ(d, 1);
  }
  {
    auto s = expected<int, int>(unexpect, -1);
    auto d = expected<int, int>(s);
    CHECK_EQ(s, fitoria::unexpected<int>(-1));
    CHECK_EQ(d, fitoria::unexpected<int>(-1));
  }
}

TEST_CASE("move constructor")
{
  {
    auto s = expected<int, int>(1);
    auto d = expected<int, int>(std::move(s));
    CHECK_EQ(s, 1);
    CHECK_EQ(d, 1);
  }
  {
    auto s = expected<int, int>(unexpect, -1);
    auto d = expected<int, int>(std::move(s));
    CHECK_EQ(s, fitoria::unexpected<int>(-1));
    CHECK_EQ(d, fitoria::unexpected<int>(-1));
  }
}

TEST_CASE("perfect forwarding constructor")
{
  {
    auto exp = expected<int, int>(1);
    CHECK_EQ(exp.value(), 1);
  }
  {
    auto exp = expected<std::vector<int>, int>({ 0, 1, 2 });
    CHECK_EQ(exp.value(), std::vector<int>({ 0, 1, 2 }));
  }
}

TEST_CASE("converting constructor")
{
  {
    auto d = expected<test_vector<int>, int>(std::in_place, { 0, 1, 2 });
    auto b = expected<std::vector<int>, int>(d);
    CHECK_EQ(d.value(), std::vector<int>({ 0, 1, 2 }));
    CHECK_EQ(b.value(), std::vector<int> { 0, 1, 2 });
  }
  {
    auto d = expected<test_vector<int>, int>(unexpect, 1);
    auto b = expected<std::vector<int>, int>(d);
    CHECK_EQ(d.error(), 1);
    CHECK_EQ(b.error(), 1);
  }
  {
    auto d = expected<test_vector<int>, int>(std::in_place, { 0, 1, 2 });
    auto b = expected<std::vector<int>, int>(std::move(d));
    CHECK_EQ(d.value(), std::vector<int>({}));
    CHECK_EQ(b.value(), std::vector<int> { 0, 1, 2 });
  }
  {
    auto d = expected<test_vector<int>, int>(unexpect, 1);
    auto b = expected<std::vector<int>, int>(std::move(d));
    CHECK_EQ(d.error(), 1);
    CHECK_EQ(b.error(), 1);
  }
}

TEST_CASE("in-place constructor")
{
  {
    auto exp = expected<int, int>(std::in_place, 1);
    CHECK_EQ(exp, 1);
  }
  {
    auto exp = expected<test_vector<int>, int>(std::in_place, { 0, 1, 2 });
    CHECK_EQ(exp, test_vector<int> { 0, 1, 2 });
  }
  {
    auto exp = expected<int, int>(unexpect, -1);
    CHECK_EQ(exp, fitoria::unexpected<int>(-1));
  }
  {
    auto exp = expected<int, test_vector<int>>(unexpect, { 0, 1, 2 });
    CHECK_EQ(exp, fitoria::unexpected<test_vector<int>>({ 0, 1, 2 }));
  }
}

TEST_CASE("copy assignment operator")
{
  {
    auto s = expected<std::vector<int>, int>({ 0, 1, 2 });
    auto d = expected<std::vector<int>, int>({ 3, 4, 5 });
    d = s;
    CHECK_EQ(s, std::vector<int>({ 0, 1, 2 }));
    CHECK_EQ(d, std::vector<int>({ 0, 1, 2 }));
  }
  {
    auto s = expected<std::vector<int>, int>(unexpect, -1);
    auto d = expected<std::vector<int>, int>({ 3, 4, 5 });
    d = s;
    CHECK_EQ(s, fitoria::unexpected<int>(-1));
    CHECK_EQ(d, fitoria::unexpected<int>(-1));
  }
}

TEST_CASE("perfect forwarding assignment operator")
{
  {
    auto exp = expected<std::vector<int>, int>();
    auto val = std::vector<int>({ 0, 1, 2 });
    exp = val;
    CHECK_EQ(exp.value(), std::vector<int>({ 0, 1, 2 }));
  }
  {
    auto exp = expected<std::vector<int>, int>(unexpect, 0);
    auto val = std::vector<int>({ 0, 1, 2 });
    exp = val;
    CHECK_EQ(exp.value(), std::vector<int>({ 0, 1, 2 }));
  }
  {
    auto exp = expected<std::vector<int>, int>();
    auto val = std::vector<int>({ 0, 1, 2 });
    exp = std::move(val);
    CHECK_EQ(exp.value(), std::vector<int>({ 0, 1, 2 }));
  }
  {
    auto exp = expected<std::vector<int>, int>(unexpect, 0);
    auto val = std::vector<int>({ 0, 1, 2 });
    exp = std::move(val);
    CHECK_EQ(exp.value(), std::vector<int>({ 0, 1, 2 }));
  }
}

TEST_CASE("converting assignment operator")
{
  {
    auto exp = expected<int, int>();
    auto une = fitoria::unexpected<int>(2);
    exp = une;
    CHECK_EQ(exp.error(), 2);
  }
  {
    auto exp = expected<int, int>(unexpect, 1);
    auto une = fitoria::unexpected<int>(2);
    exp = une;
    CHECK_EQ(exp.error(), 2);
  }
  {
    auto exp = expected<int, int>();
    auto une = fitoria::unexpected<int>(2);
    exp = std::move(une);
    CHECK_EQ(exp.error(), 2);
  }
  {
    auto exp = expected<int, int>(unexpect, 1);
    auto une = fitoria::unexpected<int>(2);
    exp = std::move(une);
    CHECK_EQ(exp.error(), 2);
  }
}

TEST_CASE("operator->")
{
  static_assert(std::is_same_v<memop_mem_of<expected<char, int>&>, char*>);
  static_assert(
      std::is_same_v<memop_mem_of<const expected<char, int>&>, const char*>);
  static_assert(std::is_same_v<memop_mem_of<expected<char, int>>, char*>);
  static_assert(
      std::is_same_v<memop_mem_of<const expected<char, int>>, const char*>);
  {
    auto exp = expected<int, int>(1);
    CHECK_EQ(*exp.operator->(), 1);
  }
  {
    const auto exp = expected<int, int>(1);
    CHECK_EQ(*exp.operator->(), 1);
  }
  {
    auto exp = expected<std::vector<int>, int>({ 0, 1, 2 });
    CHECK_EQ(exp->size(), 3);
  }
  {
    const auto exp = expected<std::vector<int>, int>({ 0, 1, 2 });
    CHECK_EQ(exp->size(), 3);
  }
}

TEST_CASE("operator*")
{
  static_assert(std::is_same_v<memop_deref<expected<char, int>&>, char&>);
  static_assert(
      std::is_same_v<memop_deref<const expected<char, int>&>, const char&>);
  static_assert(std::is_same_v<memop_deref<expected<char, int>>, char&&>);
  static_assert(
      std::is_same_v<memop_deref<const expected<char, int>>, const char&&>);
  {
    auto exp = expected<int, int>(1);
    CHECK_EQ(*exp, 1);
  }
  {
    const auto exp = expected<int, int>(1);
    CHECK_EQ(*exp, 1);
  }
  {
    auto exp = expected<int, int>(1);
    CHECK_EQ(*std::move(exp), 1);
  }
  {
    const auto exp = expected<int, int>(1);
    CHECK_EQ(*std::move(exp), 1);
  }
  {
    auto exp = expected<std::vector<int>, int>({ 0, 1, 2 });
    CHECK_EQ(*exp, std::vector<int>({ 0, 1, 2 }));
  }
  {
    const auto exp = expected<std::vector<int>, int>({ 0, 1, 2 });
    CHECK_EQ(*exp, std::vector<int>({ 0, 1, 2 }));
  }
  {
    auto exp = expected<std::vector<int>, int>({ 0, 1, 2 });
    CHECK_EQ(*std::move(exp), std::vector<int>({ 0, 1, 2 }));
  }
  {
    const auto exp = expected<std::vector<int>, int>({ 0, 1, 2 });
    CHECK_EQ(*std::move(exp), std::vector<int>({ 0, 1, 2 }));
  }
  {
    auto exp = expected<std::vector<int>, std::vector<int>>({ 0, 1, 2 });
    CHECK_EQ(*exp, std::vector<int>({ 0, 1, 2 }));
  }
  {
    const auto exp = expected<std::vector<int>, std::vector<int>>({ 0, 1, 2 });
    CHECK_EQ(*exp, std::vector<int>({ 0, 1, 2 }));
  }
  {
    auto exp = expected<std::vector<int>, std::vector<int>>({ 0, 1, 2 });
    CHECK_EQ(*std::move(exp), std::vector<int>({ 0, 1, 2 }));
  }
  {
    const auto exp = expected<std::vector<int>, std::vector<int>>({ 0, 1, 2 });
    CHECK_EQ(*std::move(exp), std::vector<int>({ 0, 1, 2 }));
  }
}

TEST_CASE("has_value")
{
  CHECK(expected<int, int>().has_value());
  CHECK(!expected<int, int>(unexpect, 1).has_value());
}

TEST_CASE("value")
{
  static_assert(std::is_same_v<memfn_value<expected<char, int>&>, char&>);
  static_assert(
      std::is_same_v<memfn_value<const expected<char, int>&>, const char&>);
  static_assert(std::is_same_v<memfn_value<expected<char, int>>, char&&>);
  static_assert(
      std::is_same_v<memfn_value<const expected<char, int>>, const char&&>);
  {
    auto exp = expected<int, int>(1);
    CHECK_EQ(exp.value(), 1);
  }
  {
    const auto exp = expected<int, int>(1);
    CHECK_EQ(exp.value(), 1);
  }
  {
    auto exp = expected<int, int>(1);
    CHECK_EQ(std::move(exp).value(), 1);
  }
  {
    const auto exp = expected<int, int>(1);
    CHECK_EQ(std::move(exp).value(), 1);
  }
#if !FITORIA_NO_EXCEPTIONS
  {
    auto exp = expected<int, int>(unexpect);
    CHECK_THROWS_AS(exp.value(), bad_expected_access<int>);
  }
  {
    const auto exp = expected<int, int>(unexpect);
    CHECK_THROWS_AS(exp.value(), bad_expected_access<int>);
  }
  {
    auto exp = expected<int, int>(unexpect);
    CHECK_THROWS_AS(std::move(exp).value(), bad_expected_access<int>);
  }
  {
    const auto exp = expected<int, int>(unexpect);
    CHECK_THROWS_AS(std::move(exp).value(), bad_expected_access<int>);
  }
#endif
  {
    auto exp = expected<std::vector<int>, int>({ 0, 1, 2 });
    CHECK_EQ(exp.value(), std::vector<int>({ 0, 1, 2 }));
  }
  {
    const auto exp = expected<std::vector<int>, int>({ 0, 1, 2 });
    CHECK_EQ(exp.value(), std::vector<int>({ 0, 1, 2 }));
  }
  {
    auto exp = expected<std::vector<int>, int>({ 0, 1, 2 });
    CHECK_EQ(std::move(exp).value(), std::vector<int>({ 0, 1, 2 }));
  }
  {
    const auto exp = expected<std::vector<int>, int>({ 0, 1, 2 });
    CHECK_EQ(std::move(exp).value(), std::vector<int>({ 0, 1, 2 }));
  }
  {
    auto exp = expected<std::vector<int>, std::vector<int>>({ 0, 1, 2 });
    CHECK_EQ(exp.value(), std::vector<int>({ 0, 1, 2 }));
  }
  {
    const auto exp = expected<std::vector<int>, std::vector<int>>({ 0, 1, 2 });
    CHECK_EQ(exp.value(), std::vector<int>({ 0, 1, 2 }));
  }
  {
    auto exp = expected<std::vector<int>, std::vector<int>>({ 0, 1, 2 });
    CHECK_EQ(std::move(exp).value(), std::vector<int>({ 0, 1, 2 }));
  }
  {
    const auto exp = expected<std::vector<int>, std::vector<int>>({ 0, 1, 2 });
    CHECK_EQ(std::move(exp).value(), std::vector<int>({ 0, 1, 2 }));
  }
}

TEST_CASE("error")
{
  static_assert(std::is_same_v<memfn_error<expected<char, int>&>, int&>);
  static_assert(
      std::is_same_v<memfn_error<const expected<char, int>&>, const int&>);
  static_assert(std::is_same_v<memfn_error<expected<char, int>>, int&&>);
  static_assert(
      std::is_same_v<memfn_error<const expected<char, int>>, const int&&>);
  {
    auto exp = expected<int, int>(unexpect, 1);
    CHECK_EQ(exp.error(), 1);
  }
  {
    const auto exp = expected<int, int>(unexpect, 1);
    CHECK_EQ(exp.error(), 1);
  }
  {
    auto exp = expected<int, int>(unexpect, 1);
    CHECK_EQ(std::move(exp).error(), 1);
  }
  {
    const auto exp = expected<int, int>(unexpect, 1);
    CHECK_EQ(std::move(exp).error(), 1);
  }
  {
    auto exp = expected<std::vector<int>, int>(unexpect, 1);
    CHECK_EQ(exp.error(), 1);
  }
  {
    const auto exp = expected<std::vector<int>, int>(unexpect, 1);
    CHECK_EQ(exp.error(), 1);
  }
  {
    auto exp = expected<std::vector<int>, int>(unexpect, 1);
    CHECK_EQ(std::move(exp).error(), 1);
  }
  {
    const auto exp = expected<std::vector<int>, int>(unexpect, 1);
    CHECK_EQ(std::move(exp).error(), 1);
  }
  {
    auto exp = expected<std::vector<int>, std::vector<int>>(unexpect, 1);
    CHECK_EQ(exp.error(), std::vector<int>(1));
  }
  {
    const auto exp = expected<std::vector<int>, std::vector<int>>(unexpect, 1);
    CHECK_EQ(exp.error(), std::vector<int>(1));
  }
  {
    auto exp = expected<std::vector<int>, std::vector<int>>(unexpect, 1);
    CHECK_EQ(std::move(exp).error(), std::vector<int>(1));
  }
  {
    const auto exp = expected<std::vector<int>, std::vector<int>>(unexpect, 1);
    CHECK_EQ(std::move(exp).error(), std::vector<int>(1));
  }
}

TEST_CASE("value_or")
{
  {
    auto exp = expected<int, int>(1);
    CHECK_EQ(exp.value_or(2LL), 1);
  }
  {
    auto exp = expected<int, int>(1);
    CHECK_EQ(std::move(exp).value_or(2LL), 1);
  }
  {
    auto exp = expected<int, int>(unexpect, 1);
    CHECK_EQ(exp.value_or(2LL), 2);
  }
  {
    auto exp = expected<int, int>(unexpect, 1);
    CHECK_EQ(std::move(exp).value_or(2LL), 2);
  }
}

TEST_CASE("error_or")
{
  {
    auto exp = expected<int, int>(1);
    CHECK_EQ(exp.error_or(2LL), 2);
  }
  {
    auto exp = expected<int, int>(1);
    CHECK_EQ(std::move(exp).error_or(2LL), 2);
  }
  {
    auto exp = expected<int, int>(unexpect, 1);
    CHECK_EQ(exp.error_or(2LL), 1);
  }
  {
    auto exp = expected<int, int>(unexpect, 1);
    CHECK_EQ(std::move(exp).error_or(2LL), 1);
  }
}

struct nothrow_list_initializable {
  nothrow_list_initializable(std::initializer_list<int> list) noexcept
      : vec_(list)
  {
  }

  friend bool operator==(const nothrow_list_initializable&,
                         const nothrow_list_initializable&)
      = default;

  std::vector<int> vec_;
};

TEST_CASE("emplace")
{
  {
    auto e = expected<std::vector<int>, int>({ 0, 1, 2 });
    e.emplace(std::vector<int> { 3, 4, 5 });
    CHECK_EQ(e, std::vector<int> { 3, 4, 5 });
  }
  {
    auto e = expected<nothrow_list_initializable, int>(unexpect, 0);
    e.emplace({ 3, 4, 5 });
    CHECK_EQ(e, nothrow_list_initializable { 3, 4, 5 });
  }
}

TEST_CASE("and_then")
{
  {
    auto exp = expected<int, int>(1);
    CHECK_EQ(
        exp.and_then([](int val) -> expected<int, int> { return val + 1; }), 2);
  }
  {
    const auto exp = expected<int, int>(1);
    CHECK_EQ(
        exp.and_then([](int val) -> expected<int, int> { return val + 1; }), 2);
  }
  {
    auto exp = expected<int, int>(1);
    CHECK_EQ(std::move(exp).and_then(
                 [](int val) -> expected<int, int> { return val + 1; }),
             2);
  }
  {
    const auto exp = expected<int, int>(1);
    CHECK_EQ(std::move(exp).and_then(
                 [](int val) -> expected<int, int> { return val + 1; }),
             2);
  }
  {
    auto exp = expected<int, int>(unexpect, -1);
    CHECK_EQ(
        exp.and_then([](int val) -> expected<int, int> { return val + 1; }),
        fitoria::unexpected<int>(-1));
  }
  {
    const auto exp = expected<int, int>(unexpect, -1);
    CHECK_EQ(
        exp.and_then([](int val) -> expected<int, int> { return val + 1; }),
        fitoria::unexpected<int>(-1));
  }
  {
    auto exp = expected<int, int>(unexpect, -1);
    CHECK_EQ(std::move(exp).and_then(
                 [](int val) -> expected<int, int> { return val + 1; }),
             fitoria::unexpected<int>(-1));
  }
  {
    const auto exp = expected<int, int>(unexpect, -1);
    CHECK_EQ(std::move(exp).and_then(
                 [](int val) -> expected<int, int> { return val + 1; }),
             fitoria::unexpected<int>(-1));
  }
}

TEST_CASE("or_else")
{
  {
    auto exp = expected<int, int>(1);
    CHECK_EQ(exp.or_else([](int err) -> expected<int, int> { return err - 1; }),
             1);
  }
  {
    const auto exp = expected<int, int>(1);
    CHECK_EQ(exp.or_else([](int err) -> expected<int, int> { return err - 1; }),
             1);
  }
  {
    auto exp = expected<int, int>(1);
    CHECK_EQ(std::move(exp).or_else(
                 [](int err) -> expected<int, int> { return err - 1; }),
             1);
  }
  {
    const auto exp = expected<int, int>(1);
    CHECK_EQ(std::move(exp).or_else(
                 [](int err) -> expected<int, int> { return err - 1; }),
             1);
  }
  {
    auto exp = expected<int, int>(unexpect, -1);
    CHECK_EQ(exp.or_else([](int err) -> expected<int, int> {
      return fitoria::unexpected<int>(err - 1);
    }),
             fitoria::unexpected<int>(-2));
  }
  {
    const auto exp = expected<int, int>(unexpect, -1);
    CHECK_EQ(exp.or_else([](int err) -> expected<int, int> {
      return fitoria::unexpected<int>(err - 1);
    }),
             fitoria::unexpected<int>(-2));
  }
  {
    auto exp = expected<int, int>(unexpect, -1);
    CHECK_EQ(std::move(exp).or_else([](int err) -> expected<int, int> {
      return fitoria::unexpected<int>(err - 1);
    }),
             fitoria::unexpected<int>(-2));
  }
  {
    const auto exp = expected<int, int>(unexpect, -1);
    CHECK_EQ(std::move(exp).or_else([](int err) -> expected<int, int> {
      return fitoria::unexpected<int>(err - 1);
    }),
             fitoria::unexpected<int>(-2));
  }
}

TEST_CASE("transform")
{
  {
    auto exp = expected<int, int>(1);
    CHECK_EQ(exp.transform([](int val) -> int { return val + 1; }), 2);
  }
  {
    const auto exp = expected<int, int>(1);
    CHECK_EQ(exp.transform([](int val) -> int { return val + 1; }), 2);
  }
  {
    auto exp = expected<int, int>(1);
    CHECK_EQ(std::move(exp).transform([](int val) -> int { return val + 1; }),
             2);
  }
  {
    const auto exp = expected<int, int>(1);
    CHECK_EQ(std::move(exp).transform([](int val) -> int { return val + 1; }),
             2);
  }
  {
    auto exp = expected<int, int>(unexpect, -1);
    CHECK_EQ(exp.transform([](int val) -> int { return val + 1; }),
             fitoria::unexpected<int>(-1));
  }
  {
    const auto exp = expected<int, int>(unexpect, -1);
    CHECK_EQ(exp.transform([](int val) -> int { return val + 1; }),
             fitoria::unexpected<int>(-1));
  }
  {
    auto exp = expected<int, int>(unexpect, -1);
    CHECK_EQ(std::move(exp).transform([](int val) -> int { return val + 1; }),
             fitoria::unexpected<int>(-1));
  }
  {
    const auto exp = expected<int, int>(unexpect, -1);
    CHECK_EQ(std::move(exp).transform([](int val) -> int { return val + 1; }),
             fitoria::unexpected<int>(-1));
  }
}

TEST_CASE("transform_error")
{
  {
    auto exp = expected<int, int>(1);
    CHECK_EQ(exp.transform_error([](int err) -> int { return err - 1; }), 1);
  }
  {
    const auto exp = expected<int, int>(1);
    CHECK_EQ(exp.transform_error([](int err) -> int { return err - 1; }), 1);
  }
  {
    auto exp = expected<int, int>(1);
    CHECK_EQ(
        std::move(exp).transform_error([](int err) -> int { return err - 1; }),
        1);
  }
  {
    const auto exp = expected<int, int>(1);
    CHECK_EQ(
        std::move(exp).transform_error([](int err) -> int { return err - 1; }),
        1);
  }
  {
    auto exp = expected<int, int>(unexpect, -1);
    CHECK_EQ(exp.transform_error([](int err) -> int { return err - 1; }),
             fitoria::unexpected<int>(-2));
  }
  {
    const auto exp = expected<int, int>(unexpect, -1);
    CHECK_EQ(exp.transform_error([](int err) -> int { return err - 1; }),
             fitoria::unexpected<int>(-2));
  }
  {
    auto exp = expected<int, int>(unexpect, -1);
    CHECK_EQ(
        std::move(exp).transform_error([](int err) -> int { return err - 1; }),
        fitoria::unexpected<int>(-2));
  }
  {
    const auto exp = expected<int, int>(unexpect, -1);
    CHECK_EQ(
        std::move(exp).transform_error([](int err) -> int { return err - 1; }),
        fitoria::unexpected<int>(-2));
  }
}

TEST_CASE("swap")
{
  {
    auto a
        = expected<std::vector<int>, std::vector<int>>(unexpect, { 7, 8, 9 });
    auto b = expected<std::vector<int>, std::vector<int>>(unexpect, 0);
    swap(a, b);
    CHECK_EQ(a.error(), std::vector<int>(0));
    CHECK_EQ(b.error(), std::vector<int>({ 7, 8, 9 }));
  }
  {
    auto a
        = expected<std::vector<int>, std::vector<int>>(unexpect, { 7, 8, 9 });
    auto b = expected<std::vector<int>, std::vector<int>>(1);
    swap(a, b);
    CHECK_EQ(a.value(), std::vector<int>(1));
    CHECK_EQ(b.error(), std::vector<int>({ 7, 8, 9 }));
  }
  {
    auto a = expected<std::vector<int>, std::vector<int>>(std::in_place,
                                                          { 0, 1, 2 });
    auto b = expected<std::vector<int>, std::vector<int>>(unexpect, 0);
    swap(a, b);
    CHECK_EQ(a.error(), std::vector<int>(0));
    CHECK_EQ(b.value(), std::vector<int>({ 0, 1, 2 }));
  }
  {
    auto a = expected<std::vector<int>, std::vector<int>>(std::in_place,
                                                          { 0, 1, 2 });
    auto b = expected<std::vector<int>, std::vector<int>>(1);
    swap(a, b);
    CHECK_EQ(a.value(), std::vector<int>(1));
    CHECK_EQ(b.value(), std::vector<int>({ 0, 1, 2 }));
  }

#if !FITORIA_NO_EXCEPTIONS
  class move_throw {
  public:
    move_throw(int val)
        : val_(val)
    {
    }

    move_throw(const move_throw&) = default;
    move_throw(move_throw&& other)
        : val_(other.val_)
    {
      if (val_ < 0) {
        throw val_;
      }
    }
    move_throw& operator=(const move_throw&) = default;
    move_throw& operator=(move_throw&&) = default;

    int v() const noexcept
    {
      return val_;
    }

  private:
    int val_ = 0;
  };
  static_assert(!std::is_nothrow_move_constructible_v<move_throw>);
  {
    auto a = expected<move_throw, int>(1);
    auto b = expected<move_throw, int>(unexpect, 2);
    swap(a, b);
    CHECK_EQ(a.error(), 2);
    CHECK_EQ(b.value().v(), 1);
  }
  {
    auto a = expected<int, move_throw>(1);
    auto b = expected<int, move_throw>(unexpect, 2);
    swap(a, b);
    CHECK_EQ(a.error().v(), 2);
    CHECK_EQ(b.value(), 1);
  }
  {
    auto a = expected<move_throw, int>(-1);
    auto b = expected<move_throw, int>(unexpect, 2);
    CHECK_THROWS_AS(swap(a, b), int);
  }
  {
    auto a = expected<int, move_throw>(1);
    auto b = expected<int, move_throw>(unexpect, -1);
    CHECK_THROWS_AS(swap(a, b), int);
  }
#endif
}

TEST_CASE("compare")
{
  CHECK(expected<int, int>() == expected<int, int>());
  CHECK(!(expected<int, int>() != expected<int, int>()));
  CHECK(expected<int, int>(unexpect) == expected<int, int>(unexpect));
  CHECK(!(expected<int, int>(unexpect) != expected<int, int>(unexpect)));
  CHECK(!(expected<int, int>() == expected<int, int>(unexpect)));
  CHECK(expected<int, int>() != expected<int, int>(unexpect));
  CHECK(!(expected<int, int>(unexpect) == expected<int, int>()));
  CHECK(expected<int, int>(unexpect) != expected<int, int>());

  CHECK(expected<int, int>() == 0);
  CHECK(!(expected<int, int>(unexpect) == 0));
  CHECK(!(expected<int, int>() != 0));
  CHECK(expected<int, int>(unexpect) != 0);

  CHECK(!(expected<int, int>() == fitoria::unexpected<int>(0)));
  CHECK(expected<int, int>(unexpect) == fitoria::unexpected<int>(0));
  CHECK(expected<int, int>() != fitoria::unexpected<int>(0));
  CHECK(!(expected<int, int>(unexpect) != fitoria::unexpected<int>(0)));
}

TEST_CASE("format")
{
  {
    CHECK_EQ(fmt::format("{}", expected<int, int>()), "0");
    CHECK_EQ(fmt::format("{}", expected<int, int>(unexpect)), "{unexpected}");
  }
}

TEST_SUITE_END();
