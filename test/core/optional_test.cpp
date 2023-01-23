//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria_test.h>

#include <fitoria/core/optional.hpp>
#include <fitoria/core/optional_ext.hpp>

using namespace fitoria;

TEST_SUITE_BEGIN("core.optional");

namespace {
template <typename T>
class dc : public std::vector<T> {
public:
  using std::vector<T>::vector;

  dc() = default;

  dc(T val)
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

template <template <typename...> class Op, typename Test, typename Expected>
struct assert_is_same {
  static_assert(std::is_same<Op<Test>, Expected>::value);
};

}

TEST_CASE("type assertion")
{
  {
    using type = optional<void>;
    static_assert(std::is_trivially_copy_constructible_v<type>);
    static_assert(std::is_trivially_copy_assignable_v<type>);
    static_assert(std::is_trivially_move_constructible_v<type>);
    static_assert(std::is_trivially_move_assignable_v<type>);
    static_assert(std::is_trivially_destructible_v<type>);
  }
  {
    using type = optional<int>;
    static_assert(std::is_trivially_copy_constructible_v<type>);
    static_assert(std::is_trivially_copy_assignable_v<type>);
    static_assert(std::is_trivially_move_constructible_v<type>);
    static_assert(std::is_trivially_move_assignable_v<type>);
    static_assert(std::is_trivially_destructible_v<type>);
  }
  {
    using type = optional<dc<int>>;
    static_assert(!std::is_trivially_copy_constructible_v<type>);
    static_assert(!std::is_trivially_copy_assignable_v<type>);
    static_assert(!std::is_trivially_move_constructible_v<type>);
    static_assert(!std::is_trivially_move_assignable_v<type>);
    static_assert(!std::is_trivially_destructible_v<type>);
  }
  {
    using type = optional<int&>;
    static_assert(std::is_trivially_copy_constructible_v<type>);
    static_assert(std::is_trivially_copy_assignable_v<type>);
    static_assert(std::is_trivially_move_constructible_v<type>);
    static_assert(std::is_trivially_move_assignable_v<type>);
    static_assert(std::is_trivially_destructible_v<type>);
  }
}

TEST_CASE("has_value")
{
  CHECK(!optional<void>().has_value());
  CHECK(optional<void>(std::in_place).has_value());
  CHECK(!optional<int>().has_value());
  CHECK(optional<int>(1).has_value());
  int val = 1;
  CHECK(!optional<int&>().has_value());
  CHECK(optional<int&>(val).has_value());
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
  {
    int val = 1;
    auto o = optional<int&>(val);
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
  {
    auto o = optional<int>(std::in_place, 1);
    CHECK_EQ(o.value(), 1);
  }
  {
    auto o = optional<dc<int>>(std::in_place, { 0, 1, 2 });
    CHECK_EQ(o.value(), dc<int> { 0, 1, 2 });
  }
  {
    int val = 1;
    auto o = optional<int&>(std::in_place, val);
    CHECK_EQ(o.value(), 1);
  }
}

TEST_CASE("perfect forwarding constructor")
{
  {
    auto o = optional<dc<int>>(dc<int>(1));
    CHECK_EQ(o.value(), dc<int>(1));
  }
  {
    auto val = dc<int>(1);
    auto o = optional<dc<int>&>(val);
    CHECK_EQ(o.value(), dc<int>(1));
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
  {
    auto s = optional<dc<int>>();
    auto d = optional<dc<int>>(s);
    CHECK(!s);
    CHECK(!d);
  }
  {
    auto s = optional<dc<int>>(1);
    auto d = optional<dc<int>>(s);
    CHECK_EQ(s, dc<int>(1));
    CHECK_EQ(d, dc<int>(1));
  }
  {
    auto s = optional<dc<int>&>();
    auto d = optional<dc<int>&>(s);
    CHECK(!s);
    CHECK(!d);
  }
  {
    auto val = dc<int>(1);
    auto s = optional<dc<int>&>(val);
    auto d = optional<dc<int>&>(s);
    CHECK_EQ(s, dc<int>(1));
    CHECK_EQ(d, dc<int>(1));
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
  {
    auto s = optional<dc<int>>();
    auto d = optional<dc<int>>(std::move(s));
    CHECK(!s);
    CHECK(!d);
  }
  {
    auto s = optional<dc<int>>(1);
    auto d = optional<dc<int>>(std::move(s));
    CHECK_EQ(s, dc<int>());
    CHECK_EQ(d, dc<int>(1));
  }
  {
    auto s = optional<dc<int>&>();
    auto d = optional<dc<int>&>(std::move(s));
    CHECK(!s);
    CHECK(!d);
  }
  {
    auto val = dc<int>(1);
    auto s = optional<dc<int>&>(val);
    auto d = optional<dc<int>&>(std::move(s));
    CHECK_EQ(s, dc<int>(1));
    CHECK_EQ(d, dc<int>(1));
  }
}

TEST_CASE("converting constructor")
{
  {
    auto s = optional<dc<int>>();
    auto d = optional<std::vector<int>>(s);
    CHECK(!s);
    CHECK(!d);
  }
  {
    auto s = optional<dc<int>>(std::in_place, { 0, 1, 2 });
    auto d = optional<std::vector<int>>(s);
    CHECK(s);
    CHECK_EQ(d.value(), dc<int> { 0, 1, 2 });
  }
  {
    auto s = optional<dc<int>>();
    auto d = optional<std::vector<int>>(std::move(s));
    CHECK(!s);
    CHECK(!d);
  }
  {
    auto s = optional<dc<int>>(std::in_place, { 0, 1, 2 });
    auto d = optional<std::vector<int>>(std::move(s));
    CHECK(s);
    CHECK_EQ(d.value(), dc<int> { 0, 1, 2 });
  }
  {
    auto s = optional<dc<int>&>();
    auto d = optional<std::vector<int>&>(s);
    CHECK(!s);
    CHECK(!d);
  }
  {
    auto val = dc<int>({ 0, 1, 2 });
    auto s = optional<dc<int>&>(val);
    auto d = optional<std::vector<int>&>(s);
    CHECK(s);
    CHECK_EQ(d.value(), dc<int> { 0, 1, 2 });
  }
  {
    auto s = optional<dc<int>&>();
    auto d = optional<std::vector<int>&>(std::move(s));
    CHECK(!s);
    CHECK(!d);
  }
  {
    auto val = dc<int>({ 0, 1, 2 });
    auto s = optional<dc<int>&>(val);
    auto d = optional<std::vector<int>&>(std::move(s));
    CHECK(s);
    CHECK_EQ(d.value(), dc<int> { 0, 1, 2 });
  }
}

TEST_CASE("perfect forwarding assignment operator")
{
  {
    auto o = optional<dc<int>>();
    CHECK(!o);
    o = dc<int>(1);
    CHECK_EQ(o.value(), dc<int>(1));
  }
  {
    auto o = optional<dc<int>>(1);
    CHECK(o);
    o = dc<int>(1);
    CHECK_EQ(o.value(), dc<int>(1));
  }
  {
    auto o = optional<dc<int>&>();
    CHECK(!o);
    auto val = dc<int>(1);
    o = val;
    CHECK_EQ(o.value(), dc<int>(1));
    auto val2 = dc<int>({ 0, 1, 2 });
    o = val2;
    CHECK_EQ(o.value(), dc<int>({ 0, 1, 2 }));
    CHECK_NE(val, val2);
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
  {
    auto s = optional<dc<int>>();
    auto d = optional<dc<int>>();
    d = s;
    CHECK(!s);
    CHECK(!d);
  }
  {
    auto s = optional<dc<int>>(1);
    auto d = optional<dc<int>>();
    d = s;
    CHECK_EQ(s, dc<int>(1));
    CHECK_EQ(d, dc<int>(1));
  }
  {
    auto s = optional<dc<int>>();
    auto d = optional<dc<int>>(1);
    d = s;
    CHECK(!s);
    CHECK(!d);
  }
  {
    auto s = optional<dc<int>>(1);
    auto d = optional<dc<int>>(1);
    d = s;
    CHECK_EQ(s, dc<int>(1));
    CHECK_EQ(d, dc<int>(1));
  }
  {
    auto s = optional<dc<int>&>();
    auto d = optional<dc<int>&>();
    d = s;
    CHECK(!s);
    CHECK(!d);
  }
  {
    auto val = dc<int>(1);
    auto s = optional<dc<int>&>(val);
    auto d = optional<dc<int>&>();
    d = s;
    CHECK_EQ(s, dc<int>(1));
    CHECK_EQ(d, dc<int>(1));
  }
  {
    auto val = dc<int>(1);
    auto s = optional<dc<int>&>();
    auto d = optional<dc<int>&>(val);
    d = s;
    CHECK(!s);
    CHECK(!d);
  }
  {
    auto val = dc<int>(1);
    auto val2 = dc<int>({ 0, 1, 2 });
    auto s = optional<dc<int>&>(val);
    auto d = optional<dc<int>&>(val2);
    d = s;
    CHECK_EQ(s, dc<int>(1));
    CHECK_EQ(d, dc<int>(1));
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
  {
    auto s = optional<dc<int>>();
    auto d = optional<dc<int>>();
    d = std::move(s);
    CHECK(!s);
    CHECK(!d);
  }
  {
    auto s = optional<dc<int>>(1);
    auto d = optional<dc<int>>();
    d = std::move(s);
    CHECK_EQ(s, dc<int>());
    CHECK_EQ(d, dc<int>(1));
  }
  {
    auto s = optional<dc<int>>();
    auto d = optional<dc<int>>(1);
    d = std::move(s);
    CHECK(!s);
    CHECK(!d);
  }
  {
    auto s = optional<dc<int>>(1);
    auto d = optional<dc<int>>(1);
    d = std::move(s);
    CHECK_EQ(s, dc<int>());
    CHECK_EQ(d, dc<int>(1));
  }
  {
    auto s = optional<dc<int>&>();
    auto d = optional<dc<int>&>();
    d = std::move(s);
    CHECK(!s);
    CHECK(!d);
  }
  {
    auto val = dc<int>(1);
    auto s = optional<dc<int>&>(val);
    auto d = optional<dc<int>&>();
    d = std::move(s);
    CHECK_EQ(s, dc<int>(1));
    CHECK_EQ(d, dc<int>(1));
  }
  {
    auto val = dc<int>(1);
    auto s = optional<dc<int>&>();
    auto d = optional<dc<int>&>(val);
    d = std::move(s);
    CHECK(!s);
    CHECK(!d);
  }
  {
    auto val = dc<int>(1);
    auto val2 = dc<int>({ 0, 1, 2 });
    auto s = optional<dc<int>&>(val);
    auto d = optional<dc<int>&>(val2);
    d = std::move(s);
    CHECK_EQ(s, dc<int>(1));
    CHECK_EQ(d, dc<int>(1));
  }
}

TEST_CASE("converting assignment operator")
{
  {
    auto s = optional<dc<int>>();
    auto d = optional<std::vector<int>>();
    d = s;
    CHECK(!s);
    CHECK(!d);
  }
  {
    auto s = optional<dc<int>>(std::in_place, { 0, 1, 2 });
    auto d = optional<std::vector<int>>();
    d = s;
    CHECK(s);
    CHECK_EQ(d.value(), dc<int> { 0, 1, 2 });
  }
  {
    auto s = optional<dc<int>>();
    auto d = optional<std::vector<int>>();
    d = std::move(s);
    CHECK(!s);
    CHECK(!d);
  }
  {
    auto s = optional<dc<int>>(std::in_place, { 0, 1, 2 });
    auto d = optional<std::vector<int>>();
    d = std::move(s);
    CHECK(s);
    CHECK_EQ(d.value(), dc<int> { 0, 1, 2 });
  }
  {
    auto s = optional<dc<int>&>();
    auto d = optional<std::vector<int>&>();
    d = s;
    CHECK(!s);
    CHECK(!d);
  }
  {
    auto val = dc<int>({ 0, 1, 2 });
    auto s = optional<dc<int>&>(val);
    auto d = optional<std::vector<int>&>();
    d = s;
    CHECK(s);
    CHECK_EQ(d.value(), dc<int> { 0, 1, 2 });
  }
  {
    auto s = optional<dc<int>&>();
    auto d = optional<std::vector<int>&>();
    d = std::move(s);
    CHECK(!s);
    CHECK(!d);
  }
  {
    auto val = dc<int>({ 0, 1, 2 });
    auto s = optional<dc<int>&>(val);
    auto d = optional<std::vector<int>&>();
    d = std::move(s);
    CHECK(s);
    CHECK_EQ(d.value(), dc<int> { 0, 1, 2 });
  }
}

TEST_CASE("operator->")
{
  assert_is_same<memof_op, optional<void>&, void>();
  assert_is_same<memof_op, const optional<void>&, void>();
  assert_is_same<memof_op, optional<void>, void>();
  assert_is_same<memof_op, const optional<void>, void>();
  assert_is_same<memof_op, optional<int>&, int*>();
  assert_is_same<memof_op, const optional<int>&, const int*>();
  assert_is_same<memof_op, optional<int>, int*>();
  assert_is_same<memof_op, const optional<int>, const int*>();
  assert_is_same<memof_op, optional<int&>&, int*>();
  assert_is_same<memof_op, const optional<int&>&, int*>();
  assert_is_same<memof_op, optional<int&>, int*>();
  assert_is_same<memof_op, const optional<int&>, int*>();
  {
    int val = 1;
    auto exp = optional<int&>(val);
    CHECK_EQ(exp.operator->(), &val);
  }
  {
    auto exp = optional<dc<int>>(std::in_place, { 0, 1, 2 });
    CHECK_EQ(exp->size(), 3);
  }
  {
    const auto exp = optional<dc<int>>(std::in_place, { 0, 1, 2 });
    CHECK_EQ(exp->size(), 3);
  }
  {
    auto val = dc<int>({ 0, 1, 2 });
    auto exp = optional<dc<int>&>(val);
    CHECK_EQ(exp->size(), 3);
  }
  {
    auto val = dc<int>({ 0, 1, 2 });
    const auto exp = optional<dc<int>&>(val);
    CHECK_EQ(exp->size(), 3);
  }
}

TEST_CASE("operator*")
{
  assert_is_same<deref_op, optional<void>&, void>();
  assert_is_same<deref_op, const optional<void>&, void>();
  assert_is_same<deref_op, optional<void>, void>();
  assert_is_same<deref_op, const optional<void>, void>();
  assert_is_same<deref_op, optional<int>&, int&>();
  assert_is_same<deref_op, const optional<int>&, const int&>();
  assert_is_same<deref_op, optional<int>, int&&>();
  assert_is_same<deref_op, const optional<int>, const int&&>();
  assert_is_same<deref_op, optional<int&>&, int&>();
  assert_is_same<deref_op, const optional<int&>&, int&>();
  assert_is_same<deref_op, optional<int&>, int&>();
  assert_is_same<deref_op, const optional<int&>, int&>();
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
  {
    int val = 1;
    auto o = optional<int&>(val);
    CHECK_EQ(*o, 1);
  }
  {
    int val = 1;
    const auto o = optional<int&>(val);
    CHECK_EQ(*o, 1);
  }
  {
    int val = 1;
    auto o = optional<int&>(val);
    CHECK_EQ(*std::move(o), 1);
  }
  {
    int val = 1;
    const auto o = optional<int&>(val);
    CHECK_EQ(*std::move(o), 1);
  }
}

TEST_CASE("value")
{
  assert_is_same<fn_value, optional<void>&, void>();
  assert_is_same<fn_value, const optional<void>&, void>();
  assert_is_same<fn_value, optional<void>, void>();
  assert_is_same<fn_value, const optional<void>, void>();
  assert_is_same<fn_value, optional<int>&, int&>();
  assert_is_same<fn_value, const optional<int>&, const int&>();
  assert_is_same<fn_value, optional<int>, int&&>();
  assert_is_same<fn_value, const optional<int>, const int&&>();
  assert_is_same<fn_value, optional<int&>&, int&>();
  assert_is_same<fn_value, const optional<int&>&, int&>();
  assert_is_same<fn_value, optional<int&>, int&>();
  assert_is_same<fn_value, const optional<int&>, int&>();
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
  {
    int val = 1;
    auto o = optional<int&>(val);
    CHECK_EQ(o.value(), 1);
  }
  {
    int val = 1;
    const auto o = optional<int&>(val);
    CHECK_EQ(o.value(), 1);
  }
  {
    int val = 1;
    auto o = optional<int&>(val);
    CHECK_EQ(std::move(o).value(), 1);
  }
  {
    int val = 1;
    const auto o = optional<int&>(val);
    CHECK_EQ(std::move(o).value(), 1);
  }
#if !FITORIA_NO_EXCEPTIONS
  {
    auto o = optional<int&>();
    CHECK_THROWS_AS(o.value(), bad_optional_access);
  }
  {
    const auto o = optional<int&>();
    CHECK_THROWS_AS(o.value(), bad_optional_access);
  }
  {
    auto o = optional<int&>();
    CHECK_THROWS_AS(std::move(o).value(), bad_optional_access);
  }
  {
    const auto o = optional<int&>();
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
  {
    optional<int&> o;
    CHECK_EQ(o.value_or(2LL), 2);
  }
  {
    int val = 1;
    optional<int&> o(val);
    CHECK_EQ(o.value_or(2LL), 1);
  }
  {
    optional<int&> o;
    CHECK_EQ(std::move(o).value_or(2LL), 2);
  }
  {
    int val = 1;
    optional<int&> o(val);
    CHECK_EQ(std::move(o).value_or(2LL), 1);
  }
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
    optional<dc<int>> o;
    CHECK(!o);
    o.emplace({ 3, 4, 5 });
    CHECK_EQ(o.value(), dc<int> { 3, 4, 5 });
  }
  {
    optional<dc<int>> o(std::in_place, { 0, 1, 2 });
    CHECK(o);
    o.emplace({ 3, 4, 5 });
    CHECK_EQ(o.value(), dc<int> { 3, 4, 5 });
  }
  {
    optional<int&> o;
    CHECK(!o);
    int val = 1;
    o.emplace(val);
    CHECK_EQ(o.value(), 1);
  }
  {
    int val = 1;
    optional<int&> o(val);
    CHECK(o);
    int val2 = 2;
    o.emplace(val2);
    CHECK_EQ(o.value(), 2);
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
  {
    optional<int> o(1);
    CHECK(o);
    o.reset();
    CHECK(!o);
  }
  {
    int val = 1;
    optional<int&> o(val);
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
  {
    int val = 0;
    auto f = [](int& i) -> optional<int&> { return ++i; };
    SUBCASE("")
    {
      auto o = optional<int&>();
      CHECK(!o.and_then(f));
      CHECK_EQ(val, 0);
    }
    SUBCASE("")
    {
      auto o = optional<int&>(val);
      CHECK_EQ(o.and_then(f), 1);
      CHECK_EQ(val, 1);
    }
    SUBCASE("")
    {
      const auto o = optional<int&>();
      CHECK(!o.and_then(f));
      CHECK_EQ(val, 0);
    }
    SUBCASE("")
    {
      const auto o = optional<int&>(val);
      CHECK_EQ(o.and_then(f), 1);
      CHECK_EQ(val, 1);
    }
    SUBCASE("")
    {
      auto o = optional<int&>();
      CHECK(!std::move(o).and_then(f));
      CHECK_EQ(val, 0);
    }
    SUBCASE("")
    {
      auto o = optional<int&>(val);
      CHECK_EQ(std::move(o).and_then(f), 1);
      CHECK_EQ(val, 1);
    }
    SUBCASE("")
    {
      const auto o = optional<int&>();
      CHECK(!std::move(o).and_then(f));
      CHECK_EQ(val, 0);
    }
    SUBCASE("")
    {
      const auto o = optional<int&>(val);
      CHECK_EQ(std::move(o).and_then(f), 1);
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
  {
    int val = 0;
    auto f = [](int& i) -> int& { return ++i; };
    SUBCASE("")
    {
      auto o = optional<int&>();
      CHECK(!o.transform(f));
      CHECK_EQ(val, 0);
    }
    SUBCASE("")
    {
      auto o = optional<int&>(val);
      CHECK_EQ(o.transform(f), 1);
      CHECK_EQ(val, 1);
    }
    SUBCASE("")
    {
      const auto o = optional<int&>();
      CHECK(!o.transform(f));
      CHECK_EQ(val, 0);
    }
    SUBCASE("")
    {
      const auto o = optional<int&>(val);
      CHECK_EQ(o.transform(f), 1);
      CHECK_EQ(val, 1);
    }
    SUBCASE("")
    {
      auto o = optional<int&>();
      CHECK(!std::move(o).transform(f));
      CHECK_EQ(val, 0);
    }
    SUBCASE("")
    {
      auto o = optional<int&>(val);
      CHECK_EQ(std::move(o).transform(f), 1);
      CHECK_EQ(val, 1);
    }
    SUBCASE("")
    {
      const auto o = optional<int&>();
      CHECK(!std::move(o).transform(f));
      CHECK_EQ(val, 0);
    }
    SUBCASE("")
    {
      const auto o = optional<int&>(val);
      CHECK_EQ(std::move(o).transform(f), 1);
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
  {
    auto f = []() -> optional<int> { return optional<int>(2); };
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
  {
    int val = 1;
    int val2 = 2;
    auto f = [&]() -> optional<int&> { return optional<int&>(val2); };
    {
      auto o = optional<int&>();
      CHECK_EQ(o.or_else(f), 2);
    }
    {
      auto o = optional<int&>(val);
      CHECK_EQ(o.or_else(f), 1);
    }
    {
      const auto o = optional<int&>();
      CHECK_EQ(o.or_else(f), 2);
    }
    {
      const auto o = optional<int&>(val);
      CHECK_EQ(o.or_else(f), 1);
    }
    {
      auto o = optional<int&>();
      CHECK_EQ(std::move(o).or_else(f), 2);
    }
    {
      auto o = optional<int&>(val);
      CHECK_EQ(std::move(o).or_else(f), 1);
    }
    {
      const auto o = optional<int&>();
      CHECK_EQ(std::move(o).or_else(f), 2);
    }
    {
      const auto o = optional<int&>(val);
      CHECK_EQ(std::move(o).or_else(f), 1);
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
    auto b = optional<int>();
    swap(a, b);
    CHECK(!a);
    CHECK_EQ(b.value(), 1);
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

  auto i = make_optional<dc<int>>(1);
  CHECK_EQ(i.value(), dc<int>(1));

  auto vec = make_optional<dc<int>>({ 0, 1, 2 });
  CHECK_EQ(vec.value(), dc<int> { 0, 1, 2 });
}

TEST_CASE("format")
{
  {
    CHECK_EQ(fmt::format("{}", optional<int>(1)), "1");
    CHECK_EQ(fmt::format("{}", optional<int>()), "{nullopt}");
  }
  {
    int val = 1;
    CHECK_EQ(fmt::format("{}", optional<const int&>(val)), "1");
    CHECK_EQ(fmt::format("{}", optional<const int&>()), "{nullopt}");
  }
}

TEST_CASE("value_to_optional")
{
  {
    auto e = expected<void, int>(unexpect);
    CHECK_EQ(value_to_optional(e), optional<void>());
  }
  {
    auto e = expected<void, int>();
    CHECK_EQ(value_to_optional(e), optional<void>(std::in_place));
  }
  {
    const auto e = expected<void, int>(unexpect);
    CHECK_EQ(value_to_optional(e), optional<void>());
  }
  {
    const auto e = expected<void, int>();
    CHECK_EQ(value_to_optional(e), optional<void>(std::in_place));
  }
  {
    auto e = expected<void, int>(unexpect);
    CHECK_EQ(value_to_optional(std::move(e)), optional<void>());
  }
  {
    auto e = expected<void, int>();
    CHECK_EQ(value_to_optional(std::move(e)), optional<void>(std::in_place));
  }
  {
    const auto e = expected<void, int>(unexpect);
    CHECK_EQ(value_to_optional(std::move(e)), optional<void>());
  }
  {
    const auto e = expected<void, int>();
    CHECK_EQ(value_to_optional(std::move(e)), optional<void>(std::in_place));
  }
  {
    auto e = expected<int, int>(unexpect);
    CHECK_EQ(value_to_optional(e), optional<int>());
  }
  {
    auto e = expected<int, int>(1);
    CHECK_EQ(value_to_optional(e), optional<int>(1));
  }
  {
    const auto e = expected<int, int>(unexpect);
    CHECK_EQ(value_to_optional(e), optional<int>());
  }
  {
    const auto e = expected<int, int>(1);
    CHECK_EQ(value_to_optional(e), optional<int>(1));
  }
  {
    auto e = expected<int, int>(unexpect);
    CHECK_EQ(value_to_optional(std::move(e)), optional<int>());
  }
  {
    auto e = expected<int, int>(1);
    CHECK_EQ(value_to_optional(std::move(e)), optional<int>(1));
  }
  {
    const auto e = expected<int, int>(unexpect);
    CHECK_EQ(value_to_optional(std::move(e)), optional<int>());
  }
  {
    const auto e = expected<int, int>(1);
    CHECK_EQ(value_to_optional(std::move(e)), optional<int>(1));
  }
}

TEST_CASE("error_to_optional")
{
  {
    auto e = expected<void, int>(unexpect, 1);
    CHECK_EQ(error_to_optional(e), optional<int>(1));
  }
  {
    auto e = expected<void, int>();
    CHECK_EQ(error_to_optional(e), optional<int>());
  }
  {
    const auto e = expected<void, int>(unexpect, 1);
    CHECK_EQ(error_to_optional(e), optional<int>(1));
  }
  {
    const auto e = expected<void, int>();
    CHECK_EQ(error_to_optional(e), optional<int>());
  }
  {
    auto e = expected<void, int>(unexpect, 1);
    CHECK_EQ(error_to_optional(std::move(e)), optional<int>(1));
  }
  {
    auto e = expected<void, int>();
    CHECK_EQ(error_to_optional(std::move(e)), optional<int>());
  }
  {
    const auto e = expected<void, int>(unexpect, 1);
    CHECK_EQ(error_to_optional(std::move(e)), optional<int>(1));
  }
  {
    const auto e = expected<void, int>();
    CHECK_EQ(error_to_optional(std::move(e)), optional<int>());
  }
  {
    auto e = expected<int, int>(unexpect);
    CHECK_EQ(error_to_optional(e), optional<int>(0));
  }
  {
    auto e = expected<int, int>(1);
    CHECK_EQ(error_to_optional(e), optional<int>());
  }
  {
    const auto e = expected<int, int>(unexpect);
    CHECK_EQ(error_to_optional(e), optional<int>(0));
  }
  {
    const auto e = expected<int, int>(1);
    CHECK_EQ(error_to_optional(e), optional<int>());
  }
  {
    auto e = expected<int, int>(unexpect);
    CHECK_EQ(error_to_optional(std::move(e)), optional<int>(0));
  }
  {
    auto e = expected<int, int>(1);
    CHECK_EQ(error_to_optional(std::move(e)), optional<int>());
  }
  {
    const auto e = expected<int, int>(unexpect);
    CHECK_EQ(error_to_optional(std::move(e)), optional<int>(0));
  }
  {
    const auto e = expected<int, int>(1);
    CHECK_EQ(error_to_optional(std::move(e)), optional<int>());
  }
}

TEST_SUITE_END();
