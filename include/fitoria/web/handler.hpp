//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

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
#include <fitoria/web/service.hpp>

FITORIA_NAMESPACE_BEGIN

namespace web {

template <typename Next, typename Args>
class handler_service;

template <typename Next, typename... Args>
class handler_service<Next, std::tuple<Args...>> {
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
    co_return co_await invoke_with_args_expansion_impl<0>(
        std::tuple { co_await from_http_request_t<Args> {}(
            static_cast<http_request&>(ctx))... });
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
  handler_service(Next2&& next)
      : next_(std::forward<Next2>(next))
  {
  }

  Next next_;
};

class handler {
  template <typename Next>
  auto new_service(Next&& next) const
  {
    return handler_service<std::decay_t<Next>,
                           typename function_traits<Next>::args_type>(
        std::forward<Next>(next));
  }

public:
  template <uncvref_same_as<handler> Self, typename Next>
  friend constexpr auto tag_invoke(make_service_t, Self&& self, Next&& next)
  {
    return std::forward<Self>(self).new_service(std::forward<Next>(next));
  }
};
}

FITORIA_NAMESPACE_END

#endif
