//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria/test/test.hpp>

#include <fitoria/test/async_readable_chunk_stream.hpp>
#include <fitoria/test/http_server_utils.hpp>

#include <fitoria/web.hpp>

using namespace fitoria;
using namespace fitoria::web;
using namespace fitoria::test;
using fitoria::test::async_readable_chunk_stream;

TEST_SUITE_BEGIN("[fitoria.web.middleware.decompress]");

namespace {

auto get_stream(bool chunked, auto&& range) -> any_async_readable_stream
{
  const auto s = std::span(std::begin(range), std::end(range));
  if (chunked) {
    return async_readable_chunk_stream<8>(s);
  }
  return async_readable_vector_stream(s);
}

}

TEST_CASE("deflate")
{
  const auto plain = std::string_view(
      "abcdefghijklmnopqrstuvwxyz0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ");
  const auto compressed = std::vector<std::uint8_t> {
    0x4b, 0x4c, 0x4a, 0x4e, 0x49, 0x4d, 0x4b, 0xcf, 0xc8, 0xcc, 0xca,
    0xce, 0xc9, 0xcd, 0xcb, 0x2f, 0x28, 0x2c, 0x2a, 0x2e, 0x29, 0x2d,
    0x2b, 0xaf, 0xa8, 0xac, 0x32, 0x30, 0x34, 0x32, 0x36, 0x31, 0x35,
    0x33, 0xb7, 0xb0, 0x74, 0x74, 0x72, 0x76, 0x71, 0x75, 0x73, 0xf7,
    0xf0, 0xf4, 0xf2, 0xf6, 0xf1, 0xf5, 0xf3, 0x0f, 0x08, 0x0c, 0x0a,
    0x0e, 0x09, 0x0d, 0x0b, 0x8f, 0x88, 0x8c, 0x02, 0x00
  };

  auto ioc = net::io_context();
  auto server
      = http_server::builder(ioc)
            .serve(
                route::post<"/">([&](const http::header& header,
                                     std::string body) -> awaitable<response> {
                  CHECK(!header.get(http::field::content_encoding));
                  CHECK_EQ(body, plain);
                  co_return response::ok().build();
                }).use(middleware::decompress()))
            .build();

  struct test_case_t {
    bool chunked;
  };

  const auto test_cases = std::vector<test_case_t> { { false }, { true } };

  for (auto& test_case : test_cases) {
    server.serve_request(
        "/",
        test_request::post()
            .set_header(http::field::content_encoding,
                        http::fields::content_encoding::deflate())
            .set_stream(get_stream(test_case.chunked, compressed)),
        [](auto res) -> awaitable<void> {
          CHECK_EQ(res.status_code(), http::status::ok);
          co_return;
        });
  }

  ioc.run();
}

#if defined(FITORIA_HAS_ZLIB)

TEST_CASE("gzip")
{
  const auto plain = std::string_view(
      "abcdefghijklmnopqrstuvwxyz0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ");
  const auto compressed = std::vector<std::uint8_t> {
    0x1F, 0x8B, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x4B, 0x4C,
    0x4A, 0x4E, 0x49, 0x4D, 0x4B, 0xCF, 0xC8, 0xCC, 0xCA, 0xCE, 0xC9, 0xCD,
    0xCB, 0x2F, 0x28, 0x2C, 0x2A, 0x2E, 0x29, 0x2D, 0x2B, 0xAF, 0xA8, 0xAC,
    0x32, 0x30, 0x34, 0x32, 0x36, 0x31, 0x35, 0x33, 0xB7, 0xB0, 0x74, 0x74,
    0x72, 0x76, 0x71, 0x75, 0x73, 0xF7, 0xF0, 0xF4, 0xF2, 0xF6, 0xF1, 0xF5,
    0xF3, 0x0F, 0x08, 0x0C, 0x0A, 0x0E, 0x09, 0x0D, 0x0B, 0x8F, 0x88, 0x8C,
    0x02, 0x00, 0x32, 0xFA, 0xF8, 0x21, 0x3E, 0x00, 0x00, 0x00
  };

  auto ioc = net::io_context();
  auto server
      = http_server::builder(ioc)
            .serve(
                route::post<"/">([&](const http::header& header,
                                     std::string body) -> awaitable<response> {
                  CHECK(!header.get(http::field::content_encoding));
                  CHECK_EQ(body, plain);
                  co_return response::ok().build();
                }).use(middleware::decompress()))
            .build();

  struct test_case_t {
    bool chunked;
  };

  const auto test_cases = std::vector<test_case_t> { { false }, { true } };

  for (auto& test_case : test_cases) {
    server.serve_request(
        "/",
        test_request::post()
            .set_header(http::field::content_encoding,
                        http::fields::content_encoding::gzip())
            .set_stream(get_stream(test_case.chunked, compressed)),
        [](auto res) -> awaitable<void> {
          CHECK_EQ(res.status_code(), http::status::ok);
          co_return;
        });
  }

  ioc.run();
}

