//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria/test/test.hpp>

#include <fitoria/test/cert.hpp>
#include <fitoria/test/http_server_utils.hpp>

#include <fitoria/client.hpp>

using namespace fitoria;
using namespace fitoria::client;
using namespace fitoria::test::cert;
using namespace fitoria::test;

TEST_SUITE_BEGIN("[fitoria.client.http_client]");

TEST_CASE("resource")
{
  {
    auto c = http_client()
                 .set_method(http::verb::get)
                 .set_url("http://httpbin.org/get");
    CHECK_EQ(c.host().value(), "httpbin.org");
    CHECK_EQ(c.port().value(), 80);
    CHECK_EQ(c.path().value(), "/get");
  }
  {
    auto c = http_client()
                 .set_method(http::verb::get)
                 .set_url("https://httpbin.org/get");
    CHECK_EQ(c.host().value(), "httpbin.org");
    CHECK_EQ(c.port().value(), 443);
    CHECK_EQ(c.path().value(), "/get");
  }
  {
    auto c = http_client()
                 .set_method(http::verb::get)
                 .set_url("http://abc/def%20ghi");
    CHECK_EQ(c.host().value(), "abc");
    CHECK_EQ(c.port().value(), 80);
    CHECK_EQ(c.path().value(), "/def ghi");
  }
}

TEST_CASE("misc")
{
  auto c = http_client()
               .set_method(http::verb::get)
               .set_url("http://httpbin.org/get");
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

  CHECK_EQ(c.expect_100_timeout(), std::chrono::seconds(1));
  c.set_expect_100_timeout(std::chrono::seconds(10));
  CHECK_EQ(c.expect_100_timeout(), std::chrono::seconds(10));

  c.set_query("name", "value");
  CHECK_EQ(c.query().get("name"), "value");
  const auto& cc = c;
  CHECK_EQ(cc.query().get("name"), "value");
}

TEST_CASE("async_send")
{
  sync_wait([]() -> net::awaitable<void> {
    auto res = co_await http_client()
                   .set_method(http::verb::get)
                   .set_url("http://httpbin.org/get")
                   .async_send();
    CHECK_EQ(res->status_code().value(), http::status::ok);
    CHECK((co_await res->as_string())->size() > 0);
  });

  sync_wait([]() -> net::awaitable<void> {
    CHECK(!(co_await http_client()
                .set_method(http::verb::get)
                .set_url("")
                .async_send()));
  });

#if defined(FITORIA_HAS_OPENSSL)
  auto get_certs = []() {
    auto ssl_ctx = net::ssl::context(net::ssl::context::method::tls);
    ssl_ctx.set_verify_mode(net::ssl::verify_peer);
    cacert::add_builtin_cacerts(ssl_ctx);
    return ssl_ctx;
  };

  sync_wait([&]() -> net::awaitable<void> {
    auto res = co_await http_client()
                   .set_method(http::verb::get)
                   .set_url("https://httpbin.org/get")
                   .async_send(get_certs());
    CHECK_EQ(res->status_code().value(), http::status::ok);
    CHECK((co_await res->as_string())->size() > 0);
  });

  sync_wait([&]() -> net::awaitable<void> {
    CHECK(!(co_await http_client()
                .set_method(http::verb::get)
                .set_url("")
                .async_send(get_certs())));
  });
#endif
}

TEST_CASE("request with body")
{
  sync_wait([]() -> net::awaitable<void> {
    auto res = co_await http_client()
                   .set_method(http::verb::post)
                   .set_url("http://httpbin.org/post")
                   .set_body("echo")
                   .async_send();
    CHECK_EQ(res->status_code().value(), http::status::ok);
    auto body = co_await res->as_json();
    CHECK_EQ(body->at("data"), "echo");
  });
}

TEST_SUITE_END();
