//
// Copyright (c) 2022 Ramirisu (labyrinth dot ramirisu at gmail dot com)
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

TEST_SUITE_BEGIN("http_server.request");

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

TEST_CASE("request with plain text")
{
  const auto port = generate_port();
  auto server = http_server(http_server_config().route(router(
      verb::get, "/api/v1/users/{user}/filmography/years/{year}",
      [](http_request& req, http_route& route, query_map& query)
          -> net::awaitable<expected<http_response, http_error>> {
        CHECK_EQ(req.remote_endpoint().address(),
                 net::ip::make_address(server_ip));

        auto test_route = [](http_route& route) {
          CHECK_EQ(route.path(),
                   "/api/v1/users/{user}/filmography/years/{year}");

          CHECK_EQ(route.at("user"), "Rina Hidaka");
          CHECK_EQ(route.at("year"), "2022");
        };
        test_route(req.route());
        test_route(route);

        CHECK_EQ(req.method(), verb::get);
        CHECK_EQ(req.path(),
                 "/api/v1/users/Rina Hidaka/filmography/years/2022");

        CHECK_EQ(req.query_string(), "name=Rina Hidaka&birth=1994/06/15");

        auto test_query = [](query_map& query) {
          CHECK_EQ(query.size(), 2);
          CHECK_EQ(query.at("name"), "Rina Hidaka");
          CHECK_EQ(query.at("birth"), "1994/06/15");
        };
        test_query(req.query());
        test_query(query);

        CHECK_EQ(req.headers().at(field::content_type), "text/plain");
        CHECK_EQ(req.body(), "happy birthday");

        co_return http_response(status::ok);
      })));
  server.bind(server_ip, port).run();
  std::this_thread::sleep_for(server_start_wait_time);

  auto resp
      = simple_http_client(localhost, port)
            .with_target(
                R"(/api/v1/users/Rina%20Hidaka/filmography/years/2022?name=Rina%20Hidaka&birth=1994%2F06%2F15)")
            .with(verb::get)
            .with_field(field::content_type, "text/plain")
            .with_field(field::connection, "close")
            .with_body("happy birthday")
            .send_request();
  CHECK_EQ(resp.result(), status::ok);
}

TEST_CASE("request with json")
{
  const auto port = generate_port();
  auto server = http_server(http_server_config().route(
      router(verb::get, "/api",
             [](http_request& req, from_json<user_t> user)
                 -> net::awaitable<expected<http_response, http_error>> {
               CHECK_EQ(req.method(), verb::get);

               CHECK_EQ(req.body(),
                        json::serialize(json::value {
                            { "name", "Rina Hidaka" },
                            { "birth", "1994/06/15" },
                        }));

               CHECK_EQ(user.value(),
                        user_t {
                            .name = "Rina Hidaka",
                            .birth = "1994/06/15",
                        });

               co_return http_response(status::ok);
             })));
  server.bind(server_ip, port).run();
  std::this_thread::sleep_for(server_start_wait_time);

  auto resp = simple_http_client(localhost, port)
                  .with_target("/api")
                  .with(verb::get)
                  .with_field(field::content_type, "application/json")
                  .with_field(field::connection, "close")
                  .with_body(json::serialize(json::value {
                      { "name", "Rina Hidaka" },
                      { "birth", "1994/06/15" },
                  }))
                  .send_request();
  CHECK_EQ(resp.result(), status::ok);
}

TEST_SUITE_END();
