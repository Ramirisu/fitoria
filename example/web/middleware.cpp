//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
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
class my_log_service {
  friend class my_log;

public:
  auto operator()(http_context& ctx) const -> lazy<http_response>
  {
    log::log(lv_, "before handler");

    auto res = co_await next_(ctx);

    log::log(lv_, "after handler");

    co_return res;
  }

private:
  my_log_service(Next next, log::level lv)
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
  friend constexpr auto tag_invoke(make_service_t, Self&& self, Next&& next)
  {
    return std::forward<Self>(self).new_service(std::forward<Next>(next));
  }

private:
  template <typename Next>
  auto new_service(Next&& next) const
  {
    return my_log_service(std::move(next), lv_);
  }

  log::level lv_;
};

int main()
{
  log::global_logger() = log::stdout_logger();
  log::global_logger()->set_log_level(log::level::debug);

  auto server
      = http_server::builder()
            .route(
                // Add a scope
                scope("/api/v1")
                    // Register built-in logger middleware
                    .use(middleware::logger())
#if !FITORIA_NO_EXCEPTIONS
                    // Register built-in exception_handler middleware
                    .use(middleware::exception_handler())
#endif
                    // Register built-in deflate middleware
                    .use(middleware::deflate())
#if defined(FITORIA_HAS_ZLIB)
                    // Register built-in gzip middleware
                    .use(middleware::gzip())
#endif
                    // Register a custom middleware for this group
                    .use(my_log(log::level::info))
                    // Register a route
                    // The route is associated with the middleware defined above
                    .GET("/users/{user}",
                         [](http_request& req) -> lazy<http_response> {
                           log::debug("user: {}", req.params().get("user"));

                           co_return http_response(http::status::ok)
                               .set_body(req.params().get("user").value_or(
                                   "{{unknown}}"));
                         }))
            .build();
  server //
      .bind("127.0.0.1", 8080)
      .run();
}
