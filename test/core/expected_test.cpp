//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria_test.h>

#include <fitoria/core/expected.hpp>

#include <cstring>

using namespace fitoria;

TEST_SUITE_BEGIN("expected");

namespace {
template <typename T>
class dc : public std::vector<T> {
public:
  using std::vector<T>::vector;

  dc() = default;

  dc(T val) noexcept
      : std::vector<T>({ val })
  {
  }
};

template <typename T>
class ndc : public std::vector<T> {
public:
  using std::vector<T>::vector;

  ndc() = delete;

  ndc(T val) noexcept
      : std::vector<T>({ val })
  {
  }
};

template <typename Exp>
using memof_op = decltype(std::declval<Exp>().operator->());

template <typename Exp>
using deref_op = decltype(std::declval<Exp>().operator*());

template <typename Exp>
using fn_value = decltype(std::declval<Exp>().value());

template <typename Exp>
using fn_error = decltype(std::declval<Exp>().error());

template <typename F>
struct fn_and_then {
  template <typename Exp>
  using fn = decltype(std::declval<Exp>().and_then(std::declval<F>()));
};

template <typename F>
struct fn_or_else {
  template <typename Exp>
  using fn = decltype(std::declval<Exp>().or_else(std::declval<F>()));
};

template <typename F>
struct fn_transform {
  template <typename Exp>
  using fn = decltype(std::declval<Exp>().transform(std::declval<F>()));
};

template <typename F>
struct fn_transform_error {
  template <typename Exp>
  using fn = decltype(std::declval<Exp>().transform_error(std::declval<F>()));
};

template <template <typename...> class Op, typename Test, typename Expected>
struct assert_is_same {
  static_assert(std::is_same_v<Op<Test>, Expected>);
};
}

TEST_CASE("unexpected")
{
  {
    auto s = fitoria::unexpected<int>(1);
    CHECK_EQ(s.error(), 1);
  }
  {
    auto s = fitoria::unexpected<dc<int>>(std::in_place, 1);
    CHECK_EQ(s.error(), dc<int>(1));
  }
  {
    auto s = fitoria::unexpected<dc<int>>(std::in_place, { 0, 1, 2 });
    CHECK_EQ(s.error(), dc<int>({ 0, 1, 2 }));
  }
  {
    auto s = fitoria::unexpected<dc<int>>(1);
    fitoria::unexpected<dc<int>> d(s);
    CHECK_EQ(s.error(), dc<int>(1));
    CHECK_EQ(d.error(), dc<int>(1));
  }
  {
    const auto s = fitoria::unexpected<dc<int>>(1);
    fitoria::unexpected<dc<int>> d(s);
    CHECK_EQ(s.error(), dc<int>(1));
    CHECK_EQ(d.error(), dc<int>(1));
  }
  {
    auto s = fitoria::unexpected<dc<int>>(1);
    fitoria::unexpected<dc<int>> d(std::move(s));
    CHECK_EQ(s.error(), dc<int>());
    CHECK_EQ(d.error(), dc<int>(1));
  }
  {
    const auto s = fitoria::unexpected<dc<int>>(1);
    fitoria::unexpected<dc<int>> d(std::move(s));
    CHECK_EQ(s.error(), dc<int>(1));
    CHECK_EQ(d.error(), dc<int>(1));
  }
  {
    auto s = fitoria::unexpected<dc<int>>(1);
    auto d = s;
    CHECK_EQ(s.error(), dc<int>(1));
    CHECK_EQ(d.error(), dc<int>(1));
  }
  {
    const auto s = fitoria::unexpected<dc<int>>(1);
    auto d = s;
    CHECK_EQ(s.error(), dc<int>(1));
    CHECK_EQ(d.error(), dc<int>(1));
  }
  {
    auto s = fitoria::unexpected<dc<int>>(1);
    auto d = std::move(s);
    CHECK_EQ(s.error(), dc<int>());
    CHECK_EQ(d.error(), dc<int>(1));
  }
  {
    const auto s = fitoria::unexpected<dc<int>>(1);
    auto d = std::move(s);
    CHECK_EQ(s.error(), dc<int>(1));
    CHECK_EQ(d.error(), dc<int>(1));
  }
  {
    auto s = fitoria::unexpected<dc<int>>(1);
    CHECK_EQ(s.error(), dc<int>(1));
  }
  {
    const auto s = fitoria::unexpected<dc<int>>(1);
    CHECK_EQ(s.error(), dc<int>(1));
  }
  {
    auto s = fitoria::unexpected<dc<int>>(1);
    CHECK_EQ(std::move(s).error(), dc<int>(1));
  }
  {
    const auto s = fitoria::unexpected<dc<int>>(1);
    CHECK_EQ(std::move(s).error(), dc<int>(1));
  }
  {
    auto lhs = fitoria::unexpected<dc<int>>(1);
    auto rhs = fitoria::unexpected<dc<int>>(2);
    swap(lhs, rhs);
    CHECK_EQ(lhs.error(), dc<int>(2));
    CHECK_EQ(rhs.error(), dc<int>(1));
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
    using type = expected<void, int>;
    static_assert(std::is_trivially_copy_constructible_v<type>);
    static_assert(std::is_trivially_copy_assignable_v<type>);
    static_assert(std::is_trivially_move_constructible_v<type>);
    static_assert(std::is_trivially_move_assignable_v<type>);
    static_assert(std::is_trivially_destructible_v<type>);
  }
  {
    using type = expected<int, int>;
    static_assert(std::is_trivially_copy_constructible_v<type>);
    static_assert(std::is_trivially_copy_assignable_v<type>);
    static_assert(std::is_trivially_move_constructible_v<type>);
    static_assert(std::is_trivially_move_assignable_v<type>);
    static_assert(std::is_trivially_destructible_v<type>);
  }
  {
    using type = expected<int, dc<int>>;
    static_assert(!std::is_trivially_copy_constructible_v<type>);
    static_assert(!std::is_trivially_copy_assignable_v<type>);
    static_assert(!std::is_trivially_move_constructible_v<type>);
    static_assert(!std::is_trivially_move_assignable_v<type>);
    static_assert(!std::is_trivially_destructible_v<type>);
  }
  {
    using type = expected<dc<int>, int>;
    static_assert(!std::is_trivially_copy_constructible_v<type>);
    static_assert(!std::is_trivially_copy_assignable_v<type>);
    static_assert(!std::is_trivially_move_constructible_v<type>);
    static_assert(!std::is_trivially_move_assignable_v<type>);
    static_assert(!std::is_trivially_destructible_v<type>);
  }
  {
    using type = expected<dc<int>, dc<int>>;
    static_assert(!std::is_trivially_copy_constructible_v<type>);
    static_assert(!std::is_trivially_copy_assignable_v<type>);
    static_assert(!std::is_trivially_move_constructible_v<type>);
    static_assert(!std::is_trivially_move_assignable_v<type>);
    static_assert(!std::is_trivially_destructible_v<type>);
  }
  {
    using type = expected<int&, int>;
    static_assert(std::is_trivially_copy_constructible_v<type>);
    static_assert(std::is_trivially_copy_assignable_v<type>);
    static_assert(std::is_trivially_move_constructible_v<type>);
    static_assert(std::is_trivially_move_assignable_v<type>);
    static_assert(std::is_trivially_destructible_v<type>);
  }
  {
    using type = expected<int&, dc<int>>;
    static_assert(!std::is_trivially_copy_constructible_v<type>);
    static_assert(!std::is_trivially_copy_assignable_v<type>);
    static_assert(!std::is_trivially_move_constructible_v<type>);
    static_assert(!std::is_trivially_move_assignable_v<type>);
    static_assert(!std::is_trivially_destructible_v<type>);
  }
}

TEST_CASE("default constructor")
{
  static_assert(std::is_default_constructible<expected<void, int>>::value);
  static_assert(std::is_default_constructible<expected<int, int>>::value);
  static_assert(std::is_default_constructible<expected<dc<int>, int>>::value);
  static_assert(!std::is_default_constructible<expected<ndc<int>, int>>::value);
  static_assert(!std::is_default_constructible<expected<int&, int>>::value);
  {
    auto exp = expected<void, int>();
    CHECK(exp);
  }
  {
    auto exp = expected<int, int>();
    CHECK(exp);
    CHECK_EQ(exp.value(), 0);
  }
  {
    auto exp = expected<dc<int>, int>();
    CHECK(exp);
    CHECK_EQ(exp.value(), dc<int>());
  }
  {
    auto exp = expected<ndc<int>, int>(1);
    CHECK(exp);
    CHECK_EQ(exp.value(), ndc<int>(1));
  }
}

