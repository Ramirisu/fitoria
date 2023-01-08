//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria_test.h>

#include <fitoria/http_server/http_route.hpp>

using namespace fitoria;

TEST_SUITE_BEGIN("http_route");

TEST_CASE("basic")
{
  http_route route({}, "/api/v1/users/{user}");
  CHECK_EQ(route.path(), "/api/v1/users/{user}");
}

TEST_SUITE_END();
