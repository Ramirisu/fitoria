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

#include <fitoria/web/from_request.hpp>
#include <fitoria/web/request.hpp>
#include <fitoria/web/response.hpp>
#include <fitoria/web/to_middleware.hpp>
#include <fitoria/web/to_response.hpp>

FITORIA_NAMESPACE_BEGIN

namespace web {

template <typename Request, typename Response, typename Next, typename Args>
class handler_middleware;

template <typename Request, typename Response, typename Next, typename... Args>
class handler_middleware<Request, Response, Next, std::tuple<Args...>> {
  friend class handler;

public:
  auto operator()(Request) const -> Response
    requires(sizeof...(Args) == 0)
  {
    co_return to_response(co_await next_());
  }

  auto operator()(Request req) const -> Response
    requires(sizeof...(Args) > 0)
  {
    auto args = std::tuple<optional<Args>...>();
    if (auto res = co_await extract_arg<0>(req, args); res) {
      co_return std::move(*res);
    } else {
      co_return co_await std::apply(
          [this](auto&&... args) -> Response {
            co_return to_response(co_await next_(std::forward<Args>(*args)...));
          },
          args);
    }
  }

  template <std::size_t I>
  auto extract_arg(Request req, std::tuple<optional<Args>...>& args) const
      -> awaitable<optional<response>>
  {
    if constexpr (I < sizeof...(Args)) {
      if (auto result = co_await extract_arg_impl<
              std::tuple_element_t<I, std::tuple<Args...>>>(req,
                                                            std::get<I>(args));
          result) {
        co_return result;
      } else {
        co_return co_await extract_arg<I + 1>(req, args);
      }
    } else {
      co_return nullopt;
    }
  }

  template <typename Arg>
  auto extract_arg_impl(Request req, optional<Arg>& arg) const
      -> awaitable<optional<response>>
  {
    if (auto result = co_await from_request<Arg>(req); result) {
      arg.emplace(*result);
      co_return nullopt;
    } else {
      co_return std::move(result.error());
    }
  }

private:
  template <typename Next2>
  handler_middleware(construct_t, Next2&& next)
      : next_(std::forward<Next2>(next))
  {
  }

  Next next_;
};

class handler {
public:
  template <typename Request,
            typename Response,
            decay_to<handler> Self,
            typename Next>
  friend auto
  tag_invoke(to_middleware_t<Request, Response>, Self&& self, Next&& next)
  {
    return std::forward<Self>(self)
        .template to_middleware_impl<Request, Response>(
            std::forward<Next>(next));
  }

private:
  template <typename Request, typename Response, typename Next>
  auto to_middleware_impl(Next&& next) const
  {
    return handler_middleware<Request,
                              Response,
                              std::decay_t<Next>,
                              typename function_traits<Next>::args_type>(
        construct_t {}, std::forward<Next>(next));
  }
};
}

FITORIA_NAMESPACE_END

#endif
