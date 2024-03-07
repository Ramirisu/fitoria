//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria_test.h>

#include <fitoria/web/http/http.hpp>

using namespace fitoria;

TEST_SUITE_BEGIN("[fitoria.web.http]");

TEST_CASE("version")
{
  using namespace fitoria::web;

  CHECK_EQ(to_string(http::version::unknown), "UNKNOWN");
  CHECK_EQ(to_string(http::version::v10), "1.0");
  CHECK_EQ(to_string(http::version::v11), "1.1");
}

TEST_CASE("authorization")
{
  using namespace fitoria::web::http::fields;
  {
    CHECK_EQ(authorization::bearer("abc"), "Bearer: abc");
  }
  {
    CHECK_EQ(authorization::parse_bearer(""), nullopt);
    CHECK_EQ(authorization::parse_bearer("bearer: "), nullopt);
    CHECK_EQ(authorization::parse_bearer("Bearer: "), "");
    CHECK_EQ(authorization::parse_bearer("Bearer: abcd"), "abcd");
  }
}

TEST_SUITE_END();
