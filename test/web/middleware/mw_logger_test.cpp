//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria_test.h>

#include <fitoria/web.hpp>

using namespace fitoria;
using namespace fitoria::web;

TEST_SUITE_BEGIN("[fitoria.web.middleware.logger]");

TEST_CASE("logger middleware")
{
  auto server = http_server::builder()
                    .serve(scope<"/api">()
                               .use(middleware::logger())
                               .serve(route::get<"/get">(
                                   [&]([[maybe_unused]] http_request& req)
                                       -> lazy<http_response> {
                                     co_return http_response(http::status::ok);
                                   })))
                    .build();

  net::sync_wait([&]() -> lazy<void> {
    {
      auto res = co_await server.async_serve_request(
          "/api/get",
          http_request(http::verb::get)
              .set_field(http::field::user_agent, "fitoria"));
      CHECK_EQ(res.status_code(), http::status::ok);
    }
  }());
}

TEST_SUITE_END();
