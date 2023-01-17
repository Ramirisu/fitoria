//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "fitoria/core/optional.hpp"
#include <fitoria_test.h>

#include <fitoria_certificate.h>

#include <fitoria/web/http_client.hpp>

using namespace fitoria;

TEST_SUITE_BEGIN("http_client");

TEST_CASE("basic")
{
  {
    auto c = http_client("httpbin.org", 80, "/get");
    CHECK_EQ(c.host(), "httpbin.org");
    CHECK_EQ(c.port(), 80);
    CHECK_EQ(c.target(), "/get");
  }
  {
    auto c = http_client::from_url("http://httpbin.org/get").value();
    CHECK_EQ(c.host(), "httpbin.org");
    CHECK_EQ(c.port(), 80);
    CHECK_EQ(c.target(), "/get");
  }
  {
    auto c = http_client("httpbin.org", 80, "/get");
    CHECK_EQ(c.method(), http::verb::unknown);
    c.set_method(http::verb::get);
    CHECK_EQ(c.method(), http::verb::get);
  }
  {
    auto c = http_client("httpbin.org", 80, "/get");
    CHECK_EQ(c.headers().get(http::field::content_type), nullopt);
    c.set_header(http::field::content_type, "text/plain");
    CHECK_EQ(c.headers().get(http::field::content_type), "text/plain");
  }
  {
    auto c = http_client("httpbin.org", 80, "/get");
    CHECK_EQ(c.body(), "");
    c.set_body("hello world");
    CHECK_EQ(c.body(), "hello world");
  }
  {
    auto c = http_client("httpbin.org", 80, "/get");
    CHECK_EQ(c.request_timeout(), std::chrono::seconds(5));
    c.set_request_timeout(std::chrono::seconds(10));
    CHECK_EQ(c.request_timeout(), std::chrono::seconds(10));
  }
}

TEST_CASE("send")
{
  {
    auto c = http_client::from_url("http://httpbin.org/get")
                 .value()
                 .set_method(http::verb::get);
    auto res = c.send().value();
    CHECK_EQ(res.status_code(), http::status::ok);
  }
#if defined(FITORIA_HAS_OPENSSL)
  {
    auto c = http_client::from_url("https://httpbin.org/get")
                 .value()
                 .set_method(http::verb::get);
    auto res
        = c.send(cacert::get_client_ssl_ctx(net::ssl::context::method::tls))
              .value();
    CHECK_EQ(res.status_code(), http::status::ok);
  }
#endif
}

TEST_CASE("async_send")
{
  {
    auto send
        = [](const http_client& c) -> expected<http_response, error_code> {
      net::io_context ioc;
      auto res = net::co_spawn(ioc, c.async_send(), net::use_future);
      ioc.run();
      return res.get();
    };

    auto c = http_client::from_url("http://httpbin.org/get")
                 .value()
                 .set_method(http::verb::get);
    auto res = send(c).value();
    CHECK_EQ(res.status_code(), http::status::ok);
  }
#if defined(FITORIA_HAS_OPENSSL)
  {
    auto send
        = [](const http_client& c,
             net::ssl::context ssl_ctx) -> expected<http_response, error_code> {
      net::io_context ioc;
      auto res = net::co_spawn(ioc, c.async_send(std::move(ssl_ctx)),
                               net::use_future);
      ioc.run();
      return res.get();
    };

    auto c = http_client::from_url("https://httpbin.org/get")
                 .value()
                 .set_method(http::verb::get);
    auto res
        = send(c, cacert::get_client_ssl_ctx(net::ssl::context::method::tls))
              .value();
    CHECK_EQ(res.status_code(), http::status::ok);
  }
#endif
}

TEST_SUITE_END();
