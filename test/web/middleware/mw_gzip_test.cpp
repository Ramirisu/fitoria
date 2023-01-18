//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria_test.h>

#include <fitoria_http_server_utils.h>
#include <fitoria_simple_http_client.h>

#include <fitoria/web.hpp>

using namespace fitoria;

using namespace fitoria::http_server_utils;

TEST_SUITE_BEGIN("web.middleware.gzip");

#if defined(FITORIA_HAS_ZLIB)

TEST_CASE("gzip decompress")
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

  auto out = middleware::gzip::decompress<std::string>(
      net::const_buffer(in.data(), in.size()));

  const auto exp = std::string_view(
      "abcdefghijklmnopqrstuvwxyz0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ");
  CHECK_EQ(out, exp);
}

TEST_CASE("gzip compress")
{
  const auto in = std::string_view(
      "abcdefghijklmnopqrstuvwxyz0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ");

  auto intermediate = middleware::gzip::compress<std::vector<std::uint8_t>>(
      net::const_buffer(in.data(), in.size()));

  auto out = middleware::gzip::decompress<std::string>(
      net::const_buffer(intermediate.data(), intermediate.size()));

  CHECK_EQ(out, in);
}

TEST_CASE("gzip middleware")
{
  const auto in = std::string_view(
      "abcdefghijklmnopqrstuvwxyz0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ");

  const auto port = generate_port();
  auto server
      = http_server::builder()
            .route(scope("/api")
                       .use(middleware::gzip())
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
  server.bind(server_ip, port).run();
  std::this_thread::sleep_for(server_start_wait_time);

  auto res = simple_http_client(localhost, port)
                 .with(http::verb::get)
                 .with_target("/api/get")
                 .with_field(http::field::content_encoding, "gzip")
                 .with_field(http::field::accept_encoding, "gzip")
                 .with_field(http::field::connection, "close")
                 .with_body(middleware::gzip::compress<std::string>(
                     net::const_buffer(in.data(), in.size())))
                 .send_request();
  CHECK_EQ(res.result(), http::status::ok);
  CHECK_EQ(res.at(http::field::content_encoding), "gzip");
  CHECK_EQ(res.body(),
           middleware::gzip::compress<std::string>(
               net::const_buffer(in.data(), in.size())));
}

#endif

TEST_SUITE_END();
