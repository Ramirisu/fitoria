//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#ifndef FITORIA_WEB_MIDDLEWARE_EXCEPTION_HANDLER_HPP
#define FITORIA_WEB_MIDDLEWARE_EXCEPTION_HANDLER_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/core/net.hpp>
#include <fitoria/core/type_traits.hpp>

#include <fitoria/log.hpp>

#include <fitoria/web/http_response.hpp>
#include <fitoria/web/middleware_concept.hpp>
#include <fitoria/web/request.hpp>

FITORIA_NAMESPACE_BEGIN

namespace web::middleware {

#if !FITORIA_NO_EXCEPTIONS

template <typename Request, typename Response, typename Next>
class exception_handler_middleware {
  friend class exception_handler;

public:
  auto operator()(Request req) const -> Response
  {
    try {
      co_return co_await next_(req);
    } catch (const std::exception& ex) {
      log::error("[fitoria.middleware.exception_handler] exception: {}",
                 ex.what());
    }

    co_return http_response::internal_server_error().build();
  }

private:
  template <typename Next2>
  exception_handler_middleware(Next2&& next)
      : next_(std::forward<Next2>(next))
  {
  }

  Next next_;
};

class exception_handler {
public:
  template <typename Request,
            typename Response,
            decay_to<exception_handler> Self,
            typename Next>
  friend auto
  tag_invoke(new_middleware_t<Request, Response>, Self&& self, Next&& next)
  {
    return std::forward<Self>(self)
        .template new_middleware_impl<Request, Response>(
            std::forward<Next>(next));
  }

private:
  template <typename Request, typename Response, typename Next>
  auto new_middleware_impl(Next&& next) const
  {
    return exception_handler_middleware<Request, Response, std::decay_t<Next>>(
        std::forward<Next>(next));
  }
};

#endif

}

FITORIA_NAMESPACE_END

#endif
