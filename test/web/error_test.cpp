//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria_test.h>

#include <fitoria/web/error.hpp>

using namespace fitoria;
using namespace fitoria::web;

TEST_SUITE_BEGIN("[fitoria.web.error]");

TEST_CASE("error")
{
  CHECK_EQ(make_error_code(error::route_already_exists).message(),
           "the route being registered already exists");
  CHECK_EQ(make_error_code(error::route_not_exists).message(),
           "the route being searched doesn't exist");
  CHECK_EQ(make_error_code(error::unexpected_content_type).message(),
           "unexpected Content-Type");
  CHECK_EQ(make_error_code(error::shared_state_not_exists).message(),
           "the shared state being obtained doesn't exist");
}

TEST_SUITE_END();
