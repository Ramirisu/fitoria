//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria/test/test.hpp>

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
  CHECK_EQ(make_error_code(error::unexpected_content_type_json).message(),
           "unexpected Content-Type received, expected \"Content-Type: "
           "application/json\"");
  CHECK_EQ(make_error_code(error::path_extraction_error).message(),
           "path extraction error");
  CHECK_EQ(make_error_code(error::state_not_found).message(),
           "the state being obtained doesn't exist");
  CHECK_EQ(make_error_code(error::not_upgrade).message(),
           "request is not upgradable");
}

TEST_SUITE_END();
