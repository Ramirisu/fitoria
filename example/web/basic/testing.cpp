//
// Copyright (c) 2024 fitoria (labyrinth.fitoria@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria/web.hpp>

using namespace fitoria;
using namespace fitoria::web;

struct user_t {
  std::string username;
  std::string password;
};

auto login(form_of<user_t> user) -> awaitable<response>
{
  if (user.username != "fitoria" || user.password != "123456") {
    co_return response::unauthorized()
        .set_header(http::field::content_type, mime::text_plain())
        .set_body("incorrect username or password");
  }
  co_return response::ok()
      .set_header(http::field::content_type, mime::text_plain())
      .set_body(fmt::format("{}, login succeeded", user.username));
}

int main()
{
  auto ioc = net::io_context();
  auto server = http_server::builder(ioc)
                    .serve(route::post<"/api/v1/login">(login))
                    .build();

  server.serve_request(
      "/api/v1/login",
      test_request::post()
          .set_header(http::field::content_type, mime::text_plain())
          .set_body("username=fitoria&password=123456"),
      []([[maybe_unused]] test_response res) -> awaitable<void> {
        FITORIA_ASSERT(res.status_code()
                       == http::status::internal_server_error);
        FITORIA_ASSERT(
            (co_await res.as_string())
            == "unexpected Content-Type received, expected \"Content-Type: "
               "application/x-www-form-urlencoded\"");
        co_return;
      });
  server.serve_request(
      "/api/v1/login",
      test_request::post()
          .set_header(http::field::content_type,
                      mime::application_www_form_urlencoded())
          .set_body("username=unknown&password=123456"),
      []([[maybe_unused]] test_response res) -> awaitable<void> {
        FITORIA_ASSERT(res.status_code() == http::status::unauthorized);
        FITORIA_ASSERT((co_await res.as_string())
                       == "incorrect username or password");
        co_return;
      });
  server.serve_request(
      "/api/v1/login",
      test_request::post()
          .set_header(http::field::content_type,
                      mime::application_www_form_urlencoded())
          .set_body("username=fitoria&password=123"),
      []([[maybe_unused]] test_response res) -> awaitable<void> {
        FITORIA_ASSERT(res.status_code() == http::status::unauthorized);
        FITORIA_ASSERT((co_await res.as_string())
                       == "incorrect username or password");
        co_return;
      });
  server.serve_request(
      "/api/v1/login",
      test_request::post()
          .set_header(http::field::content_type,
                      mime::application_www_form_urlencoded())
          .set_body("username=fitoria&password=123456"),
      []([[maybe_unused]] test_response res) -> awaitable<void> {
        FITORIA_ASSERT(res.status_code() == http::status::ok);
        FITORIA_ASSERT(res.header().get(http::field::content_type)
                       == mime::text_plain());
        FITORIA_ASSERT((co_await res.as_string())
                       == "fitoria, login succeeded");
        co_return;
      });

  ioc.run();
}