TEST_CASE("copy/move constructor")
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
    auto s = expected<void, dc<int>>(std::in_place);
    expected<void, dc<int>> d(s);
    CHECK(s);
    CHECK(d);
  }
  {
    auto s = expected<void, dc<int>>(unexpect, { 0, 1, 2 });
    expected<void, dc<int>> d(s);
    CHECK_EQ(s.error(), dc<int>({ 0, 1, 2 }));
    CHECK_EQ(d.error(), dc<int>({ 0, 1, 2 }));
  }
  {
    auto s = expected<void, dc<int>>(std::in_place);
    expected<void, dc<int>> d(std::move(s));
    CHECK(s);
    CHECK(d);
  }
  {
    auto s = expected<void, dc<int>>(unexpect, { 0, 1, 2 });
    expected<void, dc<int>> d(std::move(s));
    CHECK_EQ(s.error(), dc<int>());
    CHECK_EQ(d.error(), dc<int>({ 0, 1, 2 }));
  }
  {
    auto s = expected<int, int>(std::in_place, 1);
    expected<int, int> d(s);
    CHECK_EQ(s.value(), 1);
    CHECK_EQ(d.value(), 1);
  }
  {
    auto s = expected<int, int>(unexpect, 1);
    expected<int, int> d(s);
    CHECK_EQ(s.error(), 1);
    CHECK_EQ(d.error(), 1);
  }
  {
    auto s = expected<int, int>(std::in_place, 1);
    expected<int, int> d(std::move(s));
    CHECK_EQ(s.value(), 1);
    CHECK_EQ(d.value(), 1);
  }
  {
    auto s = expected<int, int>(unexpect, 1);
    expected<int, int> d(std::move(s));
    CHECK_EQ(s.error(), 1);
    CHECK_EQ(d.error(), 1);
  }
  {
    auto s = expected<dc<int>, dc<int>>(std::in_place, { 0, 1, 2 });
    expected<dc<int>, dc<int>> d(s);
    CHECK_EQ(s.value(), dc<int>({ 0, 1, 2 }));
    CHECK_EQ(d.value(), dc<int>({ 0, 1, 2 }));
  }
  {
    auto s = expected<dc<int>, dc<int>>(unexpect, { 3, 4, 5 });
    expected<dc<int>, dc<int>> d(s);
    CHECK_EQ(s.error(), dc<int>({ 3, 4, 5 }));
    CHECK_EQ(d.error(), dc<int>({ 3, 4, 5 }));
  }
  {
    auto s = expected<dc<int>, dc<int>>(std::in_place, { 0, 1, 2 });
    expected<dc<int>, dc<int>> d(std::move(s));
    CHECK_EQ(s.value(), dc<int>());
    CHECK_EQ(d.value(), dc<int>({ 0, 1, 2 }));
  }
  {
    auto s = expected<dc<int>, dc<int>>(unexpect, { 3, 4, 5 });
    expected<dc<int>, dc<int>> d(std::move(s));
    CHECK_EQ(s.error(), dc<int>());
    CHECK_EQ(d.error(), dc<int>({ 3, 4, 5 }));
  }
  {
    auto val = dc<int>({ 0, 1, 2 });
    auto s = expected<dc<int>&, dc<int>>(val);
    expected<dc<int>&, dc<int>> d(s);
    CHECK_EQ(s.value(), dc<int>({ 0, 1, 2 }));
    CHECK_EQ(d.value(), dc<int>({ 0, 1, 2 }));
  }
  {
    auto s = expected<dc<int>&, dc<int>>(unexpect, { 3, 4, 5 });
    expected<dc<int>&, dc<int>> d(s);
    CHECK_EQ(s.error(), dc<int>({ 3, 4, 5 }));
    CHECK_EQ(d.error(), dc<int>({ 3, 4, 5 }));
  }
  {
    auto val = dc<int>({ 0, 1, 2 });
    auto s = expected<dc<int>&, dc<int>>(val);
    expected<dc<int>&, dc<int>> d(std::move(s));
    CHECK_EQ(s.value(), dc<int>({ 0, 1, 2 }));
    CHECK_EQ(d.value(), dc<int>({ 0, 1, 2 }));
  }
  {
    auto s = expected<dc<int>&, dc<int>>(unexpect, { 3, 4, 5 });
    expected<dc<int>&, dc<int>> d(std::move(s));
    CHECK_EQ(s.error(), dc<int>());
    CHECK_EQ(d.error(), dc<int>({ 3, 4, 5 }));
  }
}

TEST_CASE("perfect forwarding constructor")
{
  {
    auto exp = expected<int, int>(1);
    CHECK_EQ(exp.value(), 1);
  }
  {
    auto exp = expected<dc<int>, int>({ 0, 1, 2 });
    CHECK_EQ(exp.value(), dc<int>({ 0, 1, 2 }));
  }
  {
    int val = 1;
    auto exp = expected<int&, int>(val);
    CHECK_EQ(exp.value(), 1);
  }
}

TEST_CASE("converting constructor")
{
  {
    auto d = expected<dc<int>, int>(std::in_place, { 0, 1, 2 });
    auto b = expected<std::vector<int>, int>(d);
    CHECK_EQ(d.value(), dc<int>({ 0, 1, 2 }));
    CHECK_EQ(b.value(), std::vector<int> { 0, 1, 2 });
  }
  {
    auto d = expected<dc<int>, int>(unexpect, 1);
    auto b = expected<std::vector<int>, int>(d);
    CHECK_EQ(d.error(), 1);
    CHECK_EQ(b.error(), 1);
  }
  {
    auto d = expected<dc<int>, int>(std::in_place, { 0, 1, 2 });
    auto b = expected<std::vector<int>, int>(std::move(d));
    CHECK_EQ(d.value(), dc<int>({}));
    CHECK_EQ(b.value(), std::vector<int> { 0, 1, 2 });
  }
  {
    auto d = expected<dc<int>, int>(unexpect, 1);
    auto b = expected<std::vector<int>, int>(std::move(d));
    CHECK_EQ(d.error(), 1);
    CHECK_EQ(b.error(), 1);
  }
  {
    auto val = dc<int>({ 0, 1, 2 });
    auto d = expected<dc<int>&, int>(val);
    auto b = expected<std::vector<int>&, int>(d);
    CHECK_EQ(d.value(), dc<int>({ 0, 1, 2 }));
    CHECK_EQ(b.value(), std::vector<int> { 0, 1, 2 });
  }
  {
    auto d = expected<dc<int>&, int>(unexpect, 1);
    auto b = expected<std::vector<int>&, int>(d);
    CHECK_EQ(d.error(), 1);
    CHECK_EQ(b.error(), 1);
  }
  {
    auto val = dc<int>({ 0, 1, 2 });
    auto d = expected<dc<int>&, int>(val);
    auto b = expected<std::vector<int>&, int>(std::move(d));
    CHECK_EQ(d.value(), dc<int>({ 0, 1, 2 }));
    CHECK_EQ(b.value(), std::vector<int> { 0, 1, 2 });
  }
  {
    auto d = expected<dc<int>&, int>(unexpect, 1);
    auto b = expected<std::vector<int>&, int>(std::move(d));
    CHECK_EQ(d.error(), 1);
    CHECK_EQ(b.error(), 1);
  }
  {
    auto d = fitoria::unexpected<int>(1);
    auto b = expected<ndc<int>, int>(d);
    CHECK_EQ(b.error(), 1);
  }
  {
    auto d = fitoria::unexpected<int>(1);
    auto b = expected<ndc<int>, int>(std::move(d));
    CHECK_EQ(b.error(), 1);
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
    auto exp = expected<ndc<int>, int>(std::in_place, 1);
    CHECK_EQ(exp.value(), ndc<int>(1));
  }
  {
    auto exp = expected<ndc<int>, int>(std::in_place, { 0, 1, 2 });
    CHECK_EQ(exp.value(), ndc<int>({ 0, 1, 2 }));
  }
  {
    auto exp = expected<int, ndc<int>>(unexpect, 1);
    CHECK_EQ(exp.error(), ndc<int>(1));
  }
  {
    auto exp = expected<int, ndc<int>>(unexpect, { 0, 1, 2 });
    CHECK_EQ(exp.error(), ndc<int>({ 0, 1, 2 }));
  }
}

