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

TEST_SUITE_BEGIN("[fitoria.core.expected.lref]");

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

TEST_CASE("trivially")
{
  {
    using type = expected<int&, int>;
    static_assert(std::is_trivially_copy_constructible_v<type>);
    static_assert(std::is_trivially_copy_assignable_v<type>);
    static_assert(std::is_trivially_move_constructible_v<type>);
    static_assert(std::is_trivially_move_assignable_v<type>);
    static_assert(std::is_trivially_destructible_v<type>);
  }
  {
    using type = expected<int&, std::vector<int>>;
    static_assert(!std::is_trivially_copy_constructible_v<type>);
    static_assert(!std::is_trivially_copy_assignable_v<type>);
    static_assert(!std::is_trivially_move_constructible_v<type>);
    static_assert(!std::is_trivially_move_assignable_v<type>);
    static_assert(!std::is_trivially_destructible_v<type>);
  }
}

TEST_CASE("default constructor")
{
  static_assert(!std::is_default_constructible_v<expected<int&, int>>);
}

TEST_CASE("copy constructor")
{
  {
    int val = 1;
    auto s = expected<int&, int>(val);
    auto d = expected<int&, int>(s);
    CHECK_EQ(s, 1);
    CHECK_EQ(d, 1);
  }
  {
    auto s = expected<int&, int>(unexpect, -1);
    auto d = expected<int&, int>(s);
    CHECK_EQ(s, fitoria::unexpected<int>(-1));
    CHECK_EQ(d, fitoria::unexpected<int>(-1));
  }
}

TEST_CASE("move constructor")
{
  {
    int val = 1;
    auto s = expected<int&, int>(val);
    auto d = expected<int&, int>(std::move(s));
    CHECK_EQ(s, 1);
    CHECK_EQ(d, 1);
  }
  {
    auto s = expected<int&, int>(unexpect, -1);
    auto d = expected<int&, int>(std::move(s));
    CHECK_EQ(s, fitoria::unexpected<int>(-1));
    CHECK_EQ(d, fitoria::unexpected<int>(-1));
  }
}

TEST_CASE("perfect forwarding constructor")
{
  {
    int val = 1;
    auto exp = expected<int&, int>(val);
    CHECK_EQ(exp.value(), 1);
  }
}

TEST_CASE("converting constructor")
{
  {
    auto d = expected<test_vector<int>&, int>(unexpect, -1);
    auto s = expected<std::vector<int>&, int>(d);
    CHECK_EQ(s, fitoria::unexpected<int>(-1));
  }
  {
    auto val = test_vector<int> { 1, 2, 3 };
    auto d = expected<test_vector<int>&, int>(val);
    auto s = expected<std::vector<int>&, int>(d);
    CHECK_EQ(s, std::vector<int> { 1, 2, 3 });
  }
}

TEST_CASE("copy assignment operator")
{
  {
    int val = 1;
    auto s = expected<int&, int>(val);
    auto d = expected<int&, int>(unexpect, -2);
    d = s;
    CHECK_EQ(s, 1);
    CHECK_EQ(d, 1);
  }
  {
    auto s = expected<int&, int>(unexpect, -1);
    auto d = expected<int&, int>(unexpect, -2);
    d = s;
    CHECK_EQ(s, fitoria::unexpected<int>(-1));
    CHECK_EQ(d, fitoria::unexpected<int>(-1));
  }
}

TEST_CASE("move assignment operator")
{
  {
    int val = 1;
    auto s = expected<int&, int>(val);
    auto d = expected<int&, int>(unexpect, -2);
    d = std::move(s);
    CHECK_EQ(s, 1);
    CHECK_EQ(d, 1);
  }
  {
    auto s = expected<int&, int>(unexpect, -1);
    auto d = expected<int&, int>(unexpect, -2);
    d = std::move(s);
    CHECK_EQ(s, fitoria::unexpected<int>(-1));
    CHECK_EQ(d, fitoria::unexpected<int>(-1));
  }
}

TEST_CASE("perfect forwarding assignment operator")
{
  {
    auto val = std::vector<int> { 1, 2, 3 };
    auto exp = expected<std::vector<int>, int>();
    exp = val;
    CHECK_EQ(exp, std::vector<int> { 1, 2, 3 });
  }
}

