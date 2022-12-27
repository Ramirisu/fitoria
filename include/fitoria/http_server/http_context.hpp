//
// Copyright (c) 2022 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include <fitoria/core/config.hpp>

#include <fitoria/core/http.hpp>
#include <fitoria/core/json.hpp>
#include <fitoria/core/net.hpp>

#include <fitoria/http_server/handlers_invoker.hpp>
#include <fitoria/http_server/http_handler_trait.hpp>

#include <fitoria/http_server/http_request.hpp>
#include <fitoria/http_server/http_route.hpp>
#include <fitoria/http_server/router.hpp>

#include <string_view>

FITORIA_NAMESPACE_BEGIN

class http_context {
  using handler_trait = http_handler_trait;

public:
  http_context(handlers_invoker<handler_trait> invoker,
               http_route& route,
               http_request& request)
      : invoker_(std::move(invoker))
      , route_(route)
      , request_(request)
  {
  }

  http_route& route() noexcept
  {
    return route_;
  }

  const http_route& route() const noexcept
  {
    return route_;
  }

  operator http_route&() noexcept
  {
    return route_;
  }

  operator const http_route&() const noexcept
  {
    return route_;
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

  handler_result_t<handler_trait> start()
  {
    co_return co_await invoker_.start(*this);
  }

  handler_result_t<handler_trait> next()
  {
    co_return co_await invoker_.next(*this);
  }

private:
  handlers_invoker<handler_trait> invoker_;
  http_route& route_;
  http_request& request_;
};

FITORIA_NAMESPACE_END