TEST_CASE("copy/move assignment operator")
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
    auto s = expected<void, dc<int>>(std::in_place);
    auto d = expected<void, dc<int>>(std::in_place);
    d = s;
    CHECK(s);
    CHECK(d);
  }
  {
    auto s = expected<void, dc<int>>(unexpect, { 0, 1, 2 });
    auto d = expected<void, dc<int>>(std::in_place);
    d = s;
    CHECK_EQ(s.error(), dc<int>({ 0, 1, 2 }));
    CHECK_EQ(d.error(), dc<int>({ 0, 1, 2 }));
  }
  {
    auto s = expected<void, dc<int>>(std::in_place);
    auto d = expected<void, dc<int>>(unexpect, { 3, 4, 5 });
    d = s;
    CHECK(s);
    CHECK(d);
  }
  {
    auto s = expected<void, dc<int>>(unexpect, { 0, 1, 2 });
    auto d = expected<void, dc<int>>(unexpect, { 3, 4, 5 });
    d = s;
    CHECK_EQ(s.error(), dc<int>({ 0, 1, 2 }));
    CHECK_EQ(d.error(), dc<int>({ 0, 1, 2 }));
  }
  {
    auto s = expected<void, dc<int>>(std::in_place);
    auto d = expected<void, dc<int>>(std::in_place);
    d = std::move(s);
    CHECK(s);
    CHECK(d);
  }
  {
    auto s = expected<void, dc<int>>(unexpect, { 0, 1, 2 });
    auto d = expected<void, dc<int>>(std::in_place);
    d = std::move(s);
    CHECK_EQ(s.error(), dc<int>());
    CHECK_EQ(d.error(), dc<int>({ 0, 1, 2 }));
  }
  {
    auto s = expected<void, dc<int>>(std::in_place);
    auto d = expected<void, dc<int>>(unexpect, { 3, 4, 5 });
    d = std::move(s);
    CHECK(s);
    CHECK(d);
  }
  {
    auto s = expected<void, dc<int>>(unexpect, { 0, 1, 2 });
    auto d = expected<void, dc<int>>(unexpect, { 3, 4, 5 });
    d = std::move(s);
    CHECK_EQ(s.error(), dc<int>());
    CHECK_EQ(d.error(), dc<int>({ 0, 1, 2 }));
  }
  {
    auto s = expected<dc<int>, dc<int>>({ 0, 1, 2 });
    auto d = expected<dc<int>, dc<int>>({ 3, 4, 5 });
    d = s;
    CHECK_EQ(s.value(), dc<int>({ 0, 1, 2 }));
    CHECK_EQ(d.value(), dc<int>({ 0, 1, 2 }));
  }
  {
    auto s = expected<dc<int>, dc<int>>(unexpect, { 0, 1, 2 });
    auto d = expected<dc<int>, dc<int>>({ 3, 4, 5 });
    d = s;
    CHECK_EQ(s.error(), dc<int>({ 0, 1, 2 }));
    CHECK_EQ(d.error(), dc<int>({ 0, 1, 2 }));
  }
  {
    auto s = expected<dc<int>, dc<int>>({ 0, 1, 2 });
    auto d = expected<dc<int>, dc<int>>(unexpect, { 3, 4, 5 });
    d = s;
    CHECK_EQ(s.value(), dc<int>({ 0, 1, 2 }));
    CHECK_EQ(d.value(), dc<int>({ 0, 1, 2 }));
  }
  {
    auto s = expected<dc<int>, dc<int>>(unexpect, { 0, 1, 2 });
    auto d = expected<dc<int>, dc<int>>(unexpect, { 3, 4, 5 });
    d = s;
    CHECK_EQ(s.error(), dc<int>({ 0, 1, 2 }));
    CHECK_EQ(d.error(), dc<int>({ 0, 1, 2 }));
  }
  {
    auto s = expected<dc<int>, dc<int>>({ 0, 1, 2 });
    auto d = expected<dc<int>, dc<int>>({ 3, 4, 5 });
    d = std::move(s);
    CHECK_EQ(s.value(), dc<int>());
    CHECK_EQ(d.value(), dc<int>({ 0, 1, 2 }));
  }
  {
    auto s = expected<dc<int>, dc<int>>(unexpect, { 0, 1, 2 });
    auto d = expected<dc<int>, dc<int>>({ 3, 4, 5 });
    d = std::move(s);
    CHECK_EQ(s.error(), dc<int>());
    CHECK_EQ(d.error(), dc<int>({ 0, 1, 2 }));
  }
  {
    auto s = expected<dc<int>, dc<int>>({ 0, 1, 2 });
    auto d = expected<dc<int>, dc<int>>(unexpect, { 3, 4, 5 });
    d = std::move(s);
    CHECK_EQ(s.value(), dc<int>());
    CHECK_EQ(d.value(), dc<int>({ 0, 1, 2 }));
  }
  {
    auto s = expected<dc<int>, dc<int>>(unexpect, { 0, 1, 2 });
    auto d = expected<dc<int>, dc<int>>(unexpect, { 3, 4, 5 });
    d = std::move(s);
    CHECK_EQ(s.error(), dc<int>());
    CHECK_EQ(d.error(), dc<int>({ 0, 1, 2 }));
  }
  {
    auto val = dc<int>({ 0, 1, 2 });
    auto val2 = dc<int>({ 3, 4, 5 });
    auto s = expected<dc<int>&, dc<int>>(val);
    auto d = expected<dc<int>&, dc<int>>(val2);
    d = std::move(s);
    CHECK_EQ(s.value(), dc<int>({ 0, 1, 2 }));
    CHECK_EQ(d.value(), dc<int>({ 0, 1, 2 }));
  }
  {
    auto val2 = dc<int>({ 3, 4, 5 });
    auto s = expected<dc<int>&, dc<int>>(unexpect, { 0, 1, 2 });
    auto d = expected<dc<int>&, dc<int>>(val2);
    d = std::move(s);
    CHECK_EQ(s.error(), dc<int>());
    CHECK_EQ(d.error(), dc<int>({ 0, 1, 2 }));
  }
  {
    auto val = dc<int>({ 0, 1, 2 });
    auto s = expected<dc<int>&, dc<int>>(val);
    auto d = expected<dc<int>&, dc<int>>(unexpect, { 3, 4, 5 });
    d = std::move(s);
    CHECK_EQ(s.value(), dc<int>({ 0, 1, 2 }));
    CHECK_EQ(d.value(), dc<int>({ 0, 1, 2 }));
  }
  {
    auto s = expected<dc<int>&, dc<int>>(unexpect, { 0, 1, 2 });
    auto d = expected<dc<int>&, dc<int>>(unexpect, { 3, 4, 5 });
    d = std::move(s);
    CHECK_EQ(s.error(), dc<int>());
    CHECK_EQ(d.error(), dc<int>({ 0, 1, 2 }));
  }
}

