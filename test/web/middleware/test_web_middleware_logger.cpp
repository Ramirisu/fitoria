//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria/test/test.hpp>

#include <fitoria/web.hpp>

using namespace fitoria;
using namespace fitoria::web;

TEST_SUITE_BEGIN("[fitoria.web.middleware.logger]");

TEST_CASE("logger middleware")
{
  auto ioc = net::io_context();
  auto server
      = http_server_builder(ioc)
            .serve(
                scope()
                    .use(middleware::logger())
                    .serve(route::get<"/">([&]() -> awaitable<http_response> {
                      co_return http_response::ok().build();
                    })))
            .build();

  server.serve_request(
      "/",
      request(http::verb::get).set_field(http::field::user_agent, "fitoria"),
      [](auto res) -> awaitable<void> {
        CHECK_EQ(res.status_code(), http::status::ok);
        co_return;
      });

  ioc.run();
}

TEST_SUITE_END();
