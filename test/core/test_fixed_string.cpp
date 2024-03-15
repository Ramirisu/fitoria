//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria_test.h>

#include <fitoria/core/fixed_string.hpp>

using namespace fitoria;

TEST_SUITE_BEGIN("[fitoria.core.fixed_string]");

template <basic_fixed_string Pattern>
struct matcher {
  std::string_view v() const noexcept
  {
    return Pattern;
  }

  template <basic_fixed_string P>
  constexpr auto concat() const noexcept
  {
    return matcher<Pattern + P>();
  }
};

TEST_CASE("fixed_string")
{
  auto x = matcher<"abc">();
  CHECK_EQ(x.v(), "abc");

  auto y = x.concat<"def">();
  CHECK_EQ(y.v(), "abcdef");
}

TEST_SUITE_END();
