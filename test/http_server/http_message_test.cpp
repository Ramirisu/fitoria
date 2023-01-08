//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria_test.h>

#include <fitoria/http_server/http_message.hpp>

using namespace fitoria;

TEST_SUITE_BEGIN("http_message");

namespace {

struct user_t {
  std::string name;
  std::string birth;

  friend bool operator==(const user_t&, const user_t&) = default;
};

user_t tag_invoke(const json::value_to_tag<user_t>&, const json::value& jv)
{
  return user_t {
    .name = std::string(jv.at("name").as_string()),
    .birth = std::string(jv.at("birth").as_string()),
  };
}

void tag_invoke(const json::value_from_tag&,
                json::value& jv,
                const user_t& user)
{
  jv = { { "name", user.name }, { "birth", user.birth } };
}

}

TEST_CASE("basic")
{
  http_message msg;
  const auto& cmsg = msg;
  msg.set_header(http::field::content_type, "text/plain");
  CHECK_EQ(msg.headers().at(http::field::content_type), "text/plain");
  CHECK_EQ(cmsg.headers().at(http::field::content_type), "text/plain");
  msg.set_header("Connection", "Close");
  CHECK_EQ(msg.headers().at(http::field::connection), "Close");
  CHECK_EQ(cmsg.headers().at(http::field::connection), "Close");
}

TEST_CASE("body_as_json")
{
  const json::value jv = {
    { "name", "Rina Hidaka" },
    { "birth", "1994/06/15" },
  };
  {
    http_message msg;
    msg.set_header(http::field::content_type, "text/plain");
    msg.set_body(json::serialize(jv));
    CHECK_EQ(msg.body_as_json().error(),
             make_error_code(error::unexpected_content_type));
  }
  {
    http_message msg;
    msg.set_header(http::field::content_type, "application/json");
    msg.set_body("{");
    CHECK_EQ(msg.body_as_json().error(),
             make_error_code(error::invalid_json_format));
  }
  {
    http_message msg;
    msg.set_header(http::field::content_type, "application/json");
    msg.set_body(json::serialize(jv));
    CHECK_EQ(msg.body_as_json(), jv);
  }
  {
    http_message msg;
    msg.set_header(http::field::content_type, "application/json");
    msg.set_body(json::serialize(jv));
    CHECK_EQ(msg.body_as_json<user_t>(),
             user_t {
                 .name = "Rina Hidaka",
                 .birth = "1994/06/15",
             });
  }
}

TEST_CASE("body_as_post_form")
{
  {
    http_message msg;
    msg.set_header(http::field::content_type, "text/plain");
    msg.set_body(R"(name=Rina%20Hidaka&birth=1994%2F06%2F15)");
    CHECK_EQ(msg.body_as_post_form().error(),
             make_error_code(error::unexpected_content_type));
  }
  {
    http_message msg;
    msg.set_header(http::field::content_type,
                   "application/x-www-form-urlencoded");
    msg.set_body("%%");
    CHECK_EQ(msg.body_as_post_form().error(),
             make_error_code(error::invalid_form_format));
  }
  {
    http_message msg;
    msg.set_header(http::field::content_type,
                   "application/x-www-form-urlencoded");
    msg.set_body(R"(name=Rina%20Hidaka&birth=1994%2F06%2F15)");
    auto form = msg.body_as_post_form();
    CHECK_EQ(form->get("name"), "Rina Hidaka");
    CHECK_EQ(form->get("birth"), "1994/06/15");
  }
}

TEST_SUITE_END();
