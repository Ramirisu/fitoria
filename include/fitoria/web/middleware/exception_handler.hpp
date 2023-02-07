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

FITORIA_NAMESPACE_BEGIN

namespace web::middleware {

#if !FITORIA_NO_EXCEPTIONS

template <typename Next>
class exception_handler_service {
  Next next_;

public:
  exception_handler_service(Next next)
      : next_(std::move(next))
  {
  }

  net::awaitable<http_response> operator()(http_context& c) const
  {
    try {
      co_return co_await next_(c);
    } catch (const std::exception& ex) {
      log::error("[{}] exception: {}", name(), ex.what());
    }

    co_return http_response(http::status::internal_server_error);
  }

private:
  static const char* name() noexcept
  {
    return "fitoria.middleware.exception_handler";
  }
};

class exception_handler {
public:
  template <typename Next>
  auto create(Next next) const
  {
    return exception_handler_service(std::move(next));
  }
};

#endif

}

FITORIA_NAMESPACE_END

#endif
