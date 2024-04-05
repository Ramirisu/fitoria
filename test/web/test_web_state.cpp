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

TEST_SUITE_BEGIN("[fitoria.web.state]");

TEST_CASE("state shares the same instance")
{
  auto ioc = net::io_context();
  auto server
      = http_server_builder(ioc)
            .serve(
                scope<>()
                    .state(std::string("original"))
                    .serve(route::get<"/modify">(
                        [](http_request& req) -> awaitable<http_response> {
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
                        [](http_request& req) -> awaitable<http_response> {
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

  server.serve_request("/modify",
                       http_request(http::verb::get),
                       [](auto res) -> awaitable<void> {
                         CHECK_EQ(res.status_code(), http::status::ok);
                         co_return;
                       });
  server.serve_request(
      "/check", http_request(http::verb::get), [](auto res) -> awaitable<void> {
        CHECK_EQ(res.status_code(), http::status::ok);
        co_return;
      });

  ioc.run();
}

TEST_CASE("state access order on global, scope and route")
{
  struct shared_resource {
    std::string_view value;
  };

  auto ioc = net::io_context();
  auto server
      = http_server_builder(ioc)
            .serve(
                scope<>()
                    .state(shared_resource { "global" })
                    .serve(
                        scope<"/api/v1">()
                            .serve(route::get<
                                   "/global">([](http_request& req)
                                                  -> awaitable<http_response> {
                              co_return http_response(http::status::ok)
                                  .set_field(
                                      http::field::content_type,
                                      http::fields::content_type::plaintext())
                                  .set_body(
                                      req.state<shared_resource>()->value);
                            }))
                            .state(shared_resource { "scope" })
                            .serve(route::get<
                                   "/scope">([](http_request& req)
                                                 -> awaitable<http_response> {
                              co_return http_response(http::status::ok)
                                  .set_field(
                                      http::field::content_type,
                                      http::fields::content_type::plaintext())
                                  .set_body(
                                      req.state<shared_resource>()->value);
                            }))
                            .serve(
                                route::get<
                                    "/route">([](http_request& req)
                                                  -> awaitable<http_response> {
                                  co_return http_response(http::status::ok)
                                      .set_field(http::field::content_type,
                                                 http::fields::content_type::
                                                     plaintext())
                                      .set_body(
                                          req.state<shared_resource>()->value);
                                }).state(shared_resource { "route" }))))
            .build();

  server.serve_request("/api/v1/global",
                       http_request(http::verb::get),
                       [](auto res) -> awaitable<void> {
                         CHECK_EQ(res.status_code(), http::status::ok);
                         CHECK_EQ(co_await res.as_string(), "global");
                       });
  server.serve_request("/api/v1/scope",
                       http_request(http::verb::get),
                       [](auto res) -> awaitable<void> {
                         CHECK_EQ(res.status_code(), http::status::ok);
                         CHECK_EQ(co_await res.as_string(), "scope");
                       });
  server.serve_request("/api/v1/route",
                       http_request(http::verb::get),
                       [](auto res) -> awaitable<void> {
                         CHECK_EQ(res.status_code(), http::status::ok);
                         CHECK_EQ(co_await res.as_string(), "route");
                       });

  ioc.run();
}

TEST_SUITE_END();
