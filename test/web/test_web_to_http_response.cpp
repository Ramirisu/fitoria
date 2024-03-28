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

TEST_SUITE_BEGIN("[fitoria.web.to_http_response]");

TEST_CASE("http_response")
{
  auto server
      = http_server::builder()
            .serve(route::get<"/">(
                [](http_request&) -> net::awaitable<http_response> {
                  co_return http_response(http::status::ok)
                      .set_field(http::field::content_type,
                                 http::fields::content_type::plaintext())
                      .set_body("OK");
                }))
            .build();
  net::sync_wait([&]() -> net::awaitable<void> {
    auto res = co_await server.async_serve_request(
        "/", http_request(http::verb::get));
    CHECK_EQ(res.status_code(), http::status::ok);
    CHECK_EQ(res.fields().get(http::field::content_type),
             http::fields::content_type::plaintext());
    CHECK_EQ(co_await res.as_string(), "OK");
  }());
}

TEST_CASE("std::string")
{
  auto server = http_server::builder()
                    .serve(route::get<"/">(
                        [](http_request&) -> net::awaitable<std::string> {
                          co_return "OK";
                        }))
                    .build();
  net::sync_wait([&]() -> net::awaitable<void> {
    auto res = co_await server.async_serve_request(
        "/", http_request(http::verb::get));
    CHECK_EQ(res.status_code(), http::status::ok);
    CHECK_EQ(res.fields().get(http::field::content_type),
             http::fields::content_type::plaintext());
    CHECK_EQ(co_await res.as_string(), "OK");
  }());
}

TEST_CASE("std::vector<std::byte>")
{
  auto server
      = http_server::builder()
            .serve(route::get<"/">(
                [](http_request&) -> net::awaitable<std::vector<std::byte>> {
                  co_return std::vector<std::byte> { std::byte('O'),
                                                     std::byte('K') };
                }))
            .build();
  net::sync_wait([&]() -> net::awaitable<void> {
    auto res = co_await server.async_serve_request(
        "/", http_request(http::verb::get));
    CHECK_EQ(res.status_code(), http::status::ok);
    CHECK_EQ(res.fields().get(http::field::content_type),
             http::fields::content_type::octet_stream());
    CHECK_EQ(co_await res.as_string(), "OK");
  }());
}

TEST_CASE("std::vector<std::uint8_t>")
{
  auto server
      = http_server::builder()
            .serve(route::get<"/">(
                [](http_request&) -> net::awaitable<std::vector<std::uint8_t>> {
                  co_return std::vector<std::uint8_t> { 0x4f, 0x4b };
                }))
            .build();
  net::sync_wait([&]() -> net::awaitable<void> {
    auto res = co_await server.async_serve_request(
        "/", http_request(http::verb::get));
    CHECK_EQ(res.status_code(), http::status::ok);
    CHECK_EQ(res.fields().get(http::field::content_type),
             http::fields::content_type::octet_stream());
    CHECK_EQ(co_await res.as_string(), "OK");
  }());
}

TEST_CASE("std::variant")
{
  auto server
      = http_server::builder()
            .serve(route::get<"/{text}">(
                [](const path_info& path_info)
                    -> net::awaitable<
                        std::variant<std::string, std::vector<std::uint8_t>>> {
                  if (path_info.get("text") == "yes") {
                    co_return "OK";
                  }
                  co_return std::vector<std::uint8_t> { 0x4f, 0x4b };
                }))
            .build();
  net::sync_wait([&]() -> net::awaitable<void> {
    {
      auto res = co_await server.async_serve_request(
          "/yes", http_request(http::verb::get));
      CHECK_EQ(res.status_code(), http::status::ok);
      CHECK_EQ(res.fields().get(http::field::content_type),
               http::fields::content_type::plaintext());
      CHECK_EQ(co_await res.as_string(), "OK");
    }
    {
      auto res = co_await server.async_serve_request(
          "/no", http_request(http::verb::get));
      CHECK_EQ(res.status_code(), http::status::ok);
      CHECK_EQ(res.fields().get(http::field::content_type),
               http::fields::content_type::octet_stream());
      CHECK_EQ(co_await res.as_string(), "OK");
    }
  }());
}

TEST_SUITE_END();
