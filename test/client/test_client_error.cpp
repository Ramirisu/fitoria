//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria/test/test.hpp>

#include <fitoria/client/error.hpp>

using namespace fitoria;
using namespace fitoria::client;

TEST_SUITE_BEGIN("[fitoria.client.error]");

TEST_CASE("error")
{
  CHECK_EQ(make_error_code(error::content_type_not_application_json).message(),
           "unexpected Content-Type received, expected \"Content-Type: "
           "application/json\"");
}

TEST_SUITE_END();