TEST_CASE("operator->")
{
  static_assert(std::is_same_v<memop_mem_of<expected<int&, int>&>, int*>);
  static_assert(std::is_same_v<memop_mem_of<const expected<int&, int>&>, int*>);
  static_assert(std::is_same_v<memop_mem_of<expected<int&, int>>, int*>);
  static_assert(std::is_same_v<memop_mem_of<const expected<int&, int>>, int*>);
  {
    auto val = std::vector<int>({ 0, 1, 2 });
    auto exp = expected<std::vector<int>&, int>(val);
    CHECK_EQ(exp->size(), 3);
  }
  {
    auto val = std::vector<int>({ 0, 1, 2 });
    const auto exp = expected<std::vector<int>&, int>(val);
    CHECK_EQ(exp->size(), 3);
  }
}

TEST_CASE("operator*")
{
  static_assert(std::is_same_v<memop_deref<expected<int&, int>&>, int&>);
  static_assert(std::is_same_v<memop_deref<const expected<int&, int>&>, int&>);
  static_assert(std::is_same_v<memop_deref<expected<int&, int>>, int&>);
  static_assert(std::is_same_v<memop_deref<const expected<int&, int>>, int&>);
  {
    auto val = std::vector<int>({ 0, 1, 2 });
    auto exp = expected<std::vector<int>&, std::vector<int>>(val);
    CHECK_EQ(*exp, std::vector<int>({ 0, 1, 2 }));
  }
  {
    auto val = std::vector<int>({ 0, 1, 2 });
    const auto exp = expected<std::vector<int>&, std::vector<int>>(val);
    CHECK_EQ(*exp, std::vector<int>({ 0, 1, 2 }));
  }
  {
    auto val = std::vector<int>({ 0, 1, 2 });
    auto exp = expected<std::vector<int>&, std::vector<int>>(val);
    CHECK_EQ(*std::move(exp), std::vector<int>({ 0, 1, 2 }));
  }
  {
    auto val = std::vector<int>({ 0, 1, 2 });
    const auto exp = expected<std::vector<int>&, std::vector<int>>(val);
    CHECK_EQ(*std::move(exp), std::vector<int>({ 0, 1, 2 }));
  }
}

TEST_CASE("has_value")
{
  int val = 1;
  CHECK(expected<int&, int>(val).has_value());
  CHECK(!expected<int&, int>(unexpect, 1).has_value());
}

TEST_CASE("value")
{
  static_assert(std::is_same_v<memfn_value<expected<int&, int>&>, int&>);
  static_assert(std::is_same_v<memfn_value<const expected<int&, int>&>, int&>);
  static_assert(std::is_same_v<memfn_value<expected<int&, int>>, int&>);
  static_assert(std::is_same_v<memfn_value<const expected<int&, int>>, int&>);
  {
    auto val = std::vector<int>({ 0, 1, 2 });
    auto exp = expected<std::vector<int>&, int>(val);
    CHECK_EQ(exp.value(), std::vector<int>({ 0, 1, 2 }));
  }
  {
    auto val = std::vector<int>({ 0, 1, 2 });
    const auto exp = expected<std::vector<int>&, int>(val);
    CHECK_EQ(exp.value(), std::vector<int>({ 0, 1, 2 }));
  }
  {
    auto val = std::vector<int>({ 0, 1, 2 });
    auto exp = expected<std::vector<int>&, int>(val);
    CHECK_EQ(std::move(exp).value(), std::vector<int>({ 0, 1, 2 }));
  }
  {
    auto val = std::vector<int>({ 0, 1, 2 });
    const auto exp = expected<std::vector<int>&, int>(val);
    CHECK_EQ(std::move(exp).value(), std::vector<int>({ 0, 1, 2 }));
  }
#if !FITORIA_NO_EXCEPTIONS
  {
    auto exp = expected<std::vector<int>&, int>(unexpect);
    CHECK_THROWS_AS(exp.value(), bad_expected_access<int>);
  }
  {
    const auto exp = expected<std::vector<int>&, int>(unexpect);
    CHECK_THROWS_AS(exp.value(), bad_expected_access<int>);
  }
  {
    auto exp = expected<std::vector<int>&, int>(unexpect);
    CHECK_THROWS_AS(std::move(exp).value(), bad_expected_access<int>);
  }
  {
    const auto exp = expected<std::vector<int>&, int>(unexpect);
    CHECK_THROWS_AS(std::move(exp).value(), bad_expected_access<int>);
  }
#endif
}

