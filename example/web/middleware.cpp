//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria/web.hpp>

#include <iostream>

using namespace fitoria;
using namespace fitoria::web;

template <typename Request, typename Response, typename Next>
class my_log_middleware {
  friend class my_log;

public:
  awaitable<response> operator()(request& req) const
  {
    std::cout << fmt::format("{} {} HTTP/{}",
                             std::string(to_string(req.method())),
                             req.path().match_path(),
                             req.version())
              << std::endl;

    response res = co_await next_(req);

    std::cout << fmt::format(
        "HTTP/{} {} {}",
        http::version::v1_1,
        to_underlying(res.status_code().value()),
        std::string(obsolete_reason(res.status_code().value())))
              << std::endl;

    co_return res;
  }

private:
  template <typename Next2>
  my_log_middleware(Next2 next, log::level lv)
      : next_(std::forward<Next2>(next))
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

  template <typename Request,
            typename Response,
            decay_to<my_log> Self,
            typename Next>
  friend auto
  tag_invoke(to_middleware_t<Request, Response>, Self&& self, Next&& next)
  {
    return std::forward<Self>(self)
        .template to_middleware_impl<Request, Response>(
            std::forward<Next>(next));
  }

private:
  template <typename Request, typename Response, typename Next>
  auto to_middleware_impl(Next&& next) const
  {
    return my_log_middleware<Request, Response, std::decay_t<Next>>(
        std::forward<Next>(next), lv_);
  }

  log::level lv_;
};

auto get_user(request& req) -> awaitable<response>
{
  co_return response::ok()
      .set_field(http::field::content_type,
                 http::fields::content_type::plaintext())
      .set_body(req.path().get("user").value_or("{{unknown}}"));
}

int main()
{
  auto ioc = net::io_context();
  auto server = http_server::builder(ioc)
                    .serve(scope<"/api/v1">()
                               .use(middleware::logger())
#if !FITORIA_NO_EXCEPTIONS
                               .use(middleware::exception_handler())
#endif
                               .use(middleware::decompress())
                               .use(my_log(log::level::info))
                               .serve(route::get<"/users/{user}">(get_user)))
                    .build();
  server.bind("127.0.0.1", 8080);

  ioc.run();
}
