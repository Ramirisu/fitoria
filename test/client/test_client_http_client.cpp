//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria/test/test.hpp>

#include <fitoria/test/http_client.hpp>
#include <fitoria/test/http_server_utils.hpp>

using namespace fitoria;
using namespace fitoria::test;

TEST_SUITE_BEGIN("[fitoria.client.http_client]");

TEST_CASE("resource")
{
  {
    auto c = http_client()
                 .set_method(http::verb::get)
                 .set_url("http://httpbun.com/get");
    CHECK_EQ(c.host().value(), "httpbun.com");
    CHECK_EQ(c.port().value(), 80);
    CHECK_EQ(c.path().value(), "/get");
  }
  {
    auto c = http_client()
                 .set_method(http::verb::get)
                 .set_url("https://httpbun.com/get");
    CHECK_EQ(c.host().value(), "httpbun.com");
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
               .set_url("http://httpbun.com/get");
  CHECK_EQ(c.method(), http::verb::get);
  c.set_method(http::verb::post);
  CHECK_EQ(c.method(), http::verb::post);

  CHECK_EQ(c.header().get(http::field::content_type), nullopt);
  c.set_header(http::field::content_type, mime::text_plain());
  CHECK_EQ(c.header().get(http::field::content_type), mime::text_plain());

  c.set_plaintext("hello world");
  CHECK_EQ(c.header().get(http::field::content_type), mime::text_plain());

  c.set_json({ { "msg", "hello world" } });
  CHECK_EQ(c.header().get(http::field::content_type), mime::application_json());

  CHECK_EQ(c.transfer_timeout(), std::chrono::seconds(5));
  c.set_transfer_timeout(std::chrono::seconds(10));
  CHECK_EQ(c.transfer_timeout(), std::chrono::seconds(10));

  c.set_query("name", "value");
  CHECK_EQ(c.query().get("name"), "value");
  const auto& cc = c;
  CHECK_EQ(cc.query().get("name"), "value");
}

TEST_CASE("async_send")
{
  sync_wait([]() -> awaitable<void> {
    auto res = co_await http_client()
                   .set_method(http::verb::get)
                   .set_url("http://httpbun.com/get")
                   .async_send();
    CHECK_EQ(res->status_code().value(), http::status::ok);
    CHECK(!(co_await res->as_string())->empty());
  });

  sync_wait([]() -> awaitable<void> {
    CHECK(!(co_await http_client()
                .set_method(http::verb::get)
                .set_url("")
                .async_send()));
  });
}

TEST_CASE("request with body")
{
  sync_wait([]() -> awaitable<void> {
    auto res = co_await http_client()
                   .set_method(http::verb::post)
                   .set_url("http://httpbun.com/post")
                   .set_body("echo")
                   .async_send();
    CHECK_EQ(res->status_code().value(), http::status::ok);
    CHECK_EQ((co_await res->as_json())->at("data"), "echo");
  });
}

TEST_SUITE_END();
