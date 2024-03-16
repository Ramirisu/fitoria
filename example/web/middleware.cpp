//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria/fitoria.hpp>

using namespace fitoria;
using namespace fitoria::web;

// clang-format off
// 
// $ ./middleware
// $ curl -X GET http://127.0.0.1:8080/api/v1/users/ramirisu -v
//
// server output:
// > 2023-01-01T00:00:00Z INFO before handler [middleware.cpp:36:23]
// > 2023-01-01T00:00:00Z DEBUG user: ramirisu [middleware.cpp:47:37]
// > 2023-01-01T00:00:00Z INFO after handler [middleware.cpp:38:23]
// > 2023-01-01T00:00:00Z INFO [fitoria.middleware.logger] 127.0.0.1 GET /api/v1/users/ramirisu 200 curl/7.83.1 [logger.hpp:31:14]
//
//
// clang-format on

template <typename Next>
class my_log_middleware {
  friend class my_log;

public:
  auto operator()(http_context& ctx) const -> net::awaitable<http_response>
  {
    log::log(lv_, "before handler");

    auto res = co_await next_(ctx);

    log::log(lv_, "after handler");

    co_return res;
  }

private:
  my_log_middleware(Next next, log::level lv)
      : next_(std::move(next))
      , lv_(lv)
  {
  }

  Next next_;
  log::level lv_;
};

class my_log {
public:
  my_log(log::level lv)
      : lv_(lv)
  {
  }

  template <uncvref_same_as<my_log> Self, typename Next>
  friend auto tag_invoke(new_middleware_t, Self&& self, Next&& next)
  {
    return std::forward<Self>(self).new_middleware_impl(
        std::forward<Next>(next));
  }

private:
  template <typename Next>
  auto new_middleware_impl(Next&& next) const
  {
    return my_log_middleware(std::move(next), lv_);
  }

  log::level lv_;
};

auto get_user(http_request& req) -> net::awaitable<http_response>
{
  log::debug("user: {}", req.path().get("user"));

  co_return http_response(http::status::ok)
      .set_field(http::field::content_type,
                 http::fields::content_type::plaintext())
      .set_body(req.path().get("user").value_or("{{unknown}}"));
}

int main()
{
  log::global_logger() = log::stdout_logger();
  log::global_logger()->set_log_level(log::level::debug);

  auto server = http_server::builder()
                    .serve(scope<"/api/v1">()
                               .use(middleware::logger())
                               .use(middleware::exception_handler())
                               .use(my_log(log::level::info))
                               .serve(route::get<"/users/{user}">(get_user)))
                    .build();
  server //
      .bind("127.0.0.1", 8080)
      .run();
}
