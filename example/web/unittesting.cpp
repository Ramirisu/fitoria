//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria/fitoria.hpp>

using namespace fitoria;

int main()
{
  auto server
      = http_server::builder()
            .route(
                route(http::verb::post, "/api/v1/login",
                      [](http_request& req) -> net::awaitable<http_response> {
                        if (req.headers().get(http::field::content_type)
                            != http::fields::content_type::form_urlencoded()) {
                          co_return http_response(http::status::bad_request);
                        }
                        auto user = as_form(req.body());
                        if (!user || user->get("name") != "ramirisu"
                            || user->get("password") != "123456") {
                          co_return http_response(http::status::unauthorized);
                        }
                        co_return http_response(http::status::ok)
                            .set_header(http::field::content_type,
                                        http::fields::content_type::plaintext())
                            .set_body(fmt::format("{}, login succeeded",
                                                  user->get("name")));
                      }))
            .build();

  {
    auto res = server.serve_http_request(
        "/api/v1/login",
        http_request()
            .set_method(http::verb::post)
            .set_header(http::field::content_type,
                        http::fields::content_type::plaintext())
            .set_body("name=ramirisu&password=123456"));
    FITORIA_ASSERT(res.status_code() == http::status::bad_request);
  }
  {
    auto res = server.serve_http_request(
        "/api/v1/login",
        http_request()
            .set_method(http::verb::post)
            .set_header(http::field::content_type,
                        http::fields::content_type::form_urlencoded())
            .set_body("name=unknown&password=123"));
    FITORIA_ASSERT(res.status_code() == http::status::unauthorized);
  }
  {
    auto res = server.serve_http_request(
        "/api/v1/login",
        http_request()
            .set_method(http::verb::post)
            .set_header(http::field::content_type,
                        http::fields::content_type::form_urlencoded())
            .set_body("name=ramirisu&password=123"));
    FITORIA_ASSERT(res.status_code() == http::status::unauthorized);
  }
  {
    auto res = server.serve_http_request(
        "/api/v1/login",
        http_request()
            .set_method(http::verb::post)
            .set_header(http::field::content_type,
                        http::fields::content_type::form_urlencoded())
            .set_body("name=ramirisu&password=123456"));
    FITORIA_ASSERT(res.status_code() == http::status::ok);
    FITORIA_ASSERT(res.headers().get(http::field::content_type)
                   == http::fields::content_type::plaintext());
    FITORIA_ASSERT(res.body() == "ramirisu, login succeeded");
  }
}
