//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef FITORIA_WEB_MIDDLEWARE_EXCEPTION_HANDLER_HPP
#define FITORIA_WEB_MIDDLEWARE_EXCEPTION_HANDLER_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/log.hpp>

#include <fitoria/web/http_context.hpp>
#include <fitoria/web/http_response.hpp>
#include <fitoria/web/service.hpp>

FITORIA_NAMESPACE_BEGIN

namespace web::middleware {

#if !FITORIA_NO_EXCEPTIONS

template <typename Next>
class exception_handler_service {
  friend class exception_handler;

public:
  net::awaitable<http_response> operator()(http_context& c) const
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
  exception_handler_service(Next2&& next)
      : next_(std::forward<Next2>(next))
  {
  }

  Next next_;
};

template <typename Next>
exception_handler_service(Next&&)
    -> exception_handler_service<std::decay_t<Next>>;

class exception_handler {
  template <typename Next>
  auto new_service(Next&& next) const
  {
    return exception_handler_service(std::forward<Next>(next));
  }

public:
  template <typename Next>
  friend constexpr auto
  tag_invoke(tag_t<make_service>, const exception_handler& self, Next&& next)
  {
    return self.new_service(std::forward<Next>(next));
  }
};

#endif

}

FITORIA_NAMESPACE_END

#endif