#endif

TEST_CASE("decompress")
{
  const auto plain = std::string_view(
      "abcdefghijklmnopqrstuvwxyz0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ");

  auto ioc = net::io_context();
  auto server
      = http_server::builder(ioc)
            .serve(
                route::post<"/">([&](const http::header& header,
                                     std::string body) -> awaitable<response> {
                  CHECK(!header.get(http::field::content_encoding));
                  CHECK_EQ(body, plain);
                  co_return response::ok().build();
                }).use(middleware::decompress()))
            .build();

  struct test_case_t {
    bool chunked;
  };

  const auto test_cases = std::vector<test_case_t> { { false }, { true } };

  for ([[maybe_unused]] auto& test_case : test_cases) {
#if defined(FITORIA_HAS_ZLIB)
    server.serve_request(
        "/",
        test_request::post()
            .set_header(http::field::content_encoding,
                        fmt::format("{}, {}, {}",
                                    http::fields::content_encoding::deflate(),
                                    http::fields::content_encoding::identity(),
                                    http::fields::content_encoding::gzip()))
            .set_stream(get_stream(
                test_case.chunked,
                std::vector<std::uint8_t> {
                    0x1f, 0x8b, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0a,
                    0x01, 0x40, 0x00, 0xbf, 0xff, 0x4b, 0x4c, 0x4a, 0x4e, 0x49,
                    0x4d, 0x4b, 0xcf, 0xc8, 0xcc, 0xca, 0xce, 0xc9, 0xcd, 0xcb,
                    0x2f, 0x28, 0x2c, 0x2a, 0x2e, 0x29, 0x2d, 0x2b, 0xaf, 0xa8,
                    0xac, 0x32, 0x30, 0x34, 0x32, 0x36, 0x31, 0x35, 0x33, 0xb7,
                    0xb0, 0x74, 0x74, 0x72, 0x76, 0x71, 0x75, 0x73, 0xf7, 0xf0,
                    0xf4, 0xf2, 0xf6, 0xf1, 0xf5, 0xf3, 0x0f, 0x08, 0x0c, 0x0a,
                    0x0e, 0x09, 0x0d, 0x0b, 0x8f, 0x88, 0x8c, 0x02, 0x00, 0xf2,
                    0x92, 0x53, 0x55, 0x40, 0x00, 0x00, 0x00 })),
        [](auto res) -> awaitable<void> {
          CHECK_EQ(res.status_code(), http::status::ok);
          co_return;
        });
    server.serve_request(
        "/",
        test_request::post()
            .set_header(http::field::content_encoding,
                        fmt::format("{}, {}, {}",
                                    http::fields::content_encoding::gzip(),
                                    http::fields::content_encoding::identity(),
                                    http::fields::content_encoding::deflate()))
            .set_stream(get_stream(
                test_case.chunked,
                std::vector<std::uint8_t> {
                    0x93, 0xef, 0xe6, 0x60, 0x00, 0x03, 0x2e, 0x6f, 0x1f, 0x2f,
                    0x3f, 0x4f, 0x5f, 0xef, 0xf3, 0x27, 0xce, 0x9c, 0x3a, 0x77,
                    0xf2, 0xec, 0x69, 0x7d, 0x0d, 0x1d, 0x2d, 0x3d, 0x4d, 0x5d,
                    0xed, 0xf5, 0x2b, 0xd6, 0x18, 0x19, 0x98, 0x18, 0x99, 0x19,
                    0x9a, 0x1a, 0x6f, 0xdf, 0x50, 0x52, 0x52, 0x54, 0x56, 0x58,
                    0x5a, 0xfc, 0xfd, 0xc3, 0x97, 0x4f, 0xdf, 0x3e, 0x7e, 0xfd,
                    0xcc, 0xcf, 0xc1, 0xc3, 0xc5, 0xc7, 0xc9, 0xcb, 0xdd, 0xdf,
                    0xd1, 0xc3, 0xc4, 0x60, 0xf4, 0xeb, 0x87, 0xa2, 0x1d, 0xd0,
                    0x20, 0x00 })),
        [](auto res) -> awaitable<void> {
          CHECK_EQ(res.status_code(), http::status::ok);
          co_return;
        });
#endif

#if defined(FITORIA_HAS_BROTLI)
    server.serve_request(
        "/",
        test_request::post()
            .set_header(http::field::content_encoding,
                        fmt::format("{}, {}, {}",
                                    http::fields::content_encoding::deflate(),
                                    http::fields::content_encoding::identity(),
                                    http::fields::content_encoding::brotli()))
            .set_stream(get_stream(
                test_case.chunked,
                std::vector<std::uint8_t> {
                    0x8b, 0x1f, 0x80, 0x4b, 0x4c, 0x4a, 0x4e, 0x49, 0x4d, 0x4b,
                    0xcf, 0xc8, 0xcc, 0xca, 0xce, 0xc9, 0xcd, 0xcb, 0x2f, 0x28,
                    0x2c, 0x2a, 0x2e, 0x29, 0x2d, 0x2b, 0xaf, 0xa8, 0xac, 0x32,
                    0x30, 0x34, 0x32, 0x36, 0x31, 0x35, 0x33, 0xb7, 0xb0, 0x74,
                    0x74, 0x72, 0x76, 0x71, 0x75, 0x73, 0xf7, 0xf0, 0xf4, 0xf2,
                    0xf6, 0xf1, 0xf5, 0xf3, 0x0f, 0x08, 0x0c, 0x0a, 0x0e, 0x09,
                    0x0d, 0x0b, 0x8f, 0x88, 0x8c, 0x02, 0x00, 0x03 })),
        [](auto res) -> awaitable<void> {
          CHECK_EQ(res.status_code(), http::status::ok);
          co_return;
        });
    server.serve_request(
        "/",
        test_request::post()
            .set_header(http::field::content_encoding,
                        fmt::format("{}, {}, {}",
                                    http::fields::content_encoding::brotli(),
                                    http::fields::content_encoding::identity(),
                                    http::fields::content_encoding::deflate()))
            .set_stream(get_stream(
                test_case.chunked,
                std::vector<std::uint8_t> {
                    0xeb, 0x96, 0x6b, 0x48, 0x4c, 0x4a, 0x4e, 0x49, 0x4d, 0x4b,
                    0xcf, 0xc8, 0xcc, 0xca, 0xce, 0xc9, 0xcd, 0xcb, 0x2f, 0x28,
                    0x2c, 0x2a, 0x2e, 0x29, 0x2d, 0x2b, 0xaf, 0xa8, 0xac, 0x32,
                    0x30, 0x34, 0x32, 0x36, 0x31, 0x35, 0x33, 0xb7, 0xb0, 0x74,
                    0x74, 0x72, 0x76, 0x71, 0x75, 0x73, 0xf7, 0xf0, 0xf4, 0xf2,
                    0xf6, 0xf1, 0xf5, 0xf3, 0x0f, 0x08, 0x0c, 0x0a, 0x0e, 0x09,
                    0x0d, 0x0b, 0x8f, 0x88, 0x8c, 0x62, 0x06, 0x00 })),
        [](auto res) -> awaitable<void> {
          CHECK_EQ(res.status_code(), http::status::ok);
          co_return;
        });
#endif
  }

  ioc.run();
}

TEST_SUITE_END();
