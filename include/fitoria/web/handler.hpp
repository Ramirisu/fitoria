//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#ifndef FITORIA_WEB_HANDLER_HPP
#define FITORIA_WEB_HANDLER_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/core/lazy.hpp>
#include <fitoria/core/net.hpp>
#include <fitoria/core/type_traits.hpp>

#include <fitoria/web/from_http_request.hpp>
#include <fitoria/web/http_context.hpp>
#include <fitoria/web/http_request.hpp>
#include <fitoria/web/http_response.hpp>
#include <fitoria/web/middleware_concept.hpp>

FITORIA_NAMESPACE_BEGIN

namespace web {

template <typename Next, typename Args>
class handler_middleware;

template <typename Next, typename... Args>
class handler_middleware<Next, std::tuple<Args...>> {
  friend class handler;

public:
  auto operator()(http_context& ctx) const -> lazy<http_response>
  {
    return invoke_with_args_expansion(ctx);
  }

private:
  auto invoke_with_args_expansion(http_context& ctx) const
      -> lazy<http_response>
  {
    co_return co_await invoke_with_args_expansion_impl<0>(std::tuple {
        co_await from_http_request<Args>(static_cast<http_request&>(ctx))... });
  }

  template <std::size_t I>
  auto invoke_with_args_expansion_impl(
      std::tuple<expected<Args, error_code>...> args) const
      -> lazy<http_response>
  {
    if constexpr (I < sizeof...(Args)) {
      if (auto& arg = std::get<I>(args); !arg) {
        return [](auto&&) -> lazy<http_response> {
          co_return http_response(http::status::bad_request);
        }(std::move(arg).error());
      } else {
        return invoke_with_args_expansion_impl<I + 1>(std::move(args));
      }
    } else {
      return std::apply(
          [&](auto... args) { return next_(std::move(args).value()...); },
          std::move(args));
    }
  }

  template <typename Next2>
  handler_middleware(Next2&& next)
      : next_(std::forward<Next2>(next))
  {
  }

  Next next_;
};

class handler {
public:
  template <uncvref_same_as<handler> Self, typename Next>
  friend constexpr auto tag_invoke(new_middleware_t, Self&& self, Next&& next)
  {
    return std::forward<Self>(self).new_middleware_impl(
        std::forward<Next>(next));
  }

private:
  template <typename Next>
  auto new_middleware_impl(Next&& next) const
  {
    return handler_middleware<std::decay_t<Next>,
                              typename function_traits<Next>::args_type>(
        std::forward<Next>(next));
  }
};
}

FITORIA_NAMESPACE_END

#endif
