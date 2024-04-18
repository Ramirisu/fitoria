//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria/test/test.hpp>

#include <fitoria/test/http_server_utils.hpp>

#include <fitoria/web/async_read_until_eof.hpp>
#include <fitoria/web/response.hpp>

using namespace fitoria;
using namespace fitoria::web;
using namespace fitoria::test;

TEST_SUITE_BEGIN("[fitoria.web.response]");

TEST_CASE("builder")
{
  auto res = response::ok().build();
  auto builder
      = res.builder().set_status_code(http::status::internal_server_error);
  CHECK_EQ(builder.build().status_code(), http::status::internal_server_error);
}

TEST_CASE("status_code")
{
  auto res = response::ok().build();
  CHECK_EQ(res.status_code(), http::status::ok);
}

TEST_CASE("insert_field")
{
  auto res = response::ok()
                 .insert_field(http::field::content_type,
                               http::fields::content_type::plaintext())
                 .insert_field("content-encoding",
                               http::fields::content_encoding::deflate())
                 .build();
  CHECK_EQ(res.fields().get(http::field::content_type),
           http::fields::content_type::plaintext());
  CHECK_EQ(res.fields().get("Content-Type"),
           http::fields::content_type::plaintext());
  CHECK_EQ(res.fields().get(http::field::content_encoding),
           http::fields::content_encoding::deflate());
  CHECK_EQ(res.fields().get("Content-Encoding"),
           http::fields::content_encoding::deflate());
}

TEST_CASE("set_field")
{
  auto res = response::ok()
                 .set_field(http::field::content_type,
                            http::fields::content_type::plaintext())
                 .set_field("content-encoding",
                            http::fields::content_encoding::deflate())
                 .build();
  CHECK_EQ(res.fields().get(http::field::content_type),
           http::fields::content_type::plaintext());
  CHECK_EQ(res.fields().get("Content-Type"),
           http::fields::content_type::plaintext());
  CHECK_EQ(res.fields().get(http::field::content_encoding),
           http::fields::content_encoding::deflate());
  CHECK_EQ(res.fields().get("Content-Encoding"),
           http::fields::content_encoding::deflate());
}

struct user_t {
  std::string name;

  friend bool operator==(const user_t&, const user_t&) = default;
};

void tag_invoke(const boost::json::value_from_tag&,
                boost::json::value& jv,
                const user_t& user)
{
  jv = {
    { "name", user.name },
  };
}

TEST_CASE("set_json")
{
  sync_wait([]() -> awaitable<void> {
    {
      auto res = response::ok().set_json({ { "name", "Rina Hidaka" } });
      CHECK_EQ(co_await async_read_until_eof<std::string>(res.body().stream()),
               R"({"name":"Rina Hidaka"})");
    }
    {
      auto res = response::ok().set_json(user_t { .name = "Rina Hidaka" });
      CHECK_EQ(co_await async_read_until_eof<std::string>(res.body().stream()),
               R"({"name":"Rina Hidaka"})");
    }
  });
}

TEST_CASE("set_body")
{
  sync_wait([]() -> awaitable<void> {
    auto res = response::ok().set_body("Hello World");
    CHECK_EQ(co_await async_read_until_eof<std::string>(res.body().stream()),
             "Hello World");
  });
}

TEST_SUITE_END();
