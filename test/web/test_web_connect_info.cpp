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

TEST_SUITE_BEGIN("[fitoria.web.connect_info]");

TEST_CASE("connect_info")
{
  auto ioc = net::io_context();
  auto server
      = http_server::builder(ioc)
            .serve(route::get<"/">([](request& req) -> awaitable<response> {
              CHECK_EQ(req.connection().local(), "localhost");
              CHECK_EQ(req.connection().remote(), "localhost");
              co_return response::ok().build();
            }))
            .build();

  server.serve_request("/",
                       test_request::get().build(),
                       [](test_response res) -> awaitable<void> {
                         CHECK_EQ(res.status_code(), http::status::ok);
                         CHECK_EQ(co_await res.as_string(), "");
                       });

  ioc.run();
}

TEST_SUITE_END();
