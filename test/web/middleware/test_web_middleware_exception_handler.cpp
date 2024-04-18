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

TEST_SUITE_BEGIN("[fitoria.web.middleware.exception_handler]");

#if !FITORIA_NO_EXCEPTIONS

TEST_CASE("exception_handler middleware")
{
  auto ioc = net::io_context();
  auto server
      = http_server_builder(ioc)
            .serve(scope<"/api">()
                       .use(middleware::exception_handler())
                       .serve(route::get<"/">(
                           [&](std::string body) -> awaitable<response> {
                             if (body.ends_with("true")) {
                               throw std::exception();
                             }
                             co_return response::ok().build();
                           })))
            .build();

  server.serve_request("/api/",
                       request(http::verb::get).set_body("throw: false"),
                       [](auto res) -> awaitable<void> {
                         CHECK_EQ(res.status_code(), http::status::ok);
                         co_return;
                       });
  server.serve_request("/api/",
                       request(http::verb::get).set_body("throw: true"),
                       [](auto res) -> awaitable<void> {
                         CHECK_EQ(res.status_code(),
                                  http::status::internal_server_error);
                         co_return;
                       });

  ioc.run();
}

#endif

TEST_SUITE_END();
