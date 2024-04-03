//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#ifndef FITORIA_WEB_MIDDLEWARE_LOGGER_HPP
#define FITORIA_WEB_MIDDLEWARE_LOGGER_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/core/type_traits.hpp>
#include <fitoria/core/utility.hpp>

#include <fitoria/log/log.hpp>

#include <fitoria/web/http_request.hpp>
#include <fitoria/web/http_response.hpp>
#include <fitoria/web/middleware_concept.hpp>

FITORIA_NAMESPACE_BEGIN

namespace web::middleware {

template <typename Next>
class logger_middleware {
  friend class logger;

public:
  using clock_t = std::chrono::system_clock;

  auto operator()(http_request& req) const -> net::awaitable<http_response>
  {
    auto start_time = clock_t::now();

    auto res = co_await next_(req);

    log::info("[fitoria.middleware.logger] {} {} {} {} {}B {} {:%T}s",
              req.connection().remote().address().to_string(),
              std::string(to_string(req.method())),
              req.path().match_path(),
              res.status_code(),
              res.body().size_hint().value_or(0),
              req.fields().get(http::field::user_agent).value_or(""),
              std::chrono::floor<std::chrono::microseconds>(clock_t::now()
                                                            - start_time));

    co_return res;
  }

private:
  template <typename Next2>
  logger_middleware(Next2&& next)
      : next_(std::forward<Next2>(next))
  {
  }

  Next next_;
};

template <typename Next>
logger_middleware(Next&&) -> logger_middleware<std::decay_t<Next>>;

class logger {
public:
  template <decay_to<logger> Self, typename Next>
  friend constexpr auto tag_invoke(new_middleware_t, Self&& self, Next&& next)
  {
    return std::forward<Self>(self).new_middleware_impl(
        std::forward<Next>(next));
  }

private:
  template <typename Next>
  auto new_middleware_impl(Next&& next) const
  {
    return logger_middleware(std::forward<Next>(next));
  }
};
}

FITORIA_NAMESPACE_END

#endif
