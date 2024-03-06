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

TEST_SUITE_BEGIN("[fitoria.web.state_map]");

TEST_CASE("state_map")
{
  struct shared_resource {
    std::string_view value;
  };

  auto server
      = http_server::builder()
            .serve(
                scope<>()
                    .state(shared_resource { "global" })
                    .serve(
                        scope<"/api/v1">()
                            .serve(route::get<
                                   "/global">([](http_request& req)
                                                  -> lazy<http_response> {
                              co_return http_response(http::status::ok)
                                  .set_field(
                                      http::field::content_type,
                                      http::fields::content_type::plaintext())
                                  .set_body(req.state<const shared_resource&>()
                                                ->value);
                            }))
                            .state(shared_resource { "scope" })
                            .serve(route::get<
                                   "/scope">([](http_request& req)
                                                 -> lazy<http_response> {
                              co_return http_response(http::status::ok)
                                  .set_field(
                                      http::field::content_type,
                                      http::fields::content_type::plaintext())
                                  .set_body(req.state<const shared_resource&>()
                                                ->value);
                            }))
                            .serve(route::get<
                                       "/route">([](http_request& req)
                                                     -> lazy<http_response> {
                                     co_return http_response(http::status::ok)
                                         .set_field(http::field::content_type,
                                                    http::fields::content_type::
                                                        plaintext())
                                         .set_body(
                                             req.state<const shared_resource&>()
                                                 ->value);
                                   }).state(shared_resource { "route" }))))
            .build();

  net::sync_wait([&]() -> lazy<void> {
    {
      auto res = co_await server.async_serve_request(
          "/api/v1/global", http_request(http::verb::get));
      CHECK_EQ(res.status_code(), http::status::ok);
      CHECK_EQ(co_await res.as_string(), "global");
    }
    {
      auto res = co_await server.async_serve_request(
          "/api/v1/scope", http_request(http::verb::get));
      CHECK_EQ(res.status_code(), http::status::ok);
      CHECK_EQ(co_await res.as_string(), "scope");
    }
    {
      auto res = co_await server.async_serve_request(
          "/api/v1/route", http_request(http::verb::get));
      CHECK_EQ(res.status_code(), http::status::ok);
      CHECK_EQ(co_await res.as_string(), "route");
    }
  }());
}

TEST_SUITE_END();