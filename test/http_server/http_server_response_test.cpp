//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria_test.h>

#include <fitoria_certificate.h>
#include <fitoria_http_server_utils.h>
#include <fitoria_simple_http_client.h>

#include <fitoria/http_server.hpp>

using namespace fitoria;

using namespace http_server_utils;

TEST_SUITE_BEGIN("http_server.response");

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

TEST_CASE("response status only")
{
  const auto port = generate_port();
  auto server = http_server(http_server_config().route(
      router(http::verb::get, "/api",
             []([[maybe_unused]] http_request& req)
                 -> net::awaitable<http_response> {
               co_return http_response(http::status::accepted);
             })));
  server.bind(server_ip, port).run();
  std::this_thread::sleep_for(server_start_wait_time);

  auto resp = simple_http_client(localhost, port)
                  .with(http::verb::get)
                  .with_target("/api")
                  .with_field(http::field::connection, "close")
                  .send_request();
  CHECK_EQ(resp.result(), http::status::accepted);
  CHECK_EQ(resp.at(http::field::connection), "close");
  CHECK_EQ(resp.at(http::field::content_length), "0");
  CHECK_EQ(resp.body(), "");
}

TEST_CASE("response with plain text")
{
  const auto port = generate_port();
  auto server = http_server(http_server_config().route(
      router(http::verb::get, "/api",
             []([[maybe_unused]] http_request& req)
                 -> net::awaitable<http_response> {
               co_return http_response(http::status::ok)
                   .set_header(http::field::content_type, "text/plain")
                   .set_body("plain text");
             })));
  server.bind(server_ip, port).run();
  std::this_thread::sleep_for(server_start_wait_time);

  auto resp = simple_http_client(localhost, port)
                  .with(http::verb::get)
                  .with_target("/api")
                  .with_field(http::field::connection, "close")
                  .send_request();
  CHECK_EQ(resp.result(), http::status::ok);
  CHECK_EQ(resp.at(http::field::content_type), "text/plain");
  CHECK_EQ(resp.body(), "plain text");
}

TEST_CASE("response with json")
{
  const auto port = generate_port();
  auto server = http_server(http_server_config().route(
      router(http::verb::get, "/api",
             []([[maybe_unused]] http_request& req)
                 -> net::awaitable<http_response> {
               co_return http_response(http::status::ok)
                   .set_header(http::field::content_type, "application/json")
                   .set_json({
                       { "obj_boolean", true },
                       { "obj_number", 1234567 },
                       { "obj_string", "str" },
                       { "obj_array", json::array { false, 7654321, "rts" } },
                   });
             })));
  server.bind(server_ip, port).run();
  std::this_thread::sleep_for(server_start_wait_time);

  auto resp = simple_http_client(localhost, port)
                  .with(http::verb::get)
                  .with_target("/api")
                  .with_field(http::field::connection, "close")
                  .send_request();
  CHECK_EQ(resp.result(), http::status::ok);
  CHECK_EQ(resp.at(http::field::content_type), "application/json");
  CHECK_EQ(resp.body(),
           json::serialize(json::value({
               { "obj_boolean", true },
               { "obj_number", 1234567 },
               { "obj_string", "str" },
               { "obj_array", json::array { false, 7654321, "rts" } },
           })));
}

TEST_CASE("response with struct to json")
{
  const auto port = generate_port();
  auto server = http_server(http_server_config().route(
      router(http::verb::get, "/api",
             []([[maybe_unused]] http_request& req)
                 -> net::awaitable<http_response> {
               co_return http_response(http::status::ok)
                   .set_header(http::field::content_type, "application/json")
                   .set_json(user_t {
                       .name = "Rina Hidaka",
                       .birth = "1994/06/15",
                   });
             })));
  server.bind(server_ip, port).run();
  std::this_thread::sleep_for(server_start_wait_time);

  auto resp = simple_http_client(localhost, port)
                  .with(http::verb::get)
                  .with_target("/api")
                  .with_field(http::field::connection, "close")
                  .send_request();
  CHECK_EQ(resp.result(), http::status::ok);
  CHECK_EQ(resp.at(http::field::content_type), "application/json");
  CHECK_EQ(json::value_to<user_t>(json::parse(resp.body())),
           user_t {
               .name = "Rina Hidaka",
               .birth = "1994/06/15",
           });
}

TEST_SUITE_END();
