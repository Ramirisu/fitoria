//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria/web.hpp>

using namespace fitoria;
using namespace fitoria::web;

template <typename Next>
class my_log_middleware {
  friend class my_log;

public:
  auto operator()(http_request& req) const -> net::awaitable<http_response>
  {
    // do something before the handler

    auto res = co_await next_(req);

    // do something after the handler

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

  template <decay_to<my_log> Self, typename Next>
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
  co_return http_response(http::status::ok)
      .set_field(http::field::content_type,
                 http::fields::content_type::plaintext())
      .set_body(req.path().get("user").value_or("{{unknown}}"));
}

int main()
{
  auto ioc = net::io_context();
  auto server = http_server_builder(ioc)
                    .serve(scope<"/api/v1">()
                               .use(middleware::logger())
#if !FITORIA_NO_EXCEPTIONS
                               .use(middleware::exception_handler())
#endif
                               .use(my_log(log::level::info))
                               .serve(route::get<"/users/{user}">(get_user)))
                    .build();
  server.bind("127.0.0.1", 8080);

  ioc.run();
}
