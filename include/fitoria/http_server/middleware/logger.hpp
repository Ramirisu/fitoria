//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include <fitoria/core/config.hpp>

#include <fitoria/core/log.hpp>
#include <fitoria/core/utility.hpp>

#include <fitoria/http_server/http_context.hpp>
#include <fitoria/http_server/http_response.hpp>

FITORIA_NAMESPACE_BEGIN

namespace middleware {

class logger {
public:
  net::awaitable<http_response> operator()(http_context& c) const
  {
    const auto& req = c.request();

    auto res = co_await c.next();

    log::info(name(), "{} {} {} {} {}",
              req.remote_endpoint().address().to_string(),
              std::string_view(to_string(req.method())), req.path(),
              to_underlying(res.status()),
              req.headers().get(http::field::user_agent).value_or(""));

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
