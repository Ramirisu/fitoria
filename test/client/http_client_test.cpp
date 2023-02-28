//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria_test.h>

#include <fitoria_certificate.h>

#include <fitoria/client.hpp>

using namespace fitoria;
using namespace fitoria::client;

TEST_SUITE_BEGIN("client.http_client");

TEST_CASE("basic")
{
  auto c = http_client(http::verb::get, "http://httpbin.org/get");
  CHECK_EQ(c.method(), http::verb::get);
  c.set_method(http::verb::post);
  CHECK_EQ(c.method(), http::verb::post);

  CHECK_EQ(c.fields().get(http::field::content_type), nullopt);
  c.set_field(http::field::content_type,
              http::fields::content_type::plaintext());
  CHECK_EQ(c.fields().get(http::field::content_type),
           http::fields::content_type::plaintext());

  c.set_plaintext("hello world");
  CHECK_EQ(c.fields().get(http::field::content_type),
           http::fields::content_type::plaintext());

  c.set_json({ { "msg", "hello world" } });
  CHECK_EQ(c.fields().get(http::field::content_type),
           http::fields::content_type::json());

  CHECK_EQ(c.request_timeout(), std::chrono::seconds(5));
  c.set_request_timeout(std::chrono::seconds(10));
  CHECK_EQ(c.request_timeout(), std::chrono::seconds(10));

  c.set_query("name", "value");
  CHECK_EQ(c.query().get("name"), "value");
  const auto& cc = c;
  CHECK_EQ(cc.query().get("name"), "value");
}

TEST_CASE("methods")
{
  {
    auto c = http_client::GET("http://httpbin.org/get");
    CHECK_EQ(c.method(), http::verb::get);
  }
  {
    auto c = http_client::POST("http://httpbin.org/get");
    CHECK_EQ(c.method(), http::verb::post);
  }
  {
    auto c = http_client::PUT("http://httpbin.org/get");
    CHECK_EQ(c.method(), http::verb::put);
  }
  {
    auto c = http_client::PATCH("http://httpbin.org/get");
    CHECK_EQ(c.method(), http::verb::patch);
  }
  {
    auto c = http_client::DELETE_("http://httpbin.org/get");
    CHECK_EQ(c.method(), http::verb::delete_);
  }
  {
    auto c = http_client::HEAD("http://httpbin.org/get");
    CHECK_EQ(c.method(), http::verb::head);
  }
  {
    auto c = http_client::OPTIONS("http://httpbin.org/get");
    CHECK_EQ(c.method(), http::verb::options);
  }
}

TEST_CASE("async_send")
{
  {
    auto res = net::sync_wait(
                   http_client::GET("http://httpbin.org/get").async_send())
                   .value();
    CHECK_EQ(res.status_code().value(), http::status::ok);
  }
  {
    CHECK(!net::sync_wait(http_client::GET("").async_send()));
  }
#if defined(FITORIA_HAS_OPENSSL)
  auto get_certs = []() {
    auto ssl_ctx = net::ssl::context(net::ssl::context::method::tls);
    ssl_ctx.set_verify_mode(net::ssl::verify_peer);
    cacert::add_builtin_cacerts(ssl_ctx);
    return ssl_ctx;
  };
  {
    auto res = net::sync_wait(http_client::GET("https://httpbin.org/get")
                                  .async_send(get_certs()))
                   .value();
    CHECK_EQ(res.status_code().value(), http::status::ok);
  }
  {
    CHECK(!net::sync_wait(http_client::GET("").async_send(get_certs())));
  }
#endif
}

TEST_CASE("format response")
{
  auto res
      = net::sync_wait(
            http_client(http::verb::get, "http://httpbin.org/get").async_send())
            .value();
  CHECK_EQ(res.status_code().value(), http::status::ok);

  auto contains = [](std::string_view str, std::string_view substr) {
    return str.find(substr) != std::string_view::npos;
  };

  auto str = fmt::format("{}", res);
  CHECK(str.starts_with("HTTP/1.1 OK\n"));
  CHECK(contains(str, "Connection: keep-alive\n"));
  CHECK(contains(str, "Content-Type: application/json\n"));
  CHECK(contains(str, "\"Host\": \"httpbin.org\","));
  CHECK(contains(str, "\"url\": \"http://httpbin.org/get\""));
}

TEST_SUITE_END();
