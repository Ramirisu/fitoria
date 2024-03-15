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

TEST_SUITE_BEGIN("[fitoria.web.share_state]");

TEST_CASE("share_state shares the same instance")
{
  auto server
      = http_server::builder()
            .serve(
                scope<>()
                    .share_state(std::string("original"))
                    .serve(route::get<"/modify">(
                        [](http_request& req) -> net::awaitable<http_response> {
                          auto state = req.state<std::string>();
                          static_assert(std::same_as<decltype(state),
                                                     optional<std::string&>>);
                          CHECK(state);
                          if (state) {
                            CHECK_EQ(*state, std::string("original"));
                            *state = "modifed";
                          }
                          co_return http_response(http::status::ok);
                        }))
                    .serve(route::get<"/check">(
                        [](http_request& req) -> net::awaitable<http_response> {
                          auto state = req.state<std::string>();
                          static_assert(std::same_as<decltype(state),
                                                     optional<std::string&>>);
                          CHECK(state);
                          if (state) {
                            CHECK_EQ(*state, std::string("modifed"));
                          }
                          co_return http_response(http::status::ok);
                        })))
            .build();

  net::sync_wait([&]() -> net::awaitable<void> {
    {
      auto res = co_await server.async_serve_request(
          "/modify", http_request(http::verb::get));
      CHECK_EQ(res.status_code(), http::status::ok);
    }
    {
      auto res = co_await server.async_serve_request(
          "/check", http_request(http::verb::get));
      CHECK_EQ(res.status_code(), http::status::ok);
    }
  }());
}

TEST_CASE("share_state access order on global, scope and route")
{
  struct shared_resource {
    std::string_view value;
  };

  auto server
      = http_server::builder()
            .serve(
                scope<>()
                    .share_state(shared_resource { "global" })
                    .serve(
                        scope<"/api/v1">()
                            .serve(route::get<"/global">(
                                [](http_request& req)
                                    -> net::awaitable<http_response> {
                                  co_return http_response(http::status::ok)
                                      .set_field(http::field::content_type,
                                                 http::fields::content_type::
                                                     plaintext())
                                      .set_body(
                                          req.state<shared_resource>()->value);
                                }))
                            .share_state(shared_resource { "scope" })
                            .serve(route::get<"/scope">([](http_request& req)
                                                            -> net::awaitable<
                                                                http_response> {
                              co_return http_response(http::status::ok)
                                  .set_field(
                                      http::field::content_type,
                                      http::fields::content_type::plaintext())
                                  .set_body(
                                      req.state<shared_resource>()->value);
                            }))
                            .serve(
                                route::get<"/route">([](http_request& req)
                                                         -> net::awaitable<
                                                             http_response> {
                                  co_return http_response(http::status::ok)
                                      .set_field(http::field::content_type,
                                                 http::fields::content_type::
                                                     plaintext())
                                      .set_body(
                                          req.state<shared_resource>()->value);
                                }).share_state(shared_resource { "route" }))))
            .build();

  net::sync_wait([&]() -> net::awaitable<void> {
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
