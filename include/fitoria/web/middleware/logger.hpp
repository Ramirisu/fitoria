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

#include <fitoria/web/request.hpp>
#include <fitoria/web/response.hpp>
#include <fitoria/web/to_middleware.hpp>

FITORIA_NAMESPACE_BEGIN

namespace web::middleware {

template <typename Request, typename Response, typename Next>
class logger_middleware {
  friend class logger;

public:
  using clock_t = std::chrono::system_clock;

  auto operator()(Request req) const -> Response
  {
    auto start_time = clock_t::now();

    auto res = co_await next_(req);

    log::info("[fitoria.middleware.logger] {} {} {} {} {} {:%T}s",
              req.connection().remote(),
              std::string(to_string(req.method())),
              req.path().match_path(),
              res.status(),
              req.headers().get(http::field::user_agent).value_or(""),
              std::chrono::floor<std::chrono::microseconds>(clock_t::now()
                                                            - start_time));

    co_return res;
  }

private:
  template <typename Next2>
  logger_middleware(construct_t, Next2&& next)
      : next_(std::forward<Next2>(next))
  {
  }

  Next next_;
};

class logger {
public:
  template <typename Request,
            typename Response,
            decay_to<logger> Self,
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
    return logger_middleware<Request, Response, std::decay_t<Next>>(
        construct_t {}, std::forward<Next>(next));
  }
};
}

FITORIA_NAMESPACE_END

#endif
