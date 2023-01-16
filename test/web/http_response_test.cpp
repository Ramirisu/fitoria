//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria_test.h>

#include <fitoria/web/http_response.hpp>

using namespace fitoria;

TEST_SUITE_BEGIN("http_response");

TEST_CASE("status")
{
  {
    http_response res;
    CHECK_EQ(res.status(), http::status::ok);
    res.set_status(http::status::not_found);
    CHECK_EQ(res.status(), http::status::not_found);
  }
  {
    http_response res(http::status::not_found);
    CHECK_EQ(res.status(), http::status::not_found);
  }
}

TEST_CASE("set_header")
{
  {
    http_response res;
    res.set_header(http::field::content_type, "text/plain");
    CHECK_EQ(res.headers().get(http::field::content_type), "text/plain");
  }
  {
    http_response res;
    res.set_header("Content-Type", "text/plain");
    CHECK_EQ(res.headers().get(http::field::content_type), "text/plain");
  }
}

TEST_SUITE_END();
