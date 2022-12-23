//
// Copyright (c) 2022 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include <fitoria/core/config.hpp>

#include <fitoria/http_server/detail/handler_trait.hpp>

#include <fitoria/core/net.hpp>
#include <fitoria/http_server/detail/handlers_invoker.hpp>
#include <fitoria/http_server/router.hpp>

#include <functional>
#include <vector>

FITORIA_NAMESPACE_BEGIN

class http_context {
  using handler_trait = detail::handler_trait;
  using router_type = basic_router<handler_trait>;
  using native_request_t = http::request<http::string_body>;

public:
  class request_t {
  public:
    explicit request_t(native_request_t& native)
        : native_(native)
    {
    }

    methods method() const noexcept
    {
      return native_.method();
    }

    std::string& body() noexcept
    {
      return native_.body();
    }

    const std::string& body() const noexcept
    {
      return native_.body();
    }

  private:
    native_request_t& native_;
  };

  http_context(detail::handlers_invoker<handler_trait> invoker,
               const router_type& router,
               native_request_t& native_request)
      : invoker_(std::move(invoker))
      , router_(router)
      , native_request_(native_request)
  {
  }

  request_t request() const noexcept
  {
    return request_t(native_request_);
  }

  string_view path() const noexcept
  {
    return router_.path();
  }

  handler_result_t<handler_trait> start()
  {
    co_await invoker_.start(*this);
  }

  handler_result_t<handler_trait> next()
  {
    co_await invoker_.next(*this);
  }

private:
  detail::handlers_invoker<handler_trait> invoker_;
  const router_type& router_;
  native_request_t& native_request_;
};

FITORIA_NAMESPACE_END
