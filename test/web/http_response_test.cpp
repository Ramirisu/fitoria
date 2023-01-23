//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria_test.h>

#include <fitoria/web/http_response.hpp>

using namespace fitoria;

TEST_SUITE_BEGIN("web.http_response");

TEST_CASE("status_code")
{
  {
    http_response res;
    CHECK_EQ(res.status_code(), http::status::ok);
    res.set_status_code(http::status::not_found);
    CHECK_EQ(res.status_code(), http::status::not_found);
  }
  {
    http_response res(http::status::not_found);
    CHECK_EQ(res.status_code(), http::status::not_found);
  }
}

TEST_CASE("set_header")
{
  {
    http_response res;
    res.set_header(http::field::content_type,
                   http::fields::content_type::plaintext());
    CHECK_EQ(res.headers().get(http::field::content_type),
             http::fields::content_type::plaintext());
  }
  {
    http_response res;
    res.set_header("Content-Type", http::fields::content_type::plaintext());
    CHECK_EQ(res.headers().get(http::field::content_type),
             http::fields::content_type::plaintext());
  }
}

TEST_SUITE_END();
