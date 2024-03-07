//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria_test.h>

#if defined(FITORIA_HAS_ZLIB)

#include <fitoria_test_utils.h>

#include <fitoria/web.hpp>

using namespace fitoria;
using namespace fitoria::web;

TEST_SUITE_BEGIN("[fitoria.web.middleware.gzip]");

TEST_CASE("async_gzip_inflate_stream: in > out")
{
  net::sync_wait([]() -> net::awaitable<void> {
    const auto in = std::vector<std::uint8_t> {
      0x1F, 0x8B, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x4B, 0x4C,
      0x4A, 0x4E, 0x49, 0x4D, 0x4B, 0xCF, 0xC8, 0xCC, 0xCA, 0xCE, 0xC9, 0xCD,
      0xCB, 0x2F, 0x28, 0x2C, 0x2A, 0x2E, 0x29, 0x2D, 0x2B, 0xAF, 0xA8, 0xAC,
      0x32, 0x30, 0x34, 0x32, 0x36, 0x31, 0x35, 0x33, 0xB7, 0xB0, 0x74, 0x74,
      0x72, 0x76, 0x71, 0x75, 0x73, 0xF7, 0xF0, 0xF4, 0xF2, 0xF6, 0xF1, 0xF5,
      0xF3, 0x0F, 0x08, 0x0C, 0x0A, 0x0E, 0x09, 0x0D, 0x0B, 0x8F, 0x88, 0x8C,
      0x02, 0x00, 0x32, 0xFA, 0xF8, 0x21, 0x3E, 0x00, 0x00, 0x00
    };

    auto out = co_await async_read_all_as<std::string>(
        middleware::detail::async_gzip_inflate_stream(
            async_readable_vector_stream(std::span(in.begin(), in.size()))));

    const auto exp = std::string_view(
        "abcdefghijklmnopqrstuvwxyz0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ");
    CHECK_EQ(out, exp);
  });
}

TEST_CASE("async_gzip_inflate_stream: in < out")
{
  net::sync_wait([]() -> net::awaitable<void> {
    const auto in
        = std::vector<std::uint8_t> { 0x1f, 0x8b, 0x08, 0x00, 0x00, 0x00, 0x00,
                                      0x00, 0x00, 0x0a, 0x4b, 0x4c, 0x1c, 0x05,
                                      0x23, 0x19, 0x00, 0x00, 0x51, 0x70, 0x51,
                                      0xf9, 0x00, 0x02, 0x00, 0x00 };

    auto out = co_await async_read_all_as<std::string>(
        middleware::detail::async_gzip_inflate_stream(
            async_readable_vector_stream(std::span(in.begin(), in.size()))));

    const auto exp = std::string(512, 'a');
    CHECK_EQ(out, exp);
  });
}

TEST_CASE("async_gzip_inflate_stream: eof stream")
{
  net::sync_wait([]() -> net::awaitable<void> {
    const auto in = std::vector<std::uint8_t> {};

    auto out = co_await async_read_all_as<std::string>(
        middleware::detail::async_gzip_inflate_stream(
            async_readable_vector_stream::eof()));
    CHECK(!out);
  });
}

TEST_CASE("async_gzip_inflate_stream: empty stream")
{
  net::sync_wait([]() -> net::awaitable<void> {
    const auto in = std::vector<std::uint8_t> {};

    auto out = co_await async_read_all_as<std::string>(
        middleware::detail::async_gzip_inflate_stream(
            async_readable_vector_stream::empty()));
    CHECK(out);
    CHECK(!*out);
  });
}

TEST_CASE("async_gzip_inflate_stream: invalid gzip stream")
{
  net::sync_wait([]() -> net::awaitable<void> {
    const auto in = std::vector<std::uint8_t> { 0x00, 0x01, 0x02, 0x03 };

    auto out = co_await async_read_all_as<std::string>(
        middleware::detail::async_gzip_inflate_stream(
            async_readable_vector_stream(std::span(in.begin(), in.size()))));
    CHECK(out);
    CHECK(!*out);
  });
}

TEST_CASE("async_gzip_deflate_stream")
{
  net::sync_wait([]() -> net::awaitable<void> {
    const auto in = std::string_view(
        "abcdefghijklmnopqrstuvwxyz0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ");

    auto out = co_await async_read_all_as<std::string>(
        middleware::detail::async_gzip_inflate_stream(
            middleware::detail::async_gzip_deflate_stream(
                async_readable_vector_stream(
                    std::span(in.begin(), in.size())))));
    CHECK_EQ(out, in);
  });
}

TEST_CASE("async_gzip_deflate_stream: eof stream")
{
  net::sync_wait([]() -> net::awaitable<void> {
    auto out = co_await async_read_all_as<std::vector<std::uint8_t>>(
        middleware::detail::async_gzip_deflate_stream(
            async_readable_vector_stream::eof()));
    CHECK(!out);
  });
}

