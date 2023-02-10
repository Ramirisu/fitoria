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

TEST_SUITE_BEGIN("web.middleware.gzip");

#if defined(FITORIA_HAS_ZLIB)

TEST_CASE("gzip decompress in > out")
{
  const auto in = std::vector<std::uint8_t> {
    0x1F, 0x8B, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x4B, 0x4C,
    0x4A, 0x4E, 0x49, 0x4D, 0x4B, 0xCF, 0xC8, 0xCC, 0xCA, 0xCE, 0xC9, 0xCD,
    0xCB, 0x2F, 0x28, 0x2C, 0x2A, 0x2E, 0x29, 0x2D, 0x2B, 0xAF, 0xA8, 0xAC,
    0x32, 0x30, 0x34, 0x32, 0x36, 0x31, 0x35, 0x33, 0xB7, 0xB0, 0x74, 0x74,
    0x72, 0x76, 0x71, 0x75, 0x73, 0xF7, 0xF0, 0xF4, 0xF2, 0xF6, 0xF1, 0xF5,
    0xF3, 0x0F, 0x08, 0x0C, 0x0A, 0x0E, 0x09, 0x0D, 0x0B, 0x8F, 0x88, 0x8C,
    0x02, 0x00, 0x32, 0xFA, 0xF8, 0x21, 0x3E, 0x00, 0x00, 0x00
  };

  auto out = middleware::detail::gzip_decompress<std::string>(
      net::const_buffer(in.data(), in.size()));

  const auto exp = std::string_view(
      "abcdefghijklmnopqrstuvwxyz0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ");
  CHECK_EQ(out, exp);
}

TEST_CASE("gzip decompress in < out")
{
  const auto in
      = std::vector<std::uint8_t> { 0x1f, 0x8b, 0x08, 0x00, 0x00, 0x00, 0x00,
                                    0x00, 0x00, 0x0a, 0x4b, 0x4c, 0x1c, 0x05,
                                    0x23, 0x19, 0x00, 0x00, 0x51, 0x70, 0x51,
                                    0xf9, 0x00, 0x02, 0x00, 0x00 };

  auto out = middleware::detail::gzip_decompress<std::string>(
      net::const_buffer(in.data(), in.size()));

  const auto exp = std::string_view(
      "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
      "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
      "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
      "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
      "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
      "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
      "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
      "aaaaaaaa");
  CHECK_EQ(out, exp);
}

TEST_CASE("gzip decompress w/ empty stream")
{
  CHECK(!middleware::detail::gzip_decompress<std::string>(
      net::const_buffer(nullptr, 0)));
}

TEST_CASE("gzip decompress w/ invalid gzip stream")
{
  const auto in = std::vector<std::uint8_t> { 0x00, 0x01, 0x02, 0x03 };

  CHECK(!middleware::detail::gzip_decompress<std::string>(
      net::const_buffer(in.data(), in.size())));
}

TEST_CASE("gzip compress")
{
  const auto in = std::string_view(
      "abcdefghijklmnopqrstuvwxyz0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ");

  auto intermediate
      = middleware::detail::gzip_compress<std::vector<std::uint8_t>>(
            net::const_buffer(in.data(), in.size()))
            .value();

  auto out = middleware::detail::gzip_decompress<std::string>(
      net::const_buffer(intermediate.data(), intermediate.size()));

  CHECK_EQ(out, in);
}

TEST_CASE("gzip compress w/ empty stream")
{
  const auto in = std::string_view();

  CHECK(!middleware::detail::gzip_compress<std::vector<std::uint8_t>>(
      net::const_buffer(in.data(), in.size())));
}

TEST_CASE("gzip middleware")
{
  const auto in = std::string_view(
      "abcdefghijklmnopqrstuvwxyz0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ");

  auto server
      = http_server::builder()
            .route(
                route::GET(
                    "/get/{no_compression}",
                    [&](http_request& req) -> lazy<http_response> {
                      CHECK(!req.fields().get(http::field::content_encoding));
                      CHECK_EQ(*req.fields().get(http::field::content_length),
                               std::to_string(in.size()));
                      CHECK_EQ(req.body(), in);

                      auto res = http_response(http::status::ok)
                                     .set_body(req.body());
                      if (req.params().get("no_compression") == "yes") {
                        res.set_field(
                            http::field::content_encoding,
                            http::fields::content_encoding::identity());
                      }
                      co_return res;
                    })
                    .use(middleware::gzip()))
            .build();
  {
    auto res = server.serve_http_request(
        "/get/no",
        http_request(http::verb::get)
            .set_field(http::field::content_encoding, "gzip")
            .set_field(http::field::accept_encoding, "gzip")
            .set_body(middleware::detail::gzip_compress<std::string>(
                          net::const_buffer(in.data(), in.size()))
                          .value())
            .prepare_payload());
    CHECK_EQ(res.status_code(), http::status::ok);
    CHECK_EQ(res.fields().get(http::field::content_encoding), "gzip");
    CHECK_EQ(res.body(),
             middleware::detail::gzip_compress<std::string>(
                 net::const_buffer(in.data(), in.size())));
  }
  {
    auto res = server.serve_http_request(
        "/get/yes",
        http_request(http::verb::get)
            .set_field(http::field::content_encoding, "gzip")
            .set_field(http::field::accept_encoding, "gzip")
            .set_body(middleware::detail::gzip_compress<std::string>(
                          net::const_buffer(in.data(), in.size()))
                          .value())
            .prepare_payload());
    CHECK_EQ(res.status_code(), http::status::ok);
    CHECK_EQ(res.fields().get(http::field::content_encoding),
             http::fields::content_encoding::identity());
    CHECK_EQ(res.body(), in);
  }
}

TEST_CASE("gzip middleware: header vary")
{
  auto server
      = http_server::builder()
            .route(scope("/api")
                       .use(middleware::gzip())
                       .GET("/get",
                            [&]([[maybe_unused]] http_request& req)
                                -> lazy<http_response> {
                              auto res = http_response(http::status::ok)
                                             .set_body("hello world");
                              if (!req.body().empty()) {
                                res.set_field(http::field::vary, req.body());
                              }
                              co_return res;
                            }))
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
    auto res = server.serve_http_request(
        "/api/get",
        http_request(http::verb::get)
            .set_field(http::field::accept_encoding, "gzip")
            .set_body(test_case.input)
            .prepare_payload());
    CHECK_EQ(res.status_code(), http::status::ok);
    CHECK_EQ(res.fields().get(http::field::vary), test_case.expected);
  }
}

#endif

TEST_SUITE_END();