TEST_CASE("perfect forwarding assignment operator")
{
  {
    auto exp = expected<dc<int>, int>();
    auto val = dc<int>({ 0, 1, 2 });
    exp = val;
    CHECK_EQ(exp.value(), dc<int>({ 0, 1, 2 }));
  }
  {
    auto exp = expected<dc<int>, int>(unexpect, 0);
    auto val = dc<int>({ 0, 1, 2 });
    exp = val;
    CHECK_EQ(exp.value(), dc<int>({ 0, 1, 2 }));
  }
  {
    auto exp = expected<dc<int>, int>();
    auto val = dc<int>({ 0, 1, 2 });
    exp = std::move(val);
    CHECK_EQ(exp.value(), dc<int>({ 0, 1, 2 }));
  }
  {
    auto exp = expected<dc<int>, int>(unexpect, 0);
    auto val = dc<int>({ 0, 1, 2 });
    exp = std::move(val);
    CHECK_EQ(exp.value(), dc<int>({ 0, 1, 2 }));
  }
  static_assert(!std::is_assignable<expected<int&, int>, int>::value);
  static_assert(std::is_assignable<expected<int&, int>, int&>::value);
  static_assert(!std::is_assignable<expected<int&, int>, int&&>::value);
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
  assert_is_same<memof_op, expected<void, int>&, void>();
  assert_is_same<memof_op, const expected<void, int>&, void>();
  assert_is_same<memof_op, expected<void, int>, void>();
  assert_is_same<memof_op, const expected<void, int>, void>();
  assert_is_same<memof_op, expected<int, int>&, int*>();
  assert_is_same<memof_op, const expected<int, int>&, const int*>();
  assert_is_same<memof_op, expected<int, int>, int*>();
  assert_is_same<memof_op, const expected<int, int>, const int*>();
  assert_is_same<memof_op, expected<int&, int>&, int*>();
  assert_is_same<memof_op, const expected<int&, int>&, int*>();
  assert_is_same<memof_op, expected<int&, int>, int*>();
  assert_is_same<memof_op, const expected<int&, int>, int*>();
  {
    auto exp = expected<int, int>(1);
    CHECK_EQ(*exp.operator->(), 1);
  }
  {
    const auto exp = expected<int, int>(1);
    CHECK_EQ(*exp.operator->(), 1);
  }
  {
    auto exp = expected<dc<int>, int>({ 0, 1, 2 });
    CHECK_EQ(exp->size(), 3);
  }
  {
    const auto exp = expected<dc<int>, int>({ 0, 1, 2 });
    CHECK_EQ(exp->size(), 3);
  }
  {
    auto val = dc<int>({ 0, 1, 2 });
    auto exp = expected<dc<int>&, int>(val);
    CHECK_EQ(exp->size(), 3);
  }
  {
    auto val = dc<int>({ 0, 1, 2 });
    const auto exp = expected<dc<int>&, int>(val);
    CHECK_EQ(exp->size(), 3);
  }
}

TEST_CASE("operator*")
{
  assert_is_same<deref_op, expected<void, int>&, void>();
  assert_is_same<deref_op, const expected<void, int>&, void>();
  assert_is_same<deref_op, expected<void, int>, void>();
  assert_is_same<deref_op, const expected<void, int>, void>();
  assert_is_same<deref_op, expected<int, int>&, int&>();
  assert_is_same<deref_op, const expected<int, int>&, const int&>();
  assert_is_same<deref_op, expected<int, int>, int&&>();
  assert_is_same<deref_op, const expected<int, int>, const int&&>();
  assert_is_same<deref_op, expected<int&, int>&, int&>();
  assert_is_same<deref_op, const expected<int&, int>&, int&>();
  assert_is_same<deref_op, expected<int&, int>, int&>();
  assert_is_same<deref_op, const expected<int&, int>, int&>();
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
    auto exp = expected<dc<int>, int>({ 0, 1, 2 });
    CHECK_EQ(*exp, dc<int>({ 0, 1, 2 }));
  }
  {
    const auto exp = expected<dc<int>, int>({ 0, 1, 2 });
    CHECK_EQ(*exp, dc<int>({ 0, 1, 2 }));
  }
  {
    auto exp = expected<dc<int>, int>({ 0, 1, 2 });
    CHECK_EQ(*std::move(exp), dc<int>({ 0, 1, 2 }));
  }
  {
    const auto exp = expected<dc<int>, int>({ 0, 1, 2 });
    CHECK_EQ(*std::move(exp), dc<int>({ 0, 1, 2 }));
  }
  {
    auto exp = expected<dc<int>, dc<int>>({ 0, 1, 2 });
    CHECK_EQ(*exp, dc<int>({ 0, 1, 2 }));
  }
  {
    const auto exp = expected<dc<int>, dc<int>>({ 0, 1, 2 });
    CHECK_EQ(*exp, dc<int>({ 0, 1, 2 }));
  }
  {
    auto exp = expected<dc<int>, dc<int>>({ 0, 1, 2 });
    CHECK_EQ(*std::move(exp), dc<int>({ 0, 1, 2 }));
  }
  {
    const auto exp = expected<dc<int>, dc<int>>({ 0, 1, 2 });
    CHECK_EQ(*std::move(exp), dc<int>({ 0, 1, 2 }));
  }
  {
    auto val = dc<int>({ 0, 1, 2 });
    auto exp = expected<dc<int>&, dc<int>>(val);
    CHECK_EQ(*exp, dc<int>({ 0, 1, 2 }));
  }
  {
    auto val = dc<int>({ 0, 1, 2 });
    const auto exp = expected<dc<int>&, dc<int>>(val);
    CHECK_EQ(*exp, dc<int>({ 0, 1, 2 }));
  }
  {
    auto val = dc<int>({ 0, 1, 2 });
    auto exp = expected<dc<int>&, dc<int>>(val);
    CHECK_EQ(*std::move(exp), dc<int>({ 0, 1, 2 }));
  }
  {
    auto val = dc<int>({ 0, 1, 2 });
    const auto exp = expected<dc<int>&, dc<int>>(val);
    CHECK_EQ(*std::move(exp), dc<int>({ 0, 1, 2 }));
  }
}

TEST_CASE("has_value")
{
  CHECK(expected<int, int>().has_value());
  CHECK(!expected<int, int>(unexpect, 1).has_value());
}

TEST_CASE("value")
{
  assert_is_same<fn_value, expected<void, int>&, void>();
  assert_is_same<fn_value, const expected<void, int>&, void>();
  assert_is_same<fn_value, expected<void, int>, void>();
  assert_is_same<fn_value, const expected<void, int>, void>();
  assert_is_same<fn_value, expected<int, int>&, int&>();
  assert_is_same<fn_value, const expected<int, int>&, const int&>();
  assert_is_same<fn_value, expected<int, int>, int&&>();
  assert_is_same<fn_value, const expected<int, int>, const int&&>();
  assert_is_same<fn_value, expected<int&, int>&, int&>();
  assert_is_same<fn_value, const expected<int&, int>&, int&>();
  assert_is_same<fn_value, expected<int&, int>, int&>();
  assert_is_same<fn_value, const expected<int&, int>, int&>();
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
    auto exp = expected<dc<int>, int>({ 0, 1, 2 });
    CHECK_EQ(exp.value(), dc<int>({ 0, 1, 2 }));
  }
  {
    const auto exp = expected<dc<int>, int>({ 0, 1, 2 });
    CHECK_EQ(exp.value(), dc<int>({ 0, 1, 2 }));
  }
  {
    auto exp = expected<dc<int>, int>({ 0, 1, 2 });
    CHECK_EQ(std::move(exp).value(), dc<int>({ 0, 1, 2 }));
  }
  {
    const auto exp = expected<dc<int>, int>({ 0, 1, 2 });
    CHECK_EQ(std::move(exp).value(), dc<int>({ 0, 1, 2 }));
  }
  {
    auto exp = expected<dc<int>, dc<int>>({ 0, 1, 2 });
    CHECK_EQ(exp.value(), dc<int>({ 0, 1, 2 }));
  }
  {
    const auto exp = expected<dc<int>, dc<int>>({ 0, 1, 2 });
    CHECK_EQ(exp.value(), dc<int>({ 0, 1, 2 }));
  }
  {
    auto exp = expected<dc<int>, dc<int>>({ 0, 1, 2 });
    CHECK_EQ(std::move(exp).value(), dc<int>({ 0, 1, 2 }));
  }
  {
    const auto exp = expected<dc<int>, dc<int>>({ 0, 1, 2 });
    CHECK_EQ(std::move(exp).value(), dc<int>({ 0, 1, 2 }));
  }
  {
    auto val = dc<int>({ 0, 1, 2 });
    auto exp = expected<dc<int>&, int>(val);
    CHECK_EQ(exp.value(), dc<int>({ 0, 1, 2 }));
  }
  {
    auto val = dc<int>({ 0, 1, 2 });
    const auto exp = expected<dc<int>&, int>(val);
    CHECK_EQ(exp.value(), dc<int>({ 0, 1, 2 }));
  }
  {
    auto val = dc<int>({ 0, 1, 2 });
    auto exp = expected<dc<int>&, int>(val);
    CHECK_EQ(std::move(exp).value(), dc<int>({ 0, 1, 2 }));
  }
  {
    auto val = dc<int>({ 0, 1, 2 });
    const auto exp = expected<dc<int>&, int>(val);
    CHECK_EQ(std::move(exp).value(), dc<int>({ 0, 1, 2 }));
  }