TEST_CASE("async_gzip_deflate_stream: empty stream")
{
  net::sync_wait([]() -> net::awaitable<void> {
    auto out = co_await async_read_all_as<std::vector<std::uint8_t>>(
        middleware::detail::async_gzip_deflate_stream(
            async_readable_vector_stream::empty()));
    CHECK(out);
    CHECK(!*out);
  });
}

TEST_CASE("gzip middleware")
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

  auto server
      = http_server::builder()
            .serve(route::get<"/get/{set_identity}/{chunked}/{empty_body}">(
                       [&](const http_request& req,
                           const route_params& params,
                           const http_fields& fields,
                           std::string body) -> net::awaitable<http_response> {
                         CHECK(!fields.get(http::field::content_encoding));
                         if (req.body().is_chunked()) {
                           CHECK_EQ(fields.get(http::field::content_length),
                                    nullopt);
                         } else {
                           CHECK_EQ(fields.get(http::field::content_length),
                                    std::to_string(plain.size()));
                         }
                         CHECK_EQ(body, plain);

                         auto res = http_response(http::status::ok);
                         if (params.get("set_identity") == "true") {
                           res.set_field(
                               http::field::content_encoding,
                               http::fields::content_encoding::identity());
                         }
                         if (params.get("empty_body") == "true") {
                         } else {
                           if (params.get("chunked") == "true") {
                             res.set_stream(
                                 test_async_readable_chunk_stream<5>(plain));
                           } else {
                             res.set_body(plain);
                           }
                         }
                         co_return res;
                       })
                       .use(middleware::gzip()))
            .build();

  struct test_case_t {
    bool send_chunked;
    bool identity;
    bool chunked;
  };

  const auto test_cases = std::vector<test_case_t> {
    { true, true, true },   { true, true, false },   { true, false, true },
    { true, false, false }, { false, true, true },   { false, true, false },
    { false, false, true }, { false, false, false },
  };

  net::sync_wait([&]() -> net::awaitable<void> {
    for (auto& test_case : test_cases) {
      auto res = co_await server.async_serve_request(
          fmt::format(
              "/get/{}/{}/false", test_case.identity, test_case.chunked),
          http_request(http::verb::get)
              .set_field(http::field::content_encoding,
                         http::fields::content_encoding::gzip())
              .set_field(http::field::accept_encoding,
                         http::fields::content_encoding::gzip())
              .set_stream([&]() -> any_async_readable_stream {
                auto s = std::span(compressed.data(), compressed.size());
                if (test_case.send_chunked) {
                  return test_async_readable_chunk_stream<5>(s);
                }
                return async_readable_vector_stream(s);
              }()));
      CHECK_EQ(res.status_code(), http::status::ok);
      if (test_case.identity) {
        CHECK_EQ(res.fields().get(http::field::content_encoding),
                 http::fields::content_encoding::identity());
        CHECK_EQ(co_await res.as_string(), plain);
      } else {
        CHECK_EQ(res.fields().get(http::field::content_encoding),
                 http::fields::content_encoding::gzip());
        CHECK_EQ(co_await async_read_all_as<std::string>(
                     middleware::detail::async_gzip_inflate_stream(
                         std::move(res.body()))),
                 plain);
      }
    }
    {
      auto res = co_await server.async_serve_request(
          "/get/false/false/true",
          http_request(http::verb::get)
              .set_field(http::field::content_encoding,
                         http::fields::content_encoding::gzip())
              .set_field(http::field::accept_encoding,
                         http::fields::content_encoding::gzip())
              .set_stream(async_readable_vector_stream(
                  std::span(compressed.data(), compressed.size()))));
      CHECK_EQ(res.status_code(), http::status::ok);
      CHECK(!res.fields().get(http::field::content_encoding));
      CHECK_EQ(co_await res.as_string(), "");
    }
  });
}

TEST_CASE("gzip middleware: header vary")
{
  auto server
      = http_server::builder()
            .serve(
                route::get<"/get">([&](std::string body)
                                       -> net::awaitable<http_response> {
                  auto res
                      = http_response(http::status::ok).set_body("hello world");
                  if (!body.empty()) {
                    res.set_field(http::field::vary, body);
                  }
                  co_return res;
                }).use(middleware::gzip()))
            .build();

  struct test_case_t {
    std::string input;
    std::string expected;
  };

  const auto test_cases = std::vector<test_case_t> {
    { "", "Content-Encoding" },
    { "*", "*" },
    { "User-Agent", "User-Agent, Content-Encoding" },
  };

  for (auto& test_case : test_cases) {
    net::sync_wait([&]() -> net::awaitable<void> {
      auto res = co_await server.async_serve_request(
          "/get",
          http_request(http::verb::get)
              .set_field(http::field::accept_encoding,
                         http::fields::content_encoding::gzip())
              .set_body(test_case.input));
      CHECK_EQ(res.status_code(), http::status::ok);
      CHECK_EQ(res.fields().get(http::field::vary), test_case.expected);
    });
  }
}

TEST_SUITE_END();

#endif
