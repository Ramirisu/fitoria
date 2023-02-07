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

TEST_SUITE_BEGIN("web.middleware.deflate");

TEST_CASE("deflate decompress in > out")
{
  const auto in = std::vector<std::uint8_t> {
    0x4b, 0x4c, 0x4a, 0x4e, 0x49, 0x4d, 0x4b, 0xcf, 0xc8, 0xcc, 0xca,
    0xce, 0xc9, 0xcd, 0xcb, 0x2f, 0x28, 0x2c, 0x2a, 0x2e, 0x29, 0x2d,
    0x2b, 0xaf, 0xa8, 0xac, 0x32, 0x30, 0x34, 0x32, 0x36, 0x31, 0x35,
    0x33, 0xb7, 0xb0, 0x74, 0x74, 0x72, 0x76, 0x71, 0x75, 0x73, 0xf7,
    0xf0, 0xf4, 0xf2, 0xf6, 0xf1, 0xf5, 0xf3, 0x0f, 0x08, 0x0c, 0x0a,
    0x0e, 0x09, 0x0d, 0x0b, 0x8f, 0x88, 0x8c, 0x02, 0x00
  };

  auto out = middleware::detail::deflate_decompress<std::string>(
      net::const_buffer(in.data(), in.size()));

  const auto exp = std::string_view(
      "abcdefghijklmnopqrstuvwxyz0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ");
  CHECK_EQ(out, exp);
}

TEST_CASE("deflate decompress in < out")
{
  const auto in = std::vector<std::uint8_t> { 0x4b, 0x4c, 0x1c, 0x05,
                                              0x23, 0x19, 0x00, 0x00 };

  auto out = middleware::detail::deflate_decompress<std::string>(
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

TEST_CASE("deflate decompress w/ empty stream")
{
  CHECK(!middleware::detail::deflate_decompress<std::string>(
      net::const_buffer(nullptr, 0)));
}

TEST_CASE("deflate decompress w/ invalid deflate stream")
{
  const auto in = std::vector<std::uint8_t> { 0x00, 0x01, 0x02, 0x03 };

  CHECK(!middleware::detail::deflate_decompress<std::string>(
      net::const_buffer(in.data(), in.size())));
}

TEST_CASE("deflate compress")
{
  const auto in = std::string_view(
      "abcdefghijklmnopqrstuvwxyz0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ");

  auto intermediate
      = middleware::detail::deflate_compress<std::vector<std::uint8_t>>(
            net::const_buffer(in.data(), in.size()))
            .value();

  auto out = middleware::detail::deflate_decompress<std::string>(
      net::const_buffer(intermediate.data(), intermediate.size()));

  CHECK_EQ(out, in);
}

TEST_CASE("deflate compress w/ empty stream")
{
  const auto in = std::string_view();

  CHECK(!middleware::detail::deflate_compress<std::vector<std::uint8_t>>(
      net::const_buffer(in.data(), in.size())));
}

TEST_CASE("deflate middleware")
{
  const auto in = std::string_view(
      "abcdefghijklmnopqrstuvwxyz0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ");

  auto server
      = http_server::builder()
            .route(
                route::GET(
                    "/get/{no_compression}",
                    [&](http_request& req) -> net::awaitable<http_response> {
                      CHECK(!req.fields().get(http::field::content_encoding));
                      CHECK_EQ(*req.fields().get(http::field::content_length),
                               std::to_string(in.size()));
                      CHECK_EQ(req.body(), in);

                      auto res = http_response(http::status::ok)
                                     .set_body(req.body());
                      if (req.route_params().get("no_compression") == "yes") {
                        res.set_field(
                            http::field::content_encoding,
                            http::fields::content_encoding::identity());
                      }
                      co_return res;
                    })
                    .use(middleware::deflate()))
            .build();
  {
    auto res = server.serve_http_request(
        "/get/no",
        http_request(http::verb::get)
            .set_field(http::field::content_encoding, "deflate")
            .set_field(http::field::accept_encoding, "deflate")
            .set_body(middleware::detail::deflate_compress<std::string>(
                          net::const_buffer(in.data(), in.size()))
                          .value())
            .prepare_payload());
    CHECK_EQ(res.status_code(), http::status::ok);
    CHECK_EQ(res.fields().get(http::field::content_encoding), "deflate");
    CHECK_EQ(res.body(),
             middleware::detail::deflate_compress<std::string>(
                 net::const_buffer(in.data(), in.size())));
  }
  {
    auto res = server.serve_http_request(
        "/get/yes",
        http_request(http::verb::get)
            .set_field(http::field::content_encoding, "deflate")
            .set_field(http::field::accept_encoding, "deflate")
            .set_body(middleware::detail::deflate_compress<std::string>(
                          net::const_buffer(in.data(), in.size()))
                          .value())
            .prepare_payload());
    CHECK_EQ(res.status_code(), http::status::ok);
    CHECK_EQ(res.fields().get(http::field::content_encoding),
             http::fields::content_encoding::identity());
    CHECK_EQ(res.body(), in);
  }
}

TEST_CASE("deflate middleware: header vary")
{
  auto server
      = http_server::builder()
            .route(scope("/api")
                       .use(middleware::deflate())
                       .GET("/get",
                            [&]([[maybe_unused]] http_request& req)
                                -> net::awaitable<http_response> {
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
            .set_field(http::field::accept_encoding, "deflate")
            .set_body(test_case.input)
            .prepare_payload());
    CHECK_EQ(res.status_code(), http::status::ok);
    CHECK_EQ(res.fields().get(http::field::vary), test_case.expected);
  }
}

TEST_SUITE_END();
