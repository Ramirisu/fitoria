//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef FITORIA_FITORIA_TEST_HPP
#define FITORIA_FITORIA_TEST_HPP

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <algorithm>
#include <type_traits>

template <typename L, typename R, typename Comparator = std::equal_to<>>
bool range_equal(const L& lhs,
                 const R& rhs,
                 Comparator comparator = Comparator())
{
  return std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end(),
                    std::move(comparator));
}

#endif
