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
      = http_server::builder(ioc)
            .serve(
                scope<>()
                    .use_state(std::string("original"))
                    .serve(route::get<"/modify">(
                        [](request& req) -> awaitable<response> {
                          auto state = req.state<std::string>();
                          static_assert(std::same_as<decltype(state),
                                                     optional<std::string&>>);
                          CHECK(state);
                          if (state) {
                            CHECK_EQ(*state, std::string("original"));
                            *state = "modifed";
                          }
                          co_return response::ok().build();
                        }))
                    .serve(route::get<"/check">(
                        [](request& req) -> awaitable<response> {
                          auto state = req.state<std::string>();
                          static_assert(std::same_as<decltype(state),
                                                     optional<std::string&>>);
                          CHECK(state);
                          if (state) {
                            CHECK_EQ(*state, std::string("modifed"));
                          }
                          co_return response::ok().build();
                        })))
            .build();

  server.serve_request("/modify",
                       test_request::get().build(),
                       [](test_response res) -> awaitable<void> {
                         CHECK_EQ(res.status_code(), http::status::ok);
                         co_return;
                       });
  server.serve_request("/check",
                       test_request::get().build(),
                       [](test_response res) -> awaitable<void> {
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
      = http_server::builder(ioc)
            .serve(
                scope<>()
                    .use_state(shared_resource { "global" })
                    .serve(
                        scope<"/api/v1">()
                            .serve(route::get<"/global">(
                                [](request& req) -> awaitable<response> {
                                  co_return response::ok()
                                      .set_header(http::field::content_type,
                                                  mime::text_plain())
                                      .set_body(
                                          req.state<shared_resource>()->value);
                                }))
                            .use_state(shared_resource { "scope" })
                            .serve(route::get<"/scope">(
                                [](request& req) -> awaitable<response> {
                                  co_return response::ok()
                                      .set_header(http::field::content_type,
                                                  mime::text_plain())
                                      .set_body(
                                          req.state<shared_resource>()->value);
                                }))
                            .serve(
                                route::get<"/route">(
                                    [](request& req) -> awaitable<response> {
                                      co_return response::ok()
                                          .set_header(http::field::content_type,
                                                      mime::text_plain())
                                          .set_body(req.state<shared_resource>()
                                                        ->value);
                                    })
                                    .use_state(shared_resource { "route" }))))
            .build();

  server.serve_request("/api/v1/global",
                       test_request::get().build(),
                       [](test_response res) -> awaitable<void> {
                         CHECK_EQ(res.status_code(), http::status::ok);
                         CHECK_EQ(co_await res.as_string(), "global");
                       });
  server.serve_request("/api/v1/scope",
                       test_request::get().build(),
                       [](test_response res) -> awaitable<void> {
                         CHECK_EQ(res.status_code(), http::status::ok);
                         CHECK_EQ(co_await res.as_string(), "scope");
                       });
  server.serve_request("/api/v1/route",
                       test_request::get().build(),
                       [](test_response res) -> awaitable<void> {
                         CHECK_EQ(res.status_code(), http::status::ok);
                         CHECK_EQ(co_await res.as_string(), "route");
                       });

  ioc.run();
}

TEST_SUITE_END();
