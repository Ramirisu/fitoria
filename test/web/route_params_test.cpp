//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria_test.h>

#include <fitoria/web/route_params.hpp>

using namespace fitoria::web;

TEST_SUITE_BEGIN("[fitoria.web.route_params]");

TEST_CASE("basic")
{
  route_params params({}, "/api/v1/users/{user}");
  CHECK_EQ(params.path(), "/api/v1/users/{user}");
}

TEST_SUITE_END();
