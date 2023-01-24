//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria_test.h>

#include <fitoria/web.hpp>

using namespace fitoria;

TEST_SUITE_BEGIN("web.middleware.zlib");

#if defined(FITORIA_HAS_ZLIB)

TEST_CASE("zlib decompress")
{
  const auto in = std::vector<std::uint8_t> {
    0x4b, 0x4c, 0x4a, 0x4e, 0x49, 0x4d, 0x4b, 0xcf, 0xc8, 0xcc, 0xca,
    0xce, 0xc9, 0xcd, 0xcb, 0x2f, 0x28, 0x2c, 0x2a, 0x2e, 0x29, 0x2d,
    0x2b, 0xaf, 0xa8, 0xac, 0x32, 0x30, 0x34, 0x32, 0x36, 0x31, 0x35,
    0x33, 0xb7, 0xb0, 0x74, 0x74, 0x72, 0x76, 0x71, 0x75, 0x73, 0xf7,
    0xf0, 0xf4, 0xf2, 0xf6, 0xf1, 0xf5, 0xf3, 0x0f, 0x08, 0x0c, 0x0a,
    0x0e, 0x09, 0x0d, 0x0b, 0x8f, 0x88, 0x8c, 0x02, 0x00
  };

  auto out = middleware::zlib::decompress<std::string>(
      net::const_buffer(in.data(), in.size()));

  const auto exp = std::string_view(
      "abcdefghijklmnopqrstuvwxyz0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ");
  CHECK_EQ(out, exp);
}

TEST_CASE("zlib compress")
{
  const auto in = std::string_view(
      "abcdefghijklmnopqrstuvwxyz0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ");

  auto intermediate = middleware::zlib::compress<std::vector<std::uint8_t>>(
                          net::const_buffer(in.data(), in.size()))
                          .value();

  auto out = middleware::zlib::decompress<std::string>(
      net::const_buffer(intermediate.data(), intermediate.size()));

  CHECK_EQ(out, in);
}

TEST_CASE("zlib middleware")
{
  const auto in = std::string_view(
      "abcdefghijklmnopqrstuvwxyz0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ");

  auto server
      = http_server::builder()
            .route(scope("/api")
                       .use(middleware::zlib())
                       .route(http::verb::get, "/get",
                              [&]([[maybe_unused]] http_request& req)
                                  -> net::awaitable<http_response> {
                                CHECK(!req.headers().get(
                                    http::field::content_encoding));
                                CHECK_EQ(*req.headers().get(
                                             http::field::content_length),
                                         std::to_string(in.size()));
                                CHECK_EQ(req.body(), in);
                                co_return http_response(http::status::ok)
                                    .set_body(req.body());
                              }))
            .build();
  auto res = server.serve_http_request(
      "/api/get",
      http_request()
          .set_method(http::verb::get)
          .set_header(http::field::content_encoding, "deflate")
          .set_header(http::field::accept_encoding, "deflate")
          .set_body(middleware::zlib::compress<std::string>(
                        net::const_buffer(in.data(), in.size()))
                        .value())
          .prepare_payload());
  CHECK_EQ(res.status_code(), http::status::ok);
  CHECK_EQ(res.headers().get(http::field::content_encoding), "deflate");
  CHECK_EQ(res.body(),
           middleware::zlib::compress<std::string>(
               net::const_buffer(in.data(), in.size())));
}

#endif

TEST_SUITE_END();