#if !FITORIA_NO_EXCEPTIONS
  {
    auto exp = expected<dc<int>&, int>(unexpect);
    CHECK_THROWS_AS(exp.value(), bad_expected_access<int>);
  }
  {
    const auto exp = expected<dc<int>&, int>(unexpect);
    CHECK_THROWS_AS(exp.value(), bad_expected_access<int>);
  }
  {
    auto exp = expected<dc<int>&, int>(unexpect);
    CHECK_THROWS_AS(std::move(exp).value(), bad_expected_access<int>);
  }
  {
    const auto exp = expected<dc<int>&, int>(unexpect);
    CHECK_THROWS_AS(std::move(exp).value(), bad_expected_access<int>);
  }
#endif
}

TEST_CASE("error")
{
  assert_is_same<fn_error, expected<void, int>&, int&>();
  assert_is_same<fn_error, const expected<void, int>&, const int&>();
  assert_is_same<fn_error, expected<void, int>, int&&>();
  assert_is_same<fn_error, const expected<void, int>, const int&&>();
  assert_is_same<fn_error, expected<int, int>&, int&>();
  assert_is_same<fn_error, const expected<int, int>&, const int&>();
  assert_is_same<fn_error, expected<int, int>, int&&>();
  assert_is_same<fn_error, const expected<int, int>, const int&&>();
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
    auto exp = expected<dc<int>, int>(unexpect, 1);
    CHECK_EQ(exp.error(), 1);
  }
  {
    const auto exp = expected<dc<int>, int>(unexpect, 1);
    CHECK_EQ(exp.error(), 1);
  }
  {
    auto exp = expected<dc<int>, int>(unexpect, 1);
    CHECK_EQ(std::move(exp).error(), 1);
  }
  {
    const auto exp = expected<dc<int>, int>(unexpect, 1);
    CHECK_EQ(std::move(exp).error(), 1);
  }
  {
    auto exp = expected<dc<int>, dc<int>>(unexpect, 1);
    CHECK_EQ(exp.error(), dc<int>(1));
  }
  {
    const auto exp = expected<dc<int>, dc<int>>(unexpect, 1);
    CHECK_EQ(exp.error(), dc<int>(1));
  }
  {
    auto exp = expected<dc<int>, dc<int>>(unexpect, 1);
    CHECK_EQ(std::move(exp).error(), dc<int>(1));
  }
  {
    const auto exp = expected<dc<int>, dc<int>>(unexpect, 1);
    CHECK_EQ(std::move(exp).error(), dc<int>(1));
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
  {
    int val = 1;
    auto exp = expected<int&, int>(val);
    CHECK_EQ(exp.value_or(2LL), 1);
  }
  {
    int val = 1;
    auto exp = expected<int&, int>(val);
    CHECK_EQ(std::move(exp).value_or(2LL), 1);
  }
  {
    auto exp = expected<int&, int>(unexpect, 1);
    CHECK_EQ(exp.value_or(2LL), 2);
  }
  {
    auto exp = expected<int&, int>(unexpect, 1);
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

TEST_CASE("emplace")
{
  static_assert(
      std::is_same_v<decltype(std::declval<expected<void, int>&>().emplace()),
                     void>);
  static_assert(
      std::is_same_v<decltype(std::declval<expected<int, int>&>().emplace(1)),
                     int&>);
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
  {
    auto e = expected<dc<int>, int>({ 0, 1, 2 });
    e.emplace(1);
    CHECK_EQ(e.value(), dc<int>(1));
  }
  {
    auto e = expected<dc<int>, int>(unexpect, 0);
    e.emplace(1);
    CHECK_EQ(e.value(), dc<int>(1));
  }
  {
    auto val = dc<int>({ 0, 1, 2 });
    auto e = expected<dc<int>&, int>(val);
    auto val2 = dc<int>({ 3, 4, 5 });
    e.emplace(val2);
    CHECK_EQ(val, dc<int>({ 0, 1, 2 }));
    CHECK_EQ(e.value(), dc<int>({ 3, 4, 5 }));
  }
  {
    auto e = expected<dc<int>&, int>(unexpect, 0);
    auto val2 = dc<int>({ 3, 4, 5 });
    e.emplace(val2);
    CHECK_EQ(e.value(), dc<int>({ 3, 4, 5 }));
  }
}

TEST_CASE("and_then")
{
  {
    int val = 0;
    auto f = [&]() -> expected<void, int> {
      val = 1;
      return expected<void, int>();
    };
    using ft = decltype(f);
    assert_is_same<fn_and_then<ft>::fn, expected<void, int>&,
                   expected<void, int>>();
    assert_is_same<fn_and_then<ft>::fn, const expected<void, int>&,
                   expected<void, int>>();
    assert_is_same<fn_and_then<ft>::fn, expected<void, int>,
                   expected<void, int>>();
    assert_is_same<fn_and_then<ft>::fn, const expected<void, int>,
                   expected<void, int>>();
    SUBCASE("")
    {
      auto e = expected<void, int>(unexpect, 1);
      CHECK_EQ(e.and_then(f).error(), 1);
      CHECK_EQ(val, 0);
    }
    SUBCASE("")
    {
      auto e = expected<void, int>();
      CHECK(e.and_then(f));
      CHECK_EQ(val, 1);
    }
    SUBCASE("")
    {
      const auto e = expected<void, int>(unexpect, 1);
      CHECK_EQ(e.and_then(f).error(), 1);
      CHECK_EQ(val, 0);
    }
    SUBCASE("")
    {
      const auto e = expected<void, int>();
      CHECK(e.and_then(f));
      CHECK_EQ(val, 1);
    }
    SUBCASE("")
    {
      auto e = expected<void, int>(unexpect, 1);
      CHECK_EQ(std::move(e).and_then(f).error(), 1);
      CHECK_EQ(val, 0);
    }
    SUBCASE("")
    {
      auto e = expected<void, int>();
      CHECK(std::move(e).and_then(f));
      CHECK_EQ(val, 1);
    }
    SUBCASE("")
    {
      const auto e = expected<void, int>(unexpect, 1);
      CHECK_EQ(std::move(e).and_then(f).error(), 1);
      CHECK_EQ(val, 0);
    }
    SUBCASE("")
    {
      const auto e = expected<void, int>();
      CHECK(std::move(e).and_then(f));
      CHECK_EQ(val, 1);
    }
  }
  {
    auto f
        = [](int i) -> expected<std::string, int> { return std::to_string(i); };
    using ft = decltype(f);
    assert_is_same<fn_and_then<ft>::fn, expected<int, int>&,
                   expected<std::string, int>>();
    assert_is_same<fn_and_then<ft>::fn, const expected<int, int>&,
                   expected<std::string, int>>();
    assert_is_same<fn_and_then<ft>::fn, expected<int, int>,
                   expected<std::string, int>>();
    assert_is_same<fn_and_then<ft>::fn, const expected<int, int>,
                   expected<std::string, int>>();
    {
      auto e = expected<int, int>(unexpect, 1);
      CHECK_EQ(e.and_then(f).error(), 1);
    }
    {
      auto e = expected<int, int>();
      CHECK_EQ(e.and_then(f).value(), "0");
    }
    {
      const auto e = expected<int, int>(unexpect, 1);
      CHECK_EQ(e.and_then(f).error(), 1);
    }
    {
      const auto e = expected<int, int>();
      CHECK_EQ(e.and_then(f).value(), "0");
    }
    {
      auto e = expected<int, int>(unexpect, 1);
      CHECK_EQ(std::move(e).and_then(f).error(), 1);
    }
    {
      auto e = expected<int, int>();
      CHECK_EQ(std::move(e).and_then(f).value(), "0");
    }
    {
      const auto e = expected<int, int>(unexpect, 1);
      CHECK_EQ(std::move(e).and_then(f).error(), 1);
    }
    {
      const auto e = expected<int, int>();
      CHECK_EQ(std::move(e).and_then(f).value(), "0");
    }
  }
  {
    int val = 0;
    auto f
        = [](int& i) -> expected<int&, int> { return expected<int&, int>(i); };
    using ft = decltype(f);
    assert_is_same<fn_and_then<ft>::fn, expected<int&, int>&,
                   expected<int&, int>>();
    assert_is_same<fn_and_then<ft>::fn, const expected<int&, int>&,
                   expected<int&, int>>();
    assert_is_same<fn_and_then<ft>::fn, expected<int&, int>,
                   expected<int&, int>>();
    assert_is_same<fn_and_then<ft>::fn, const expected<int&, int>,
                   expected<int&, int>>();
    SUBCASE("")
    {
      auto e = expected<int&, int>(unexpect, 1);
      CHECK_EQ(e.and_then(f).error(), 1);
      CHECK_EQ(val, 0);
    }
    SUBCASE("")
    {
      auto e = expected<int&, int>(val);
      CHECK_EQ(++e.and_then(f).value(), 1);
      CHECK_EQ(val, 1);
    }
    SUBCASE("")
    {
      const auto e = expected<int&, int>(unexpect, 1);
      CHECK_EQ(e.and_then(f).error(), 1);
      CHECK_EQ(val, 0);
    }
    SUBCASE("")
    {
      const auto e = expected<int&, int>(val);
      CHECK_EQ(++e.and_then(f).value(), 1);
      CHECK_EQ(val, 1);
    }
    SUBCASE("")
    {
      auto e = expected<int&, int>(unexpect, 1);
      CHECK_EQ(std::move(e).and_then(f).error(), 1);
      CHECK_EQ(val, 0);
    }
    SUBCASE("")
    {
      auto e = expected<int&, int>(val);
      CHECK_EQ(++std::move(e).and_then(f).value(), 1);
      CHECK_EQ(val, 1);
    }
    SUBCASE("")
    {
      const auto e = expected<int&, int>(unexpect, 1);
      CHECK_EQ(std::move(e).and_then(f).error(), 1);
      CHECK_EQ(val, 0);
    }
    SUBCASE("")
    {
      const auto e = expected<int&, int>(val);
      CHECK_EQ(++std::move(e).and_then(f).value(), 1);
      CHECK_EQ(val, 1);
    }
  }
}

TEST_CASE("or_else")
{
  {
    auto f = [](int i) -> expected<void, int> {
      return expected<void, int>(unexpect, i + 1);
    };
    using ft = decltype(f);
    assert_is_same<fn_or_else<ft>::fn, expected<void, int>&,
                   expected<void, int>>();
    assert_is_same<fn_or_else<ft>::fn, const expected<void, int>&,
                   expected<void, int>>();
    assert_is_same<fn_or_else<ft>::fn, expected<void, int>,
                   expected<void, int>>();
    assert_is_same<fn_or_else<ft>::fn, const expected<void, int>,
                   expected<void, int>>();
    {
      auto e = expected<void, int>(unexpect, 1);
      CHECK_EQ(e.or_else(f).error(), 2);
    }
    {
      auto e = expected<void, int>();
      CHECK(e.or_else(f));
    }
    {
      const auto e = expected<void, int>(unexpect, 1);
      CHECK_EQ(e.or_else(f).error(), 2);
    }
    {
      const auto e = expected<void, int>();
      CHECK(e.or_else(f));
    }
    {
      auto e = expected<void, int>(unexpect, 1);
      CHECK_EQ(std::move(e).or_else(f).error(), 2);
    }
    {
      auto e = expected<void, int>();
      CHECK(std::move(e).or_else(f));
    }
    {
      const auto e = expected<void, int>(unexpect, 1);
      CHECK_EQ(std::move(e).or_else(f).error(), 2);
    }
    {
      const auto e = expected<void, int>();
      CHECK(std::move(e).or_else(f));
    }
  }
  {
    auto f = [](int i) -> expected<int, std::string> {
      return expected<int, std::string>(unexpect, std::to_string(i));
    };
    using ft = decltype(f);
    assert_is_same<fn_or_else<ft>::fn, expected<int, int>&,
                   expected<int, std::string>>();
    assert_is_same<fn_or_else<ft>::fn, const expected<int, int>&,
                   expected<int, std::string>>();
    assert_is_same<fn_or_else<ft>::fn, expected<int, int>,
                   expected<int, std::string>>();
    assert_is_same<fn_or_else<ft>::fn, const expected<int, int>,
                   expected<int, std::string>>();
    {
      auto e = expected<int, int>(unexpect, 1);
      CHECK_EQ(e.or_else(f).error(), "1");
    }
    {
      auto e = expected<int, int>();
      CHECK_EQ(e.or_else(f), 0);
    }
    {
      const auto e = expected<int, int>(unexpect, 1);
      CHECK_EQ(e.or_else(f).error(), "1");
    }
    {
      const auto e = expected<int, int>();
      CHECK_EQ(e.or_else(f), 0);
    }
    {
      auto e = expected<int, int>(unexpect, 1);
      CHECK_EQ(std::move(e).or_else(f).error(), "1");
    }
    {
      auto e = expected<int, int>();
      CHECK_EQ(std::move(e).or_else(f), 0);
    }
    {
      const auto e = expected<int, int>(unexpect, 1);
      CHECK_EQ(std::move(e).or_else(f).error(), "1");
    }
    {
      const auto e = expected<int, int>();
      CHECK_EQ(std::move(e).or_else(f), 0);
    }
  }
  {
    int val = 0;
    int val2 = 10;
    auto f
        = [&](int) -> expected<int&, int> { return expected<int&, int>(val2); };
    using ft = decltype(f);
    assert_is_same<fn_or_else<ft>::fn, expected<int&, int>&,
                   expected<int&, int>>();
    assert_is_same<fn_or_else<ft>::fn, const expected<int&, int>&,
                   expected<int&, int>>();
    assert_is_same<fn_or_else<ft>::fn, expected<int&, int>,
                   expected<int&, int>>();
    assert_is_same<fn_or_else<ft>::fn, const expected<int&, int>,
                   expected<int&, int>>();
    SUBCASE("")
    {
      auto e = expected<int&, int>(unexpect, 1);
      CHECK_EQ(++e.or_else(f).value(), 11);
      CHECK_EQ(val, 0);
      CHECK_EQ(val2, 11);
    }
    SUBCASE("")
    {
      auto e = expected<int&, int>(val);
      CHECK_EQ(++e.or_else(f).value(), 1);
      CHECK_EQ(val, 1);
      CHECK_EQ(val2, 10);
    }
    SUBCASE("")
    {
      const auto e = expected<int&, int>(unexpect, 1);
      CHECK_EQ(++e.or_else(f).value(), 11);
      CHECK_EQ(val, 0);
      CHECK_EQ(val2, 11);
    }
    SUBCASE("")
    {
      const auto e = expected<int&, int>(val);
      CHECK_EQ(++e.or_else(f).value(), 1);
      CHECK_EQ(val, 1);
      CHECK_EQ(val2, 10);
    }
    SUBCASE("")
    {
      auto e = expected<int&, int>(unexpect, 1);
      CHECK_EQ(++std::move(e).or_else(f).value(), 11);
      CHECK_EQ(val, 0);
      CHECK_EQ(val2, 11);
    }
    SUBCASE("")
    {
      auto e = expected<int&, int>(val);
      CHECK_EQ(++std::move(e).or_else(f).value(), 1);
      CHECK_EQ(val, 1);
      CHECK_EQ(val2, 10);
    }
    SUBCASE("")
    {
      const auto e = expected<int&, int>(unexpect, 1);
      CHECK_EQ(++std::move(e).or_else(f).value(), 11);
      CHECK_EQ(val, 0);
      CHECK_EQ(val2, 11);
    }
    SUBCASE("")
    {
      const auto e = expected<int&, int>(val);
      CHECK_EQ(++std::move(e).or_else(f).value(), 1);
      CHECK_EQ(val, 1);
      CHECK_EQ(val2, 10);
    }
  }
}

TEST_CASE("transform")
{
  {
    int val = 0;
    auto f = [&]() -> int { return ++val; };
    using ft = decltype(f);
    assert_is_same<fn_transform<ft>::fn, expected<void, int>&,
                   expected<int, int>>();
    assert_is_same<fn_transform<ft>::fn, const expected<void, int>&,
                   expected<int, int>>();
    assert_is_same<fn_transform<ft>::fn, expected<void, int>,
                   expected<int, int>>();
    assert_is_same<fn_transform<ft>::fn, const expected<void, int>,
                   expected<int, int>>();
    SUBCASE("")
    {
      auto e = expected<void, int>(unexpect, 1);
      CHECK_EQ(e.transform(f).error(), 1);
      CHECK_EQ(val, 0);
    }
    SUBCASE("")
    {
      auto e = expected<void, int>();
      CHECK_EQ(e.transform(f).value(), 1);
      CHECK_EQ(val, 1);
    }
    SUBCASE("")
    {
      const auto e = expected<void, int>(unexpect, 1);
      CHECK_EQ(e.transform(f).error(), 1);
      CHECK_EQ(val, 0);
    }
    SUBCASE("")
    {
      const auto e = expected<void, int>();
      CHECK_EQ(e.transform(f).value(), 1);
      CHECK_EQ(val, 1);
    }
    SUBCASE("")
    {
      auto e = expected<void, int>(unexpect, 1);
      CHECK_EQ(std::move(e).transform(f).error(), 1);
      CHECK_EQ(val, 0);
    }
    SUBCASE("")
    {
      auto e = expected<void, int>();
      CHECK_EQ(std::move(e).transform(f).value(), 1);
      CHECK_EQ(val, 1);
    }
    SUBCASE("")
    {
      const auto e = expected<void, int>(unexpect, 1);
      CHECK_EQ(std::move(e).transform(f).error(), 1);
      CHECK_EQ(val, 0);
    }
    SUBCASE("")
    {
      const auto e = expected<void, int>();
      CHECK_EQ(std::move(e).transform(f).value(), 1);
      CHECK_EQ(val, 1);
    }
  }
  {
    auto f = [](int i) -> std::string { return std::to_string(i); };
    using ft = decltype(f);
    assert_is_same<fn_transform<ft>::fn, expected<int, int>&,
                   expected<std::string, int>>();
    assert_is_same<fn_transform<ft>::fn, const expected<int, int>&,
                   expected<std::string, int>>();
    assert_is_same<fn_transform<ft>::fn, expected<int, int>,
                   expected<std::string, int>>();
    assert_is_same<fn_transform<ft>::fn, const expected<int, int>,
                   expected<std::string, int>>();
    {
      auto e = expected<int, int>(unexpect, 1);
      CHECK_EQ(e.transform(f).error(), 1);
    }
    {
      auto e = expected<int, int>();
      CHECK_EQ(e.transform(f).value(), "0");
    }
    {
      const auto e = expected<int, int>(unexpect, 1);
      CHECK_EQ(e.transform(f).error(), 1);
    }
    {
      const auto e = expected<int, int>();
      CHECK_EQ(e.transform(f).value(), "0");
    }
    {
      auto e = expected<int, int>(unexpect, 1);
      CHECK_EQ(std::move(e).transform(f).error(), 1);
    }
    {
      auto e = expected<int, int>();
      CHECK_EQ(std::move(e).transform(f).value(), "0");
    }
    {
      const auto e = expected<int, int>(unexpect, 1);
      CHECK_EQ(std::move(e).transform(f).error(), 1);
    }
    {
      const auto e = expected<int, int>();
      CHECK_EQ(std::move(e).transform(f).value(), "0");
    }
  }
  {
    int val = 0;
    auto f = [](int& i) -> int& { return i; };
    using ft = decltype(f);
    assert_is_same<fn_transform<ft>::fn, expected<int&, int>&,
                   expected<int&, int>>();
    assert_is_same<fn_transform<ft>::fn, const expected<int&, int>&,
                   expected<int&, int>>();
    assert_is_same<fn_transform<ft>::fn, expected<int&, int>,
                   expected<int&, int>>();
    assert_is_same<fn_transform<ft>::fn, const expected<int&, int>,
                   expected<int&, int>>();
    SUBCASE("")
    {
      auto e = expected<int&, int>(unexpect, 1);
      CHECK_EQ(e.transform(f).error(), 1);
      CHECK_EQ(val, 0);
    }
    SUBCASE("")
    {
      auto e = expected<int&, int>(val);
      CHECK_EQ(++e.transform(f).value(), 1);
      CHECK_EQ(val, 1);
    }
    SUBCASE("")
    {
      const auto e = expected<int&, int>(unexpect, 1);
      CHECK_EQ(e.transform(f).error(), 1);
      CHECK_EQ(val, 0);
    }
    SUBCASE("")
    {
      const auto e = expected<int&, int>(val);
      CHECK_EQ(++e.transform(f).value(), 1);
      CHECK_EQ(val, 1);
    }
    SUBCASE("")
    {
      auto e = expected<int&, int>(unexpect, 1);
      CHECK_EQ(std::move(e).transform(f).error(), 1);
      CHECK_EQ(val, 0);
    }
    SUBCASE("")
    {
      auto e = expected<int&, int>(val);
      CHECK_EQ(++std::move(e).transform(f).value(), 1);
      CHECK_EQ(val, 1);
    }
    SUBCASE("")
    {
      const auto e = expected<int&, int>(unexpect, 1);
      CHECK_EQ(std::move(e).transform(f).error(), 1);
      CHECK_EQ(val, 0);
    }
    SUBCASE("")
    {
      const auto e = expected<int&, int>(val);
      CHECK_EQ(++std::move(e).transform(f).value(), 1);
      CHECK_EQ(val, 1);
    }
  }
}

TEST_CASE("transform_error")
{
  {
    auto f = [](int i) -> std::string { return std::to_string(i); };
    using ft = decltype(f);
    assert_is_same<fn_transform_error<ft>::fn, expected<void, int>&,
                   expected<void, std::string>>();
    assert_is_same<fn_transform_error<ft>::fn, const expected<void, int>&,
                   expected<void, std::string>>();
    assert_is_same<fn_transform_error<ft>::fn, expected<void, int>,
                   expected<void, std::string>>();
    assert_is_same<fn_transform_error<ft>::fn, const expected<void, int>,
                   expected<void, std::string>>();
    {
      auto e = expected<void, int>(unexpect, 1);
      CHECK_EQ(e.transform_error(f).error(), "1");
    }
    {
      auto e = expected<void, int>();
      CHECK(e.transform_error(f));
    }
    {
      const auto e = expected<void, int>(unexpect, 1);
      CHECK_EQ(e.transform_error(f).error(), "1");
    }
    {
      const auto e = expected<void, int>();
      CHECK(e.transform_error(f));
    }
    {
      auto e = expected<void, int>(unexpect, 1);
      CHECK_EQ(std::move(e).transform_error(f).error(), "1");
    }
    {
      auto e = expected<void, int>();
      CHECK(std::move(e).transform_error(f));
    }
    {
      const auto e = expected<void, int>(unexpect, 1);
      CHECK_EQ(std::move(e).transform_error(f).error(), "1");
    }
    {
      const auto e = expected<void, int>();
      CHECK(std::move(e).transform_error(f));
    }
  }
  {
    int val = 0;
    auto f = [&](int i) -> std::string { return std::to_string(i); };
    using ft = decltype(f);
    assert_is_same<fn_transform_error<ft>::fn, expected<int&, int>&,
                   expected<int&, std::string>>();
    assert_is_same<fn_transform_error<ft>::fn, const expected<int&, int>&,
                   expected<int&, std::string>>();
    assert_is_same<fn_transform_error<ft>::fn, expected<int&, int>,
                   expected<int&, std::string>>();
    assert_is_same<fn_transform_error<ft>::fn, const expected<int&, int>,
                   expected<int&, std::string>>();
    SUBCASE("")
    {
      auto e = expected<int&, int>(unexpect, 1);
      CHECK_EQ(e.transform_error(f).error(), "1");
      CHECK_EQ(val, 0);
    }
    SUBCASE("")
    {
      auto e = expected<int&, int>(val);
      CHECK_EQ(++e.transform_error(f).value(), 1);
      CHECK_EQ(val, 1);
    }
    SUBCASE("")
    {
      const auto e = expected<int&, int>(unexpect, 1);
      CHECK_EQ(e.transform_error(f).error(), "1");
      CHECK_EQ(val, 0);
    }
    SUBCASE("")
    {
      const auto e = expected<int&, int>(val);
      CHECK_EQ(++e.transform_error(f).value(), 1);
      CHECK_EQ(val, 1);
    }
    SUBCASE("")
    {
      auto e = expected<int&, int>(unexpect, 1);
      CHECK_EQ(std::move(e).transform_error(f).error(), "1");
      CHECK_EQ(val, 0);
    }
    SUBCASE("")
    {
      auto e = expected<int&, int>(val);
      CHECK_EQ(++std::move(e).transform_error(f).value(), 1);
      CHECK_EQ(val, 1);
    }
    SUBCASE("")
    {
      const auto e = expected<int&, int>(unexpect, 1);
      CHECK_EQ(std::move(e).transform_error(f).error(), "1");
      CHECK_EQ(val, 0);
    }
    SUBCASE("")
    {
      const auto e = expected<int&, int>(val);
      CHECK_EQ(++std::move(e).transform_error(f).value(), 1);
      CHECK_EQ(val, 1);
    }
  }
}

TEST_CASE("value_to_optional")
{
  {
    auto e = expected<void, int>(unexpect);
    CHECK_EQ(e.value_to_optional(), optional<void>());
  }
  {
    auto e = expected<void, int>();
    CHECK_EQ(e.value_to_optional(), optional<void>(std::in_place));
  }
  {
    const auto e = expected<void, int>(unexpect);
    CHECK_EQ(e.value_to_optional(), optional<void>());
  }
  {
    const auto e = expected<void, int>();
    CHECK_EQ(e.value_to_optional(), optional<void>(std::in_place));
  }
  {
    auto e = expected<void, int>(unexpect);
    CHECK_EQ(std::move(e).value_to_optional(), optional<void>());
  }
  {
    auto e = expected<void, int>();
    CHECK_EQ(std::move(e).value_to_optional(), optional<void>(std::in_place));
  }
  {
    const auto e = expected<void, int>(unexpect);
    CHECK_EQ(std::move(e).value_to_optional(), optional<void>());
  }
  {
    const auto e = expected<void, int>();
    CHECK_EQ(std::move(e).value_to_optional(), optional<void>(std::in_place));
  }
  {
    auto e = expected<int, int>(unexpect);
    CHECK_EQ(e.value_to_optional(), optional<int>());
  }
  {
    auto e = expected<int, int>(1);
    CHECK_EQ(e.value_to_optional(), optional<int>(1));
  }
  {
    const auto e = expected<int, int>(unexpect);
    CHECK_EQ(e.value_to_optional(), optional<int>());
  }
  {
    const auto e = expected<int, int>(1);
    CHECK_EQ(e.value_to_optional(), optional<int>(1));
  }
  {
    auto e = expected<int, int>(unexpect);
    CHECK_EQ(std::move(e).value_to_optional(), optional<int>());
  }
  {
    auto e = expected<int, int>(1);
    CHECK_EQ(std::move(e).value_to_optional(), optional<int>(1));
  }
  {
    const auto e = expected<int, int>(unexpect);
    CHECK_EQ(std::move(e).value_to_optional(), optional<int>());
  }
  {
    const auto e = expected<int, int>(1);
    CHECK_EQ(std::move(e).value_to_optional(), optional<int>(1));
  }
}

TEST_CASE("error_to_optional")
{
  {
    auto e = expected<void, int>(unexpect, 1);
    CHECK_EQ(e.error_to_optional(), optional<int>(1));
  }
  {
    auto e = expected<void, int>();
    CHECK_EQ(e.error_to_optional(), optional<int>());
  }
  {
    const auto e = expected<void, int>(unexpect, 1);
    CHECK_EQ(e.error_to_optional(), optional<int>(1));
  }
  {
    const auto e = expected<void, int>();
    CHECK_EQ(e.error_to_optional(), optional<int>());
  }
  {
    auto e = expected<void, int>(unexpect, 1);
    CHECK_EQ(std::move(e).error_to_optional(), optional<int>(1));
  }
  {
    auto e = expected<void, int>();
    CHECK_EQ(std::move(e).error_to_optional(), optional<int>());
  }
  {
    const auto e = expected<void, int>(unexpect, 1);
    CHECK_EQ(std::move(e).error_to_optional(), optional<int>(1));
  }
  {
    const auto e = expected<void, int>();
    CHECK_EQ(std::move(e).error_to_optional(), optional<int>());
  }
  {
    auto e = expected<int, int>(unexpect);
    CHECK_EQ(e.error_to_optional(), optional<int>(0));
  }
  {
    auto e = expected<int, int>(1);
    CHECK_EQ(e.error_to_optional(), optional<int>());
  }
  {
    const auto e = expected<int, int>(unexpect);
    CHECK_EQ(e.error_to_optional(), optional<int>(0));
  }
  {
    const auto e = expected<int, int>(1);
    CHECK_EQ(e.error_to_optional(), optional<int>());
  }
  {
    auto e = expected<int, int>(unexpect);
    CHECK_EQ(std::move(e).error_to_optional(), optional<int>(0));
  }
  {
    auto e = expected<int, int>(1);
    CHECK_EQ(std::move(e).error_to_optional(), optional<int>());
  }
  {
    const auto e = expected<int, int>(unexpect);
    CHECK_EQ(std::move(e).error_to_optional(), optional<int>(0));
  }
  {
    const auto e = expected<int, int>(1);
    CHECK_EQ(std::move(e).error_to_optional(), optional<int>());
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
  {
    auto a = expected<ndc<int>, ndc<int>>(unexpect, { 7, 8, 9 });
    auto b = expected<ndc<int>, ndc<int>>(unexpect, 0);
    swap(a, b);
    CHECK_EQ(a.error(), ndc<int>(0));
    CHECK_EQ(b.error(), ndc<int>({ 7, 8, 9 }));
  }
  {
    auto a = expected<ndc<int>, ndc<int>>(unexpect, { 7, 8, 9 });
    auto b = expected<ndc<int>, ndc<int>>(1);
    swap(a, b);
    CHECK_EQ(a.value(), ndc<int>(1));
    CHECK_EQ(b.error(), ndc<int>({ 7, 8, 9 }));
  }
  {
    auto a = expected<ndc<int>, ndc<int>>(std::in_place, { 0, 1, 2 });
    auto b = expected<ndc<int>, ndc<int>>(unexpect, 0);
    swap(a, b);
    CHECK_EQ(a.error(), ndc<int>(0));
    CHECK_EQ(b.value(), ndc<int>({ 0, 1, 2 }));
  }
  {
    auto a = expected<ndc<int>, ndc<int>>(std::in_place, { 0, 1, 2 });
    auto b = expected<ndc<int>, ndc<int>>(1);
    swap(a, b);
    CHECK_EQ(a.value(), ndc<int>(1));
    CHECK_EQ(b.value(), ndc<int>({ 0, 1, 2 }));
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
  static_assert(!std::is_nothrow_move_constructible<move_throw>::value);
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
  CHECK(expected<void, int>() == expected<void, int>());
  CHECK(!(expected<void, int>() != expected<void, int>()));
  CHECK(expected<void, int>(unexpect) == expected<void, int>(unexpect));
  CHECK(!(expected<void, int>(unexpect) != expected<void, int>(unexpect)));
  CHECK(!(expected<void, int>() == expected<void, int>(unexpect)));
  CHECK(expected<void, int>() != expected<void, int>(unexpect));
  CHECK(!(expected<void, int>(unexpect) == expected<void, int>()));
  CHECK(expected<void, int>(unexpect) != expected<void, int>());

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
  {
    int val = 0;
    CHECK_EQ(fmt::format("{}", expected<const int&, int>(val)), "0");
    CHECK_EQ(fmt::format("{}", expected<const int&, int>(unexpect)),
             "{unexpected}");
  }
}

TEST_SUITE_END();
