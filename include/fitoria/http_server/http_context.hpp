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

#include <fitoria/http_server/detail/handler_trait.hpp>
#include <fitoria/http_server/detail/handlers_invoker.hpp>

#include <fitoria/http_server/http_request.hpp>
#include <fitoria/http_server/http_route.hpp>
#include <fitoria/http_server/router.hpp>

#include <string_view>

FITORIA_NAMESPACE_BEGIN

class http_context {
  using handler_trait = detail::handler_trait;
  using native_response_type = http::response<http::string_body>;

public:
  http_context(detail::handlers_invoker<handler_trait> invoker,
               http_route& route,
               http_request& request,
               native_response_type& response)
      : invoker_(std::move(invoker))
      , route_(route)
      , request_(request)
      , response_(response)
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

  http_context& status(status s)
  {
    response_.result(s);
    return *this;
  }

  http_context& plain_text(std::string s)
  {
    response_.result(status::ok);
    response_.insert(field::content_type, plain_text_content_type());
    response_.body() = std::move(s);
    return *this;
  }

  http_context& json(json::value json)
  {
    response_.result(status::ok);
    response_.insert(field::content_type, application_json_content_type());
    response_.body() = json::serialize(json);
    return *this;
  }

  std::string_view plain_text_content_type()
  {
    return "text/plain; charset=utf-8";
  }

  std::string_view application_json_content_type()
  {
    return "application/json; charset=utf-8";
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
  http_route& route_;
  http_request& request_;
  native_response_type& response_;
};

FITORIA_NAMESPACE_END
