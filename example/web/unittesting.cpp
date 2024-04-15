//
// Copyright (c) 2024 fitoria (labyrinth.fitoria@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria/web.hpp>

using namespace fitoria;
using namespace fitoria::web;

int main()
{
  auto ioc = net::io_context();
  auto server
      = http_server_builder(ioc)
            .serve(route::post<"/api/v1/login">(
                [](const http_fields& fields,
                   std::string body) -> awaitable<http_response> {
                  if (fields.get(http::field::content_type)
                      != http::fields::content_type::form_urlencoded()) {
                    co_return http_response::bad_request()
                        .set_field(http::field::content_type,
                                   http::fields::content_type::plaintext())
                        .set_body("unexpected content-type");
                    ;
                  }
                  auto user = as_form(body);
                  if (!user || user->get("name") != "fitoria"
                      || user->get("password") != "123456") {
                    co_return http_response::unauthorized()
                        .set_field(http::field::content_type,
                                   http::fields::content_type::plaintext())
                        .set_body("incorrect user name or password");
                  }
                  co_return http_response::ok()
                      .set_field(http::field::content_type,
                                 http::fields::content_type::plaintext())
                      .set_body(fmt::format("{}, login succeeded",
                                            user->get("name")));
                }))
            .build();

  server.serve_request(
      "/api/v1/login",
      http_request(http::verb::post)
          .set_field(http::field::content_type,
                     http::fields::content_type::plaintext())
          .set_body("name=fitoria&password=123456"),
      []([[maybe_unused]] auto res) -> awaitable<void> {
        FITORIA_ASSERT(res.status_code() == http::status::bad_request);
        FITORIA_ASSERT((co_await res.as_string()) == "unexpected content-type");
        co_return;
      });
  server.serve_request(
      "/api/v1/login",
      http_request(http::verb::post)
          .set_field(http::field::content_type,
                     http::fields::content_type::form_urlencoded())
          .set_body("name=unknown&password=123456"),
      []([[maybe_unused]] auto res) -> awaitable<void> {
        FITORIA_ASSERT(res.status_code() == http::status::unauthorized);
        FITORIA_ASSERT((co_await res.as_string())
                       == "incorrect user name or password");
        co_return;
      });
  server.serve_request(
      "/api/v1/login",
      http_request(http::verb::post)
          .set_field(http::field::content_type,
                     http::fields::content_type::form_urlencoded())
          .set_body("name=fitoria&password=123"),
      []([[maybe_unused]] auto res) -> awaitable<void> {
        FITORIA_ASSERT(res.status_code() == http::status::unauthorized);
        FITORIA_ASSERT((co_await res.as_string())
                       == "incorrect user name or password");
        co_return;
      });
  server.serve_request(
      "/api/v1/login",
      http_request(http::verb::post)
          .set_field(http::field::content_type,
                     http::fields::content_type::form_urlencoded())
          .set_body("name=fitoria&password=123456"),
      []([[maybe_unused]] auto res) -> awaitable<void> {
        FITORIA_ASSERT(res.status_code() == http::status::ok);
        FITORIA_ASSERT(res.fields().get(http::field::content_type)
                       == http::fields::content_type::plaintext());
        FITORIA_ASSERT((co_await res.as_string())
                       == "fitoria, login succeeded");
        co_return;
      });

  ioc.run();
}
