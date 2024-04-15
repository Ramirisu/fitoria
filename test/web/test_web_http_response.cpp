//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria/test/test.hpp>

#include <fitoria/test/http_server_utils.hpp>

#include <fitoria/web/async_read_until_eof.hpp>
#include <fitoria/web/http_response.hpp>

using namespace fitoria;
using namespace fitoria::web;
using namespace fitoria::test;

TEST_SUITE_BEGIN("[fitoria.web.http_response]");

TEST_CASE("status_code")
{
  {
    http_response res;
    CHECK_EQ(res.status_code(), http::status::ok);
    res.set_status_code(http::status::not_found);
    CHECK_EQ(res.status_code(), http::status::not_found);
  }
  {
    http_response res(http::status::not_found);
    CHECK_EQ(res.status_code(), http::status::not_found);
  }
}

TEST_CASE("set_field")
{
  {
    http_response res;
    res.set_field(http::field::content_type,
                  http::fields::content_type::plaintext());
    CHECK_EQ(res.fields().get(http::field::content_type),
             http::fields::content_type::plaintext());
  }
  {
    http_response res;
    res.set_field("Content-Type", http::fields::content_type::plaintext());
    CHECK_EQ(res.fields().get(http::field::content_type),
             http::fields::content_type::plaintext());
  }
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
      http_response res;
      res.set_json({ { "name", "Rina Hidaka" } });
      CHECK_EQ(res.fields().get(http::field::content_type),
               http::fields::content_type::json());
      CHECK_EQ(co_await async_read_until_eof<std::string>(*res.body()),
               R"({"name":"Rina Hidaka"})");
    }
    {
      http_response res;
      res.set_json(user_t { .name = "Rina Hidaka" });
      CHECK_EQ(res.fields().get(http::field::content_type),
               http::fields::content_type::json());
      CHECK_EQ(co_await async_read_until_eof<std::string>(*res.body()),
               R"({"name":"Rina Hidaka"})");
    }
  });
}

TEST_CASE("set_body: string")
{
  sync_wait([]() -> awaitable<void> {
    http_response res;
    res.set_body("Hello World");
    CHECK_EQ(co_await async_read_until_eof<std::string>(*res.body()),
             "Hello World");
  });
}

TEST_CASE("set_body: vector")
{
  sync_wait([]() -> awaitable<void> {
    http_response res;
    res.set_body("Hello World");
    CHECK_EQ(
        co_await async_read_until_eof<std::vector<std::uint8_t>>(*res.body()),
        std::vector<std::uint8_t> {
            'H', 'e', 'l', 'l', 'o', ' ', 'W', 'o', 'r', 'l', 'd' });
  });
}

TEST_SUITE_END();
