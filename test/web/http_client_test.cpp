//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria_test.h>

#include <fitoria_certificate.h>

#include <fitoria/web/http_client.hpp>

using namespace fitoria;

TEST_SUITE_BEGIN("web.http_client");

TEST_CASE("basic")
{
  {
    auto rc
        = http_client::resource::parse_uri("http://httpbin.org/get").value();
    CHECK_EQ(rc.host, "httpbin.org");
    CHECK_EQ(rc.port, 80);
    CHECK_EQ(rc.path, "/get");
  }
  {
    auto c = http_client();
    CHECK_EQ(c.method(), http::verb::unknown);
    c.set_method(http::verb::get);
    CHECK_EQ(c.method(), http::verb::get);
  }
  {
    auto c = http_client();
    CHECK_EQ(c.fields().get(http::field::content_type), nullopt);
    c.set_field(http::field::content_type,
                http::fields::content_type::plaintext());
    CHECK_EQ(c.fields().get(http::field::content_type),
             http::fields::content_type::plaintext());
  }
  {
    auto c = http_client();
    CHECK_EQ(c.body(), "");
    c.set_body("hello world");
    CHECK_EQ(c.body(), "hello world");
  }
  {
    auto c = http_client();
    CHECK_EQ(c.request_timeout(), std::chrono::seconds(5));
    c.set_request_timeout(std::chrono::seconds(10));
    CHECK_EQ(c.request_timeout(), std::chrono::seconds(10));
  }
  {
    auto c = http_client();
    c.set_query("name", "value");
    CHECK_EQ(c.query().get("name"), "value");
    const auto& cc = c;
    CHECK_EQ(cc.query().get("name"), "value");
  }
}

TEST_CASE("send")
{
  {
    auto res
        = http_client()
              .set_method(http::verb::get)
              .send(http_client::resource::parse_uri("http://httpbin.org/get"))
              .value();
    CHECK_EQ(res.status_code().value(), http::status::ok);
  }
#if defined(FITORIA_HAS_OPENSSL)
  {
    auto ssl_ctx = net::ssl::context(net::ssl::context::method::tls);
    ssl_ctx.set_verify_mode(net::ssl::verify_peer);
    cacert::add_builtin_cacerts(ssl_ctx);
    auto res
        = http_client()
              .set_method(http::verb::get)
              .send(http_client::resource::parse_uri("https://httpbin.org/get"),
                    std::move(ssl_ctx))
              .value();
    CHECK_EQ(res.status_code().value(), http::status::ok);
  }
#endif
}

TEST_CASE("async_send")
{
  {
    auto send
        = [](std::string_view url) -> expected<http_response, error_code> {
      auto c = http_client().set_method(http::verb::get);
      net::io_context ioc;
      auto res = net::co_spawn(
          ioc, c.async_send(http_client::resource::parse_uri(url)),
          net::use_future);
      ioc.run();
      return res.get();
    };

    auto res = send("http://httpbin.org/get").value();
    CHECK_EQ(res.status_code().value(), http::status::ok);
  }
#if defined(FITORIA_HAS_OPENSSL)
  {
    auto send
        = [](std::string_view url,
             net::ssl::context ssl_ctx) -> expected<http_response, error_code> {
      auto c = http_client().set_method(http::verb::get);
      net::io_context ioc;
      auto res
          = net::co_spawn(ioc,
                          c.async_send(http_client::resource::parse_uri(url),
                                       std::move(ssl_ctx)),
                          net::use_future);
      ioc.run();
      return res.get();
    };

    auto ssl_ctx = net::ssl::context(net::ssl::context::method::tls);
    ssl_ctx.set_verify_mode(net::ssl::verify_peer);
    cacert::add_builtin_cacerts(ssl_ctx);
    auto res = send("https://httpbin.org/get", std::move(ssl_ctx)).value();
    CHECK_EQ(res.status_code().value(), http::status::ok);
  }
#endif
}

TEST_SUITE_END();
