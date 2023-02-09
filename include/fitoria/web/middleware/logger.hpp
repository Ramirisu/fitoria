//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef FITORIA_WEB_MIDDLEWARE_LOGGER_HPP
#define FITORIA_WEB_MIDDLEWARE_LOGGER_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/core/utility.hpp>

#include <fitoria/log/logger.hpp>

#include <fitoria/web/http_context.hpp>
#include <fitoria/web/http_response.hpp>
#include <fitoria/web/service.hpp>

FITORIA_NAMESPACE_BEGIN

namespace web::middleware {

template <typename Next>
class logger_service {
  friend class logger;

public:
  using clock_t = std::chrono::system_clock;

  net::awaitable<http_response> operator()(http_context& c) const
  {
    auto start_time = clock_t::now();

    auto res = co_await next_(c);

    log::info("[fitoria.middleware.logger] {} {} {} {} {}B {} {:%T}s",
              c.request().conn_info().remote_addr().to_string(),
              std::string(to_string(c.request().method())), c.request().path(),
              res.status_code(), res.body().size(),
              c.request().fields().get(http::field::user_agent).value_or(""),
              std::chrono::floor<std::chrono::microseconds>(clock_t::now()
                                                            - start_time));

    co_return res;
  }

private:
  template <typename Next2>
  logger_service(Next2&& next)
      : next_(std::forward<Next2>(next))
  {
  }

  Next next_;
};

template <typename Next>
logger_service(Next&&) -> logger_service<std::decay_t<Next>>;

class logger {
  template <typename Next>
  auto new_service(Next&& next) const
  {
    return logger_service(std::forward<Next>(next));
  }

public:
  template <typename Next>
  friend constexpr auto
  tag_invoke(tag_t<make_service>, const logger& self, Next&& next)
  {
    return self.new_service(std::forward<Next>(next));
  }
};
}

FITORIA_NAMESPACE_END

#endif
