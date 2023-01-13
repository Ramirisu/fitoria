//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include <fitoria/core/config.hpp>

#include <fitoria/core/net.hpp>

#include <fitoria/http_server/handler.hpp>
#include <fitoria/http_server/handlers_invoker.hpp>
#include <fitoria/http_server/http_request.hpp>
#include <fitoria/http_server/http_response.hpp>

FITORIA_NAMESPACE_BEGIN

class http_context {
public:
  using invoker_type
      = handlers_invoker<handler<http_context&, net::awaitable<http_response>>,
                         handler<http_request&, net::awaitable<http_response>>>;

  http_context(invoker_type invoker, http_request& request)
      : invoker_(invoker)
      , request_(request)
  {
  }

  [[nodiscard]] net::awaitable<http_response> next()
  {
    co_return co_await invoker_.next(*this);
  }

  http_request& request() noexcept
  {
    return request_;
  }

  const http_request& request() const noexcept
  {
    return request_;
  }

  operator http_request&() noexcept
  {
    return request_;
  }

  operator const http_request&() const noexcept
  {
    return request_;
  }

private:
  invoker_type invoker_;
  http_request& request_;
};

FITORIA_NAMESPACE_END
