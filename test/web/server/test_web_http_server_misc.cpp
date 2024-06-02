//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria/test/test.hpp>

#include <fitoria/test/async_readable_chunk_stream.hpp>
#include <fitoria/test/http_client.hpp>
#include <fitoria/test/http_server_utils.hpp>
#include <fitoria/test/utility.hpp>

#include <fitoria/web.hpp>

using namespace fitoria;
using namespace fitoria::web;
using namespace fitoria::test;

TEST_SUITE_BEGIN("[fitoria.web.http_server]");

TEST_CASE("socket reuse address")
{
  const auto port = generate_port();
  auto ioc = net::io_context();
  auto server = http_server::builder(ioc).build();
  REQUIRE(server.bind(server_ip, port));
#if defined(FITORIA_TARGET_WINDOWS)
  REQUIRE(server.bind(server_ip, port));
#else
  REQUIRE(!server.bind(server_ip, port));
#endif
}

TEST_CASE("duplicate route")
{
  auto ioc = net::io_context();
  CHECK_THROWS_AS(auto server = http_server::builder(ioc).serve(
                      scope()
                          .serve(route::get<"/">([]() -> awaitable<response> {
                            co_return response::ok().build();
                          }))
                          .serve(route::get<"/">([]() -> awaitable<response> {
                            co_return response::ok().build();
                          }))),
                  std::system_error);
}

TEST_CASE("invalid target")
{
  const auto port = generate_port();
  auto ioc = net::io_context();
  auto server = http_server::builder(ioc)
                    .serve(route::get<"/api/v1/users/{user}">(
                        []() -> awaitable<response> {
                          co_return response::ok().build();
                        }))
                    .build();
  REQUIRE(server.bind(server_ip, port));

  net::thread_pool tp(1);
  net::post(tp, [&]() { ioc.run(); });
  scope_exit guard([&]() { ioc.stop(); });
  std::this_thread::sleep_for(server_start_wait_time);

  const auto test_cases = std::vector {
    "/", "/a", "/api", "/api/", "/api/v1", "/api/v1/x", "/api/v1/users/x/y",
  };

  for (auto& test_case : test_cases) {
    net::co_spawn(
        ioc,
        [&]() -> awaitable<void> {
          auto res = co_await http_client()
                         .set_method(http::verb::get)
                         .set_url(to_local_url(
                             boost::urls::scheme::http, port, test_case))
                         .set_header(http::field::connection, "close")
                         .set_plaintext("text")
                         .async_send();
          REQUIRE_EQ(res->status_code(), http::status::not_found);
          REQUIRE_EQ(res->header().get(http::field::content_type),
                     mime::text_plain());
          REQUIRE_EQ(co_await res->as_string(), "request path is not found");
        },
        net::use_future)
        .get();
  }
}

TEST_CASE("expect: 100-continue")
{
  const auto port = generate_port();
  auto ioc = net::io_context();
  auto server = http_server::builder(ioc)
                    .serve(route::post<"/api/v1/post">(
                        [](std::string body) -> awaitable<response> {
                          REQUIRE_EQ(body, "text");
                          co_return response::ok().build();
                        }))
                    .build();
  REQUIRE(server.bind(server_ip, port));

  net::thread_pool tp(1);
  net::post(tp, [&]() { ioc.run(); });
  scope_exit guard([&]() { ioc.stop(); });
  std::this_thread::sleep_for(server_start_wait_time);

  net::co_spawn(
      ioc,
      [&]() -> awaitable<void> {
        auto res = co_await http_client()
                       .set_method(http::verb::post)
                       .set_url(to_local_url(
                           boost::urls::scheme::http, port, "/api/v1/post"))
                       .set_header(http::field::expect, "100-continue")
                       .set_header(http::field::connection, "close")
                       .set_plaintext("text")
                       .async_send();
        REQUIRE_EQ(res->status_code(), http::status::ok);
        REQUIRE_EQ(co_await res->as_string(), "");
      },
      net::use_future)
      .get();
}

TEST_SUITE_END();
