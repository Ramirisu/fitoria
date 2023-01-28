//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria_test.h>

#include <fitoria/web.hpp>

using namespace fitoria;

TEST_SUITE_BEGIN("web.middleware.logger");

TEST_CASE("logger middleware")
{
  auto server
      = http_server::builder()
            .route(scope("/api")
                       .use(middleware::logger())
                       .route(http::verb::get, "/get",
                              [&]([[maybe_unused]] http_request& req)
                                  -> net::awaitable<http_response> {
                                co_return http_response(http::status::ok);
                              }))
            .build();

  auto res = server.serve_http_request(
      "/api/get",
      http_request()
          .set_method(http::verb::get)
          .set_field(http::field::user_agent, "fitoria"));
  CHECK_EQ(res.status_code(), http::status::ok);
}

TEST_SUITE_END();
