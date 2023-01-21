//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria_test.h>

#include <fitoria/web/http/http.hpp>

using namespace fitoria;
using namespace fitoria::http;

TEST_SUITE_BEGIN("web.http");

TEST_CASE("authorization")
{
  {
    CHECK_EQ(authorization::barear("abc"), "Bearer: abc");
  }
  {
    CHECK_EQ(authorization::parse_barear(""), nullopt);
    CHECK_EQ(authorization::parse_barear("bearer: "), nullopt);
    CHECK_EQ(authorization::parse_barear("Bearer: "), "");
    CHECK_EQ(authorization::parse_barear("Bearer: abcd"), "abcd");
  }
}

TEST_SUITE_END();
