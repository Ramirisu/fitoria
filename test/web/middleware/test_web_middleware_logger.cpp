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
      = http_server::builder(ioc)
            .serve(scope()
                       .use(middleware::logger())
                       .serve(route::get<"/">([&]() -> awaitable<response> {
                         co_return response::ok().build();
                       })))
            .build();

  server.serve_request("/",
                       test_request::get()
                           .set_header(http::field::user_agent, "fitoria")
                           .build(),
                       [](test_response res) -> awaitable<void> {
                         CHECK_EQ(res.status(), http::status::ok);
                         co_return;
                       });

  ioc.run();
}

TEST_SUITE_END();
