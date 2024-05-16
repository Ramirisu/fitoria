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
  CHECK_EQ(make_error_code(error::content_type_not_application_form_urlencoded)
               .message(),
           "unexpected Content-Type received, expected \"Content-Type: "
           "application/x-www-form-urlencoded\"");
  CHECK_EQ(make_error_code(error::content_type_not_application_json).message(),
           "unexpected Content-Type received, expected \"Content-Type: "
           "application/json\"");
  CHECK_EQ(make_error_code(error::extractor_field_count_not_match).message(),
           "unable to extract key/value into struct, field count not match");
  CHECK_EQ(make_error_code(error::extractor_field_name_not_found).message(),
           "unable to extract key/value into struct, field not found");
  CHECK_EQ(make_error_code(error::state_not_found).message(),
           "the state being obtained doesn't exist");
  CHECK_EQ(make_error_code(error::not_upgrade).message(),
           "request is not upgradable");
}

TEST_SUITE_END();
