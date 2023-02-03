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

class exception_handler {
public:
  net::awaitable<http_response> operator()(http_context& c) const
  {
    try {
      co_return co_await c.next();
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

#endif

}

FITORIA_NAMESPACE_END

#endif
