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

TEST_SUITE_BEGIN("web.middleware.compression");

#if defined(FITORIA_HAS_ZLIB)

TEST_CASE("compression priority: gzip > deflate")
{
  const auto in = std::string_view(
      "abcdefghijklmnopqrstuvwxyz0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ");

  auto server = http_server::builder()
                    .route(scope("/api")
                               .use(middleware::deflate())
                               .use(middleware::gzip())
                               .GET("/get",
                                    [&]([[maybe_unused]] http_request& req)
                                        -> lazy<http_response> {
                                      CHECK(!req.fields().get(
                                          http::field::content_encoding));
                                      CHECK_EQ(*req.fields().get(
                                                   http::field::content_length),
                                               std::to_string(in.size()));
                                      CHECK_EQ(req.body(), in);
                                      co_return http_response(http::status::ok)
                                          .set_body(req.body());
                                    }))
                    .build();
  {
    auto res = server.serve_http_request(
        "/api/get",
        http_request(http::verb::get)
            .set_field(http::field::content_encoding, "gzip")
            .set_field(http::field::accept_encoding, "gzip, deflate")
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
        "/api/get",
        http_request(http::verb::get)
            .set_field(http::field::content_encoding, "deflate")
            .set_field(http::field::accept_encoding, "gzip, deflate")
            .set_body(middleware::detail::deflate_compress<std::string>(
                          net::const_buffer(in.data(), in.size()))
                          .value())
            .prepare_payload());
    CHECK_EQ(res.status_code(), http::status::ok);
    CHECK_EQ(res.fields().get(http::field::content_encoding), "gzip");
    CHECK_EQ(res.body(),
             middleware::detail::gzip_compress<std::string>(
                 net::const_buffer(in.data(), in.size())));
  }
}

TEST_CASE("compression priority: deflate > gzip")
{
  const auto in = std::string_view(
      "abcdefghijklmnopqrstuvwxyz0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ");

  auto server = http_server::builder()
                    .route(scope("/api")
                               .use(middleware::gzip())
                               .use(middleware::deflate())
                               .GET("/get",
                                    [&]([[maybe_unused]] http_request& req)
                                        -> lazy<http_response> {
                                      CHECK(!req.fields().get(
                                          http::field::content_encoding));
                                      CHECK_EQ(*req.fields().get(
                                                   http::field::content_length),
                                               std::to_string(in.size()));
                                      CHECK_EQ(req.body(), in);
                                      co_return http_response(http::status::ok)
                                          .set_body(req.body());
                                    }))
                    .build();
  {
    auto res = server.serve_http_request(
        "/api/get",
        http_request(http::verb::get)
            .set_field(http::field::content_encoding, "gzip")
            .set_field(http::field::accept_encoding, "gzip, deflate")
            .set_body(middleware::detail::gzip_compress<std::string>(
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
        "/api/get",
        http_request(http::verb::get)
            .set_field(http::field::content_encoding, "deflate")
            .set_field(http::field::accept_encoding, "gzip, deflate")
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
}

#endif

TEST_SUITE_END();
