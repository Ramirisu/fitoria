//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria/test/test.hpp>

#if defined(FITORIA_HAS_LIBURING)
#define BOOST_ASIO_HAS_IO_URING
#endif

#include <fitoria/web.hpp>

#include <fstream>

using namespace fitoria;
using namespace fitoria::web;

TEST_SUITE_BEGIN("[fitoria.web.to_http_response]");

TEST_CASE("response")
{
  auto ioc = net::io_context();
  auto server = http_server::builder(ioc)
                    .serve(route::get<"/">([]() -> awaitable<response> {
                      co_return response::ok()
                          .set_header(http::field::content_type,
                                      http::fields::content_type::plaintext())
                          .set_body("OK");
                    }))
                    .build();

  server.serve_request(
      "/", request(http::verb::get), [](auto res) -> awaitable<void> {
        CHECK_EQ(res.status_code(), http::status::ok);
        CHECK_EQ(res.header().get(http::field::content_type),
                 http::fields::content_type::plaintext());
        CHECK_EQ(co_await res.as_string(), "OK");
      });

  ioc.run();
}

TEST_CASE("std::string")
{
  auto ioc = net::io_context();
  auto server = http_server::builder(ioc)
                    .serve(route::get<"/">(
                        []() -> awaitable<std::string> { co_return "OK"; }))
                    .build();

  server.serve_request(
      "/", request(http::verb::get), [](auto res) -> awaitable<void> {
        CHECK_EQ(res.status_code(), http::status::ok);
        CHECK_EQ(res.header().get(http::field::content_type),
                 http::fields::content_type::plaintext());
        CHECK_EQ(co_await res.as_string(), "OK");
      });

  ioc.run();
}

TEST_CASE("std::vector<std::byte>")
{
  auto ioc = net::io_context();
  auto server
      = http_server::builder(ioc)
            .serve(route::get<"/">([]() -> awaitable<std::vector<std::byte>> {
              co_return std::vector<std::byte> { std::byte('O'),
                                                 std::byte('K') };
            }))
            .build();

  server.serve_request(
      "/", request(http::verb::get), [](auto res) -> awaitable<void> {
        CHECK_EQ(res.status_code(), http::status::ok);
        CHECK_EQ(res.header().get(http::field::content_type),
                 http::fields::content_type::octet_stream());
        CHECK_EQ(co_await res.as_string(), "OK");
      });

  ioc.run();
}

TEST_CASE("std::vector<std::uint8_t>")
{
  auto ioc = net::io_context();
  auto server = http_server::builder(ioc)
                    .serve(route::get<"/">(
                        []() -> awaitable<std::vector<std::uint8_t>> {
                          co_return std::vector<std::uint8_t> { 0x4f, 0x4b };
                        }))
                    .build();

  server.serve_request(
      "/", request(http::verb::get), [](auto res) -> awaitable<void> {
        CHECK_EQ(res.status_code(), http::status::ok);
        CHECK_EQ(res.header().get(http::field::content_type),
                 http::fields::content_type::octet_stream());
        CHECK_EQ(co_await res.as_string(), "OK");
      });

  ioc.run();
}

TEST_CASE("std::variant")
{
  auto ioc = net::io_context();
  auto server
      = http_server::builder(ioc)
            .serve(route::get<"/{text}">(
                [](const path_info& path_info)
                    -> awaitable<
                        std::variant<std::string, std::vector<std::uint8_t>>> {
                  if (path_info.get("text") == "yes") {
                    co_return "OK";
                  }
                  co_return std::vector<std::uint8_t> { 0x4f, 0x4b };
                }))
            .build();

  server.serve_request(
      "/yes", request(http::verb::get), [](auto res) -> awaitable<void> {
        CHECK_EQ(res.status_code(), http::status::ok);
        CHECK_EQ(res.header().get(http::field::content_type),
                 http::fields::content_type::plaintext());
        CHECK_EQ(co_await res.as_string(), "OK");
      });
  server.serve_request(
      "/no", request(http::verb::get), [](auto res) -> awaitable<void> {
        CHECK_EQ(res.status_code(), http::status::ok);
        CHECK_EQ(res.header().get(http::field::content_type),
                 http::fields::content_type::octet_stream());
        CHECK_EQ(co_await res.as_string(), "OK");
      });

  ioc.run();
}

#if defined(BOOST_ASIO_HAS_FILE)

TEST_CASE("stream_file")
{
  const auto data = std::string(1048576, 'a');
  {
    std::ofstream("test_web_to_http_response.stream_file.txt", std::ios::binary)
        << data;
  }

  auto ioc = net::io_context();
  auto server = http_server::builder(ioc)
                    .serve(route::get<"/">([]() -> awaitable<stream_file> {
                      auto file = co_await stream_file::async_open_readonly(
                          "test_web_to_http_response.stream_file.txt");
                      co_return std::move(*file);
                    }))
                    .build();

  server.serve_request(
      "/", request(http::verb::get), [&](auto res) -> awaitable<void> {
        CHECK_EQ(res.status_code(), http::status::ok);
        CHECK_EQ(co_await res.as_string(), data);
      });

  ioc.run();
}

#endif

TEST_SUITE_END();
