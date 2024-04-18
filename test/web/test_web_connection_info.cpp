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

TEST_SUITE_BEGIN("[fitoria.web.connection_info]");

TEST_CASE("connection_info")
{
  auto ioc = net::io_context();
  auto server
      = http_server_builder(ioc)
            .serve(route::get<"/">([](request& req) -> awaitable<response> {
              CHECK_EQ(req.connection().local().address(),
                       net::ip::make_address("127.0.0.1"));
              CHECK_EQ(req.connection().local().port(), 0);
              CHECK_EQ(req.connection().remote().address(),
                       net::ip::make_address("127.0.0.1"));
              CHECK_EQ(req.connection().remote().port(), 0);
              co_return response::ok().build();
            }))
            .build();

  server.serve_request(
      "/", request(http::verb::get), [](auto res) -> awaitable<void> {
        CHECK_EQ(res.status_code(), http::status::ok);
        CHECK(!(co_await res.as_string()));
      });

  ioc.run();
}

TEST_SUITE_END();