TEST_CASE("error")
{
  static_assert(std::is_same_v<memfn_error<expected<int&, int>&>, int&>);
  static_assert(
      std::is_same_v<memfn_error<const expected<int&, int>&>, const int&>);
  static_assert(std::is_same_v<memfn_error<expected<int&, int>>, int&&>);
  static_assert(
      std::is_same_v<memfn_error<const expected<int&, int>>, const int&&>);
  {
    auto exp = expected<std::vector<int>&, int>(unexpect, -1);
    CHECK_EQ(exp.error(), -1);
  }
  {
    const auto exp = expected<std::vector<int>&, int>(unexpect, -1);
    CHECK_EQ(exp.error(), -1);
  }
  {
    auto exp = expected<std::vector<int>&, int>(unexpect, -1);
    CHECK_EQ(std::move(exp).error(), -1);
  }
  {
    const auto exp = expected<std::vector<int>&, int>(unexpect, -1);
    CHECK_EQ(std::move(exp).error(), -1);
  }
}

TEST_CASE("value_or")
{
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
    int val = 1;
    auto exp = expected<int&, int>(val);
    CHECK_EQ(exp.error_or(2LL), 2);
  }
  {
    int val = 1;
    auto exp = expected<int&, int>(val);
    CHECK_EQ(std::move(exp).error_or(2LL), 2);
  }
  {
    auto exp = expected<int&, int>(unexpect, 1);
    CHECK_EQ(exp.error_or(2LL), 1);
  }
  {
    auto exp = expected<int&, int>(unexpect, 1);
    CHECK_EQ(std::move(exp).error_or(2LL), 1);
  }
}

TEST_CASE("emplace")
{
  {
    auto val = std::vector<int>({ 0, 1, 2 });
    auto e = expected<std::vector<int>&, int>(val);
    auto val2 = std::vector<int>({ 3, 4, 5 });
    e.emplace(val2);
    CHECK_EQ(val, std::vector<int>({ 0, 1, 2 }));
    CHECK_EQ(e.value(), std::vector<int>({ 3, 4, 5 }));
  }
  {
    auto e = expected<std::vector<int>&, int>(unexpect, 0);
    auto val2 = std::vector<int>({ 3, 4, 5 });
    e.emplace(val2);
    CHECK_EQ(e.value(), std::vector<int>({ 3, 4, 5 }));
  }
}

TEST_CASE("and_then")
{
  {
    auto v = test_vector<int> { 1, 2, 3 };
    auto exp = expected<test_vector<int>&, int>(v);
    CHECK_EQ(exp.and_then([](auto& val) -> expected<std::vector<int>&, int> {
      return val;
    }),
             std::vector<int> { 1, 2, 3 });
  }
  {
    auto v = test_vector<int> { 1, 2, 3 };
    const auto exp = expected<test_vector<int>&, int>(v);
    CHECK_EQ(exp.and_then([](auto& val) -> expected<std::vector<int>&, int> {
      return val;
    }),
             std::vector<int> { 1, 2, 3 });
  }
  {
    auto v = test_vector<int> { 1, 2, 3 };
    auto exp = expected<test_vector<int>&, int>(v);
    CHECK_EQ(
        std::move(exp).and_then(
            [](auto& val) -> expected<std::vector<int>&, int> { return val; }),
        std::vector<int> { 1, 2, 3 });
  }
  {
    auto v = test_vector<int> { 1, 2, 3 };
    const auto exp = expected<test_vector<int>&, int>(v);
    CHECK_EQ(
        std::move(exp).and_then(
            [](auto& val) -> expected<std::vector<int>&, int> { return val; }),
        std::vector<int> { 1, 2, 3 });
  }
  {
    auto exp = expected<test_vector<int>&, int>(unexpect, -1);
    CHECK_EQ(exp.and_then([](auto& val) -> expected<std::vector<int>&, int> {
      return val;
    }),
             fitoria::unexpected<int>(-1));
  }
  {
    const auto exp = expected<test_vector<int>&, int>(unexpect, -1);
    CHECK_EQ(exp.and_then([](auto& val) -> expected<std::vector<int>&, int> {
      return val;
    }),
             fitoria::unexpected<int>(-1));
  }
  {
    auto exp = expected<test_vector<int>&, int>(unexpect, -1);
    CHECK_EQ(
        std::move(exp).and_then(
            [](auto& val) -> expected<std::vector<int>&, int> { return val; }),
        fitoria::unexpected<int>(-1));
  }
  {
    const auto exp = expected<test_vector<int>&, int>(unexpect, -1);
    CHECK_EQ(
        std::move(exp).and_then(
            [](auto& val) -> expected<std::vector<int>&, int> { return val; }),
        fitoria::unexpected<int>(-1));
  }
}

