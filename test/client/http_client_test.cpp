//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria_test.h>

#include <fitoria_certificate.h>

#include <fitoria/client.hpp>

using namespace fitoria;
using namespace fitoria::client;

TEST_SUITE_BEGIN("[fitoria.client.http_client]");

TEST_CASE("resource")
{
  SUBCASE("http")
  {
    auto c = http_client(http::verb::get, "http://httpbin.org/get");
    CHECK_EQ(c.host().value(), "httpbin.org");
    CHECK_EQ(c.port().value(), 80);
    CHECK_EQ(c.path().value(), "/get");
  }
  SUBCASE("https")
  {
    auto c = http_client(http::verb::get, "https://httpbin.org/get");
    CHECK_EQ(c.host().value(), "httpbin.org");
    CHECK_EQ(c.port().value(), 443);
    CHECK_EQ(c.path().value(), "/get");
  }
  SUBCASE("percentage encoding")
  {
    auto c = http_client(http::verb::get, "http://abc/def%20ghi");
    CHECK_EQ(c.host().value(), "abc");
    CHECK_EQ(c.port().value(), 80);
    CHECK_EQ(c.path().value(), "/def ghi");
  }
}

TEST_CASE("misc")
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
    auto c = http_client::get("http://httpbin.org/get");
    CHECK_EQ(c.method(), http::verb::get);
  }
  {
    auto c = http_client::post("http://httpbin.org/post");
    CHECK_EQ(c.method(), http::verb::post);
  }
  {
    auto c = http_client::put("http://httpbin.org/put");
    CHECK_EQ(c.method(), http::verb::put);
  }
  {
    auto c = http_client::patch("http://httpbin.org/patch");
    CHECK_EQ(c.method(), http::verb::patch);
  }
  {
    auto c = http_client::delete_("http://httpbin.org/delete");
    CHECK_EQ(c.method(), http::verb::delete_);
  }
  {
    auto c = http_client::head("http://httpbin.org/head");
    CHECK_EQ(c.method(), http::verb::head);
  }
  {
    auto c = http_client::options("http://httpbin.org/options");
    CHECK_EQ(c.method(), http::verb::options);
  }
}

TEST_CASE("async_send")
{
  {
    net::sync_wait([]() -> net::awaitable<void> {
      auto res
          = (co_await http_client::get("http://httpbin.org/get").async_send())
                .value();
      CHECK_EQ(res.status_code().value(), http::status::ok);
      CHECK((co_await res.as_string())->size() > 0);
    });
  }
  {
    net::sync_wait([]() -> net::awaitable<void> {
      CHECK(!(co_await http_client::get("").async_send()));
    });
  }
#if defined(FITORIA_HAS_OPENSSL)
  auto get_certs = []() {
    auto ssl_ctx = net::ssl::context(net::ssl::context::method::tls);
    ssl_ctx.set_verify_mode(net::ssl::verify_peer);
    cacert::add_builtin_cacerts(ssl_ctx);
    return ssl_ctx;
  };
  {
    net::sync_wait([&]() -> net::awaitable<void> {
      auto res = (co_await http_client::get("https://httpbin.org/get")
                      .async_send(get_certs()))
                     .value();
      CHECK_EQ(res.status_code().value(), http::status::ok);
      CHECK((co_await res.as_string())->size() > 0);
    });
  }
  {
    net::sync_wait([&]() -> net::awaitable<void> {
      CHECK(!(co_await http_client::get("").async_send(get_certs())));
    });
  }
#endif
}

TEST_SUITE_END();
