//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria/test/test.hpp>

#include <fitoria/web/http/status_code.hpp>

using namespace fitoria::web::http;
using namespace fitoria;

TEST_SUITE_BEGIN("[fitoria.web.http.status_code]");

TEST_CASE("value")
{
  {
    status_code sc(status::ok);
    CHECK_EQ(sc.value(), status::ok);
    CHECK_EQ(sc, status::ok);
    CHECK_NE(sc, status::not_found);
  }
  {
    status_code sc(200);
    CHECK_EQ(sc.value(), status::ok);
    CHECK_EQ(sc, status::ok);
    CHECK_NE(sc, status::not_found);
  }
}

TEST_CASE("category")
{
  CHECK_EQ(status_code(status::continue_).category(),
           status_class::informational);
  CHECK_EQ(status_code(status::ok).category(), status_class::successful);
  CHECK_EQ(status_code(status::permanent_redirect).category(),
           status_class::redirection);
  CHECK_EQ(status_code(status::bad_request).category(),
           status_class::client_error);
  CHECK_EQ(status_code(status::internal_server_error).category(),
           status_class::server_error);
  CHECK_EQ(status_code(status::unknown).category(), status_class::unknown);
}

TEST_CASE("match")
{
  CHECK_EQ(status_code(status::ok).match({ status::ok, status::accepted }),
           status::ok);
  CHECK_EQ(
      status_code(status::bad_request).match({ status::ok, status::accepted }),
      nullopt);
}

TEST_CASE("format")
{
  CHECK_EQ(fmt::format("{}", status_code(status::ok)), "200");
}

TEST_SUITE_END();
