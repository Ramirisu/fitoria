//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria_test.h>

#include <fitoria/web.hpp>

using namespace fitoria;

TEST_SUITE_BEGIN("http_server.middleware.exception_handler");

TEST_CASE("exception_handler middleware")
{
  auto server
      = http_server::builder()
            .route(scope("/api")
                       .use(middleware::exception_handler())
                       .route(http::verb::get, "/get",
                              [&](http_request& req)
                                  -> net::awaitable<http_response> {
                                if (req.body().ends_with("true")) {
                                  throw std::exception();
                                }
                                co_return http_response(http::status::ok);
                              }))
            .build();
  {
    auto res = server.serve_http_request(
        http::verb::get, "/api/get", http_request().set_body("throw: false"));
    CHECK_EQ(res.status_code(), http::status::ok);
  }
  {
    auto res = server.serve_http_request(
        http::verb::get, "/api/get", http_request().set_body("throw: true"));
    CHECK_EQ(res.status_code(), http::status::internal_server_error);
  }
}

TEST_SUITE_END();
