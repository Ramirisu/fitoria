//
// Copyright (c) 2022 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <type_traits>

#define STATIC_ASSERT(...) static_assert(__VA_ARGS__, "")
#define STATIC_ASSERT_SAME(...)                                                \
  static_assert(std::is_same<__VA_ARGS__>::value, "")
