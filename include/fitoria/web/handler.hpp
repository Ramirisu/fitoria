//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#ifndef FITORIA_WEB_HANDLER_HPP
#define FITORIA_WEB_HANDLER_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/core/net.hpp>
#include <fitoria/core/type_traits.hpp>

#include <fitoria/web/from_http_request.hpp>
#include <fitoria/web/http_request.hpp>
#include <fitoria/web/http_response.hpp>
#include <fitoria/web/middleware_concept.hpp>
#include <fitoria/web/to_http_response.hpp>

FITORIA_NAMESPACE_BEGIN

namespace web {

template <typename Next, typename Args>
class handler_middleware;

template <typename Next, typename... Args>
class handler_middleware<Next, std::tuple<Args...>> {
  friend class handler;

public:
  auto operator()(http_request& req) const -> net::awaitable<http_response>
  {
    co_return co_await invoke_with_args(
        co_await from_http_request<Args>(req)...);
  }

private:
  auto invoke_with_args(expected<Args, std::error_code>... args) const
      -> net::awaitable<http_response>
  {
    const auto pack
        = std::tuple<expected<Args, std::error_code>&...> { args... };
    if (auto err = get_error_of<0>(pack); err) {
      co_return http_response(http::status::internal_server_error)
          .set_field(http::field::content_type,
                     http::fields::content_type::plaintext())
          .set_body(err->message());
    }

    co_return to_http_response(
        co_await next_(std::forward<Args>(args.value())...));
  }

  template <std::size_t I>
  auto get_error_of(const std::tuple<expected<Args, std::error_code>&...>& args)
      const -> optional<std::error_code>
  {
    if constexpr (I < sizeof...(Args)) {
      if (auto& arg = std::get<I>(args); !arg) {
        return arg.error();
      }

      return get_error_of<I + 1>(args);
    } else {
      return nullopt;
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
  template <decay_to<handler> Self, typename Next>
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
