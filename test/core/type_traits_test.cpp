//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria_test.h>

#include <fitoria/core/type_traits.hpp>

#include <string>
#include <vector>

using namespace fitoria;

TEST_SUITE_BEGIN("[fitoria.core.type_traits]");

TEST_CASE("is_specialization_of")
{
  static_assert(is_specialization_of_v<std::string, std::basic_string>);
  static_assert(is_specialization_of_v<std::vector<int>, std::vector>);
  static_assert(is_specialization_of_v<std::vector<int&>, std::vector>);
  static_assert(is_specialization_of_v<std::vector<const int>, std::vector>);
  static_assert(is_specialization_of_v<std::vector<const int&>, std::vector>);

  static_assert(!is_specialization_of_v<int, std::basic_string>);
  static_assert(!is_specialization_of_v<std::string&, std::basic_string>);
  static_assert(!is_specialization_of_v<const std::string&, std::basic_string>);
  static_assert(!is_specialization_of_v<const std::string&, std::basic_string>);
}

namespace {
void f() { }
}

TEST_CASE("function_traits")
{
  auto l = []() {};
  void (*fptr)();
  fptr = f;
  struct c {
    void f() { }
    void fc() const { }
    void fl() & { }
    void fcl() const& { }
    void fr() && { }
    void fcr() const&& { }
    void fn() noexcept { }
    void fln() & noexcept { }
    void fcln() const& noexcept { }
    void frn() && noexcept { }
    void fcrn() const&& noexcept { }
    void operator()() { }
    double unary(int)
    {
      return 0;
    }
    int binary(char, long)
    {
      return 0;
    }
    long ternary(short, int, char)
    {
      return 0;
    }
  };

  // clang-format off
  static_assert(std::is_same_v<function_traits<decltype(l)>::result_type, void>);
  static_assert(std::is_same_v<function_traits<decltype(f)>::result_type, void>);
  static_assert(std::is_same_v<function_traits<decltype(fptr)>::result_type, void>);
  static_assert(std::is_same_v<function_traits<decltype(&c::f)>::result_type, void>);
  static_assert(std::is_same_v<function_traits<decltype(&c::fc)>::result_type, void>);
  static_assert(std::is_same_v<function_traits<decltype(&c::fl)>::result_type, void>);
  static_assert(std::is_same_v<function_traits<decltype(&c::fcl)>::result_type, void>);
  static_assert(std::is_same_v<function_traits<decltype(&c::fr)>::result_type, void>);
  static_assert(std::is_same_v<function_traits<decltype(&c::fcr)>::result_type, void>);
  static_assert(std::is_same_v<function_traits<decltype(&c::fn)>::result_type, void>);
  static_assert(std::is_same_v<function_traits<decltype(&c::fln)>::result_type, void>);
  static_assert(std::is_same_v<function_traits<decltype(&c::fcln)>::result_type, void>);
  static_assert(std::is_same_v<function_traits<decltype(&c::frn)>::result_type, void>);
  static_assert(std::is_same_v<function_traits<decltype(&c::fcrn)>::result_type, void>);
  static_assert(std::is_same_v<function_traits<c>::result_type, void>);

  static_assert(std::is_same_v<function_traits<decltype(&c::unary)>::result_type, double>);
  static_assert(function_traits<decltype(&c::unary)>::arity == 1);
  static_assert(std::is_same_v<function_traits<decltype(&c::unary)>::args_type, std::tuple<int>>);
  static_assert(std::is_same_v<function_traits<decltype(&c::unary)>::template arg<0>::type, int>);

  static_assert(std::is_same_v<function_traits<decltype(&c::binary)>::result_type, int>);
  static_assert(function_traits<decltype(&c::binary)>::arity == 2);
  static_assert(std::is_same_v<function_traits<decltype(&c::binary)>::args_type, std::tuple<char, long>>);
  static_assert(std::is_same_v<function_traits<decltype(&c::binary)>::template arg<0>::type, char>);
  static_assert(std::is_same_v<function_traits<decltype(&c::binary)>::template arg<1>::type, long>);

  static_assert(std::is_same_v<function_traits<decltype(&c::ternary)>::result_type, long>);
  static_assert(function_traits<decltype(&c::ternary)>::arity == 3);
  static_assert(std::is_same_v<function_traits<decltype(&c::ternary)>::args_type, std::tuple<short, int, char>>);
  static_assert(std::is_same_v<function_traits<decltype(&c::ternary)>::template arg<0>::type, short>);
  static_assert(std::is_same_v<function_traits<decltype(&c::ternary)>::template arg<1>::type, int>);
  static_assert(std::is_same_v<function_traits<decltype(&c::ternary)>::template arg<2>::type, char>);
  // clang-format on
}

TEST_SUITE_END();
