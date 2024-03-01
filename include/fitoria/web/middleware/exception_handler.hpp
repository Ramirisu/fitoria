//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
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

#include <fitoria/web/http_context.hpp>
#include <fitoria/web/http_response.hpp>
#include <fitoria/web/middleware_concept.hpp>

FITORIA_NAMESPACE_BEGIN

namespace web::middleware {

#if !FITORIA_NO_EXCEPTIONS

template <typename Next>
class exception_handler_middleware {
  friend class exception_handler;

public:
  auto operator()(http_context& c) const -> lazy<http_response>
  {
    try {
      co_return co_await next_(c);
    } catch (const std::exception& ex) {
      log::error("[fitoria.middleware.exception_handler] exception: {}",
                 ex.what());
    }

    co_return http_response(http::status::internal_server_error);
  }

private:
  template <typename Next2>
  exception_handler_middleware(Next2&& next)
      : next_(std::forward<Next2>(next))
  {
  }

  Next next_;
};

template <typename Next>
exception_handler_middleware(Next&&)
    -> exception_handler_middleware<std::decay_t<Next>>;

class exception_handler {
public:
  template <uncvref_same_as<exception_handler> Self, typename Next>
  friend constexpr auto tag_invoke(new_middleware_t, Self&& self, Next&& next)
  {
    return std::forward<Self>(self).new_middleware_impl(
        std::forward<Next>(next));
  }

private:
  template <typename Next>
  auto new_middleware_impl(Next&& next) const
  {
    return exception_handler_middleware(std::forward<Next>(next));
  }
};

#endif

}

FITORIA_NAMESPACE_END

#endif
