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
  auto server
      = http_server::builder()
            .serve(
                scope<"/api">()
                    .use(middleware::exception_handler())
                    .serve(route::get<"/">(
                        [&](std::string body) -> net::awaitable<http_response> {
                          if (body.ends_with("true")) {
                            throw std::exception();
                          }
                          co_return http_response(http::status::ok);
                        })))
            .build();

  net::sync_wait([&]() -> net::awaitable<void> {
    {
      auto res = co_await server.async_serve_request(
          "/api/", http_request(http::verb::get).set_body("throw: false"));
      CHECK_EQ(res.status_code(), http::status::ok);
    }
    {
      auto res = co_await server.async_serve_request(
          "/api/", http_request(http::verb::get).set_body("throw: true"));
      CHECK_EQ(res.status_code(), http::status::internal_server_error);
    }
  }());
}

#endif

TEST_SUITE_END();
