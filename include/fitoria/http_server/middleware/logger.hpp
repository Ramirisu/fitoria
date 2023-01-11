//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include <fitoria/core/config.hpp>

#include <fitoria/core/utility.hpp>

#include <fitoria/log/logger.hpp>

#include <fitoria/http_server/http_context.hpp>
#include <fitoria/http_server/http_response.hpp>

FITORIA_NAMESPACE_BEGIN

namespace middleware {

class logger {
public:
  net::awaitable<http_response> operator()(http_context& c) const
  {
    // request may be modified in later middlewares or handler, make a copy here
    auto addr = c.request().remote_endpoint().address().to_string();
    auto method = std::string(to_string(c.request().method()));
    auto path = c.request().path();
    auto user_agent
        = c.request().headers().get(http::field::user_agent).value_or("");
    auto start_time = c.request().start_time();

    auto res = co_await c.next();

    log::info("[{}] {} {} {} {} {} {:%T}", name(), addr, method, path,
              to_underlying(res.status()), user_agent,
              std::chrono::floor<std::chrono::microseconds>(
                  http_request::clock_t::now() - start_time));

    co_return res;
  }

private:
  static const char* name() noexcept
  {
    return "fitoria.middleware.logger";
  }
};

}

FITORIA_NAMESPACE_END