TEST_CASE("or_else")
{
  {
    int v = 1;
    auto exp = expected<int&, int>(v);
    CHECK_EQ(exp.or_else([](int err) -> expected<int&, int> {
      return fitoria::unexpected<int>(err - 1);
    }),
             1);
  }
  {
    int v = 1;
    const auto exp = expected<int&, int>(v);
    CHECK_EQ(exp.or_else([](int err) -> expected<int&, int> {
      return fitoria::unexpected<int>(err - 1);
    }),
             1);
  }
  {
    int v = 1;
    auto exp = expected<int&, int>(v);
    CHECK_EQ(std::move(exp).or_else([](int err) -> expected<int&, int> {
      return fitoria::unexpected<int>(err - 1);
    }),
             1);
  }
  {
    int v = 1;
    const auto exp = expected<int&, int>(v);
    CHECK_EQ(std::move(exp).or_else([](int err) -> expected<int&, int> {
      return fitoria::unexpected<int>(err - 1);
    }),
             1);
  }
  {
    auto exp = expected<int&, int>(unexpect, -1);
    CHECK_EQ(exp.or_else([](int err) -> expected<int&, int> {
      return fitoria::unexpected<int>(err - 1);
    }),
             fitoria::unexpected<int>(-2));
  }
  {
    const auto exp = expected<int&, int>(unexpect, -1);
    CHECK_EQ(exp.or_else([](int err) -> expected<int&, int> {
      return fitoria::unexpected<int>(err - 1);
    }),
             fitoria::unexpected<int>(-2));
  }
  {
    auto exp = expected<int&, int>(unexpect, -1);
    CHECK_EQ(std::move(exp).or_else([](int err) -> expected<int&, int> {
      return fitoria::unexpected<int>(err - 1);
    }),
             fitoria::unexpected<int>(-2));
  }
  {
    const auto exp = expected<int&, int>(unexpect, -1);
    CHECK_EQ(std::move(exp).or_else([](int err) -> expected<int&, int> {
      return fitoria::unexpected<int>(err - 1);
    }),
             fitoria::unexpected<int>(-2));
  }
}

TEST_CASE("transform")
{
  {
    auto v = test_vector<int> { 1, 2, 3 };
    auto exp = expected<test_vector<int>&, int>(v);
    CHECK_EQ(exp.transform([](auto& val) -> std::vector<int>& { return val; }),
             std::vector<int> { 1, 2, 3 });
  }
  {
    auto v = test_vector<int> { 1, 2, 3 };
    const auto exp = expected<test_vector<int>&, int>(v);
    CHECK_EQ(exp.transform([](auto& val) -> std::vector<int>& { return val; }),
             std::vector<int> { 1, 2, 3 });
  }
  {
    auto v = test_vector<int> { 1, 2, 3 };
    auto exp = expected<test_vector<int>&, int>(v);
    CHECK_EQ(std::move(exp).transform(
                 [](auto& val) -> std::vector<int>& { return val; }),
             std::vector<int> { 1, 2, 3 });
  }
  {
    auto v = test_vector<int> { 1, 2, 3 };
    const auto exp = expected<test_vector<int>&, int>(v);
    CHECK_EQ(std::move(exp).transform(
                 [](auto& val) -> std::vector<int>& { return val; }),
             std::vector<int> { 1, 2, 3 });
  }
  {
    auto exp = expected<test_vector<int>&, int>(unexpect, -1);
    CHECK_EQ(exp.transform([](auto& val) -> std::vector<int>& { return val; }),
             fitoria::unexpected<int>(-1));
  }
  {
    const auto exp = expected<test_vector<int>&, int>(unexpect, -1);
    CHECK_EQ(exp.transform([](auto& val) -> std::vector<int>& { return val; }),
             fitoria::unexpected<int>(-1));
  }
  {
    auto exp = expected<test_vector<int>&, int>(unexpect, -1);
    CHECK_EQ(std::move(exp).transform(
                 [](auto& val) -> std::vector<int>& { return val; }),
             fitoria::unexpected<int>(-1));
  }
  {
    const auto exp = expected<test_vector<int>&, int>(unexpect, -1);
    CHECK_EQ(std::move(exp).transform(
                 [](auto& val) -> std::vector<int>& { return val; }),
             fitoria::unexpected<int>(-1));
  }
}

