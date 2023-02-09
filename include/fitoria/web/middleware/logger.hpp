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
  Next next_;

public:
  using clock_t = std::chrono::system_clock;

  logger_service(Next next)
      : next_(std::move(next))
  {
  }

  net::awaitable<http_response> operator()(http_context& c) const
  {
    auto start_time = clock_t::now();

    auto res = co_await next_(c);

    log::info("[{}] {} {} {} {} {}B {} {:%T}s", name(),
              c.request().conn_info().remote_addr().to_string(),
              std::string(to_string(c.request().method())), c.request().path(),
              res.status_code(), res.body().size(),
              c.request().fields().get(http::field::user_agent).value_or(""),
              std::chrono::floor<std::chrono::microseconds>(clock_t::now()
                                                            - start_time));

    co_return res;
  }

private:
  static const char* name() noexcept
  {
    return "fitoria.middleware.logger";
  }
};

class logger {
public:
  template <typename Next>
  friend constexpr auto
  tag_invoke(tag_t<make_service>, const logger&, Next&& next)
  {
    return logger_service(std::move(next));
  }
};

}

FITORIA_NAMESPACE_END

#endif
