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

#include <fitoria/test/utility.hpp>

#include <fitoria/web.hpp>

#include <fstream>

using namespace fitoria;
using namespace fitoria::web;
using namespace fitoria::test;

TEST_SUITE_BEGIN("[fitoria.web.to_response]");

TEST_CASE("response")
{
  auto ioc = net::io_context();
  auto server
      = http_server::builder(ioc)
            .serve(route::get<"/">([]() -> awaitable<response> {
              co_return response::ok()
                  .set_header(http::field::content_type, mime::text_plain())
                  .set_body("OK");
            }))
            .build();

  server.serve_request("/",
                       test_request::get().build(),
                       [](test_response res) -> awaitable<void> {
                         CHECK_EQ(res.status_code(), http::status::ok);
                         CHECK_EQ(res.header().get(http::field::content_type),
                                  mime::text_plain());
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

  server.serve_request("/",
                       test_request::get().build(),
                       [](test_response res) -> awaitable<void> {
                         CHECK_EQ(res.status_code(), http::status::ok);
                         CHECK_EQ(res.header().get(http::field::content_type),
                                  mime::text_plain());
                         CHECK_EQ(co_await res.as_string(), "OK");
                       });

  ioc.run();
}

TEST_CASE("bytes")
{
  auto ioc = net::io_context();
  auto server = http_server::builder(ioc)
                    .serve(route::get<"/">([]() -> awaitable<bytes> {
                      co_return bytes { std::byte('O'), std::byte('K') };
                    }))
                    .build();

  server.serve_request("/",
                       test_request::get().build(),
                       [](test_response res) -> awaitable<void> {
                         CHECK_EQ(res.status_code(), http::status::ok);
                         CHECK_EQ(res.header().get(http::field::content_type),
                                  mime::application_octet_stream());
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

  server.serve_request("/",
                       test_request::get().build(),
                       [](test_response res) -> awaitable<void> {
                         CHECK_EQ(res.status_code(), http::status::ok);
                         CHECK_EQ(res.header().get(http::field::content_type),
                                  mime::application_octet_stream());
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

  server.serve_request("/yes",
                       test_request::get().build(),
                       [](test_response res) -> awaitable<void> {
                         CHECK_EQ(res.status_code(), http::status::ok);
                         CHECK_EQ(res.header().get(http::field::content_type),
                                  mime::text_plain());
                         CHECK_EQ(co_await res.as_string(), "OK");
                       });
  server.serve_request("/no",
                       test_request::get().build(),
                       [](test_response res) -> awaitable<void> {
                         CHECK_EQ(res.status_code(), http::status::ok);
                         CHECK_EQ(res.header().get(http::field::content_type),
                                  mime::application_octet_stream());
                         CHECK_EQ(co_await res.as_string(), "OK");
                       });

  ioc.run();
}

namespace {

class my_error {
  std::string msg_;

public:
  my_error(std::string msg)
      : msg_(std::move(msg))
  {
  }

  auto message() const noexcept -> const std::string&
  {
    return msg_;
  }

  template <decay_to<my_error> Self>
  friend auto tag_invoke(fitoria::web::to_response_t, Self&& self) -> response
  {
    return response::not_found()
        .set_header(http::field::content_type, mime::text_plain())
        .set_body(self.message());
  }
};

}

TEST_CASE("expected<T, E>")
{
  auto ioc = net::io_context();
  auto server = http_server::builder(ioc)
                    .serve(route::get<"/">(
                        []() -> awaitable<expected<std::string, my_error>> {
                          co_return fitoria::unexpected { my_error(
                              "You will never get anything!") };
                        }))
                    .build();

  server.serve_request("/",
                       test_request::get().build(),
                       [](test_response res) -> awaitable<void> {
                         CHECK_EQ(res.status_code(), http::status::not_found);
                         CHECK_EQ(res.header().get(http::field::content_type),
                                  mime::text_plain());
                         CHECK_EQ(co_await res.as_string(),
                                  "You will never get anything!");
                       });

  ioc.run();
}

#if defined(BOOST_ASIO_HAS_FILE)

TEST_CASE("named_file")
{
  const auto file_path = get_random_temp_file_path();
  const auto data = std::string(1048576, 'a');
  {
    std::ofstream(file_path, std::ios::binary) << data;
  }

  auto ioc = net::io_context();
  auto server
      = http_server::builder(ioc)
            .serve(route::get<"/">([&file_path]() -> awaitable<named_file> {
              auto file = named_file::open_readonly(
                  co_await net::this_coro::executor, file_path);
              co_return std::move(*file);
            }))
            .build();

  server.serve_request("/",
                       test_request::get().build(),
                       [&](test_response res) -> awaitable<void> {
                         CHECK_EQ(res.status_code(), http::status::ok);
                         CHECK_EQ(co_await res.as_string(), data);
                       });

  ioc.run();
}

#endif

TEST_SUITE_END();