TEST_CASE("transform_error")
{
  {
    int v = 1;
    auto exp = expected<int&, int>(v);
    CHECK_EQ(exp.transform_error([](int err) -> int { return err - 1; }), 1);
  }
  {
    int v = 1;
    const auto exp = expected<int&, int>(v);
    CHECK_EQ(exp.transform_error([](int err) -> int { return err - 1; }), 1);
  }
  {
    int v = 1;
    auto exp = expected<int&, int>(v);
    CHECK_EQ(
        std::move(exp).transform_error([](int err) -> int { return err - 1; }),
        1);
  }
  {
    int v = 1;
    const auto exp = expected<int&, int>(v);
    CHECK_EQ(
        std::move(exp).transform_error([](int err) -> int { return err - 1; }),
        1);
  }
  {
    auto exp = expected<int&, int>(unexpect, -1);
    CHECK_EQ(exp.transform_error([](int err) -> int { return err - 1; }),
             fitoria::unexpected<int>(-2));
  }
  {
    const auto exp = expected<int&, int>(unexpect, -1);
    CHECK_EQ(exp.transform_error([](int err) -> int { return err - 1; }),
             fitoria::unexpected<int>(-2));
  }
  {
    auto exp = expected<int&, int>(unexpect, -1);
    CHECK_EQ(
        std::move(exp).transform_error([](int err) -> int { return err - 1; }),
        fitoria::unexpected<int>(-2));
  }
  {
    const auto exp = expected<int&, int>(unexpect, -1);
    CHECK_EQ(
        std::move(exp).transform_error([](int err) -> int { return err - 1; }),
        fitoria::unexpected<int>(-2));
  }
}

TEST_CASE("swap")
{
  {
    int av = 1;
    int bv = 2;
    auto a = expected<int&, int>(av);
    auto b = expected<int&, int>(bv);
    swap(a, b);
    CHECK_EQ(std::addressof(a.value()), std::addressof(bv));
    CHECK_EQ(std::addressof(b.value()), std::addressof(av));
    CHECK_EQ(a, bv);
    CHECK_EQ(b, av);
  }
}

TEST_CASE("compare")
{
  int val = 1;

  CHECK(expected<int&, int>(val) == expected<int&, int>(val));
  CHECK(!(expected<int&, int>(val) != expected<int&, int>(val)));
  CHECK(expected<int&, int>(unexpect) == expected<int&, int>(unexpect));
  CHECK(!(expected<int&, int>(unexpect) != expected<int&, int>(unexpect)));
  CHECK(!(expected<int&, int>(val) == expected<int&, int>(unexpect)));
  CHECK(expected<int&, int>(val) != expected<int&, int>(unexpect));
  CHECK(!(expected<int&, int>(unexpect) == expected<int&, int>(val)));
  CHECK(expected<int&, int>(unexpect) != expected<int&, int>(val));

  CHECK(expected<int&, int>(val) == 1);
  CHECK(!(expected<int&, int>(unexpect) == 0));
  CHECK(!(expected<int&, int>(val) != 1));
  CHECK(expected<int&, int>(unexpect) != 0);

  CHECK(!(expected<int&, int>(val) == fitoria::unexpected<int>(0)));
  CHECK(expected<int&, int>(unexpect) == fitoria::unexpected<int>(0));
  CHECK(expected<int&, int>(val) != fitoria::unexpected<int>(0));
  CHECK(!(expected<int&, int>(unexpect) != fitoria::unexpected<int>(0)));
}

TEST_CASE("format")
{
  {
    int val = 0;
    CHECK_EQ(fmt::format("{}", expected<int&, int>(val)), "0");
    CHECK_EQ(fmt::format("{}", expected<int&, int>(unexpect)), "{unexpected}");
  }
}

TEST_SUITE_END();
