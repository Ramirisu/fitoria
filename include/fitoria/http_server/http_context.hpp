//
// Copyright (c) 2022 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include <fitoria/core/config.hpp>

#include <fitoria/core/net.hpp>

#include <fitoria/http_server/detail/handler_trait.hpp>
#include <fitoria/http_server/detail/handlers_invoker.hpp>

#include <fitoria/http_server/http_request.hpp>
#include <fitoria/http_server/router.hpp>

#include <string_view>

FITORIA_NAMESPACE_BEGIN

class http_context {
  using handler_trait = detail::handler_trait;

public:
  http_context(detail::handlers_invoker<handler_trait> invoker,
               std::string_view path,
               http_request& request)
      : invoker_(std::move(invoker))
      , path_(path)
      , request_(request)
  {
  }

  http_request& request() noexcept
  {
    return request_;
  }

  const http_request& request() const noexcept
  {
    return request_;
  }

  std::string_view path() const noexcept
  {
    return path_;
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
  std::string_view path_;
  http_request& request_;
};

FITORIA_NAMESPACE_END
