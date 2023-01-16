//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include <fitoria/core/config.hpp>

#include <fitoria/core/coroutine_concept.hpp>
#include <fitoria/core/type_traits.hpp>

#include <functional>
#include <type_traits>

FITORIA_NAMESPACE_BEGIN

template <typename Request, typename Response>
class basic_handler {
  template <typename R,
            typename F,
            typename T,
            typename Is = std::make_index_sequence<function_traits<F>::arity>>
  struct is_invocable_r;

  template <typename R, typename F, typename T, std::size_t... Is>
  struct is_invocable_r<R, F, T, std::index_sequence<Is...>> {
    template <auto>
    using rebind = T;
    static constexpr bool value = std::is_invocable_r_v<R, F, rebind<Is>...>;
  };

public:
  using request_type = Request;
  using response_type = Response;

  template <typename F>
  basic_handler(F f)
    requires(!std::is_same_v<F, basic_handler>
             && is_invocable_r<Response, F, Request>::value)
  {
    static constexpr auto arity = function_traits<F>::arity;
    static_assert(arity > 0 && arity <= 5);
    if constexpr (awaitable<Response>) {
      if constexpr (arity == 1) {
        handler_ = [f = std::move(f)](Request req) -> Response {
          co_return co_await f(req);
        };
      } else if constexpr (arity == 2) {
        handler_ = [f = std::move(f)](Request req) -> Response {
          co_return co_await f(req, req);
        };
      } else if constexpr (arity == 3) {
        handler_ = [f = std::move(f)](Request req) -> Response {
          co_return co_await f(req, req, req);
        };
      } else if constexpr (arity == 4) {
        handler_ = [f = std::move(f)](Request req) -> Response {
          co_return co_await f(req, req, req, req);
        };
      } else if constexpr (arity == 5) {
        handler_ = [f = std::move(f)](Request req) -> Response {
          co_return co_await f(req, req, req, req, req);
        };
      }
    } else {
      if constexpr (arity == 1) {
        handler_
            = [f = std::move(f)](Request req) -> Response { return f(req); };
      } else if constexpr (arity == 2) {
        handler_ = [f = std::move(f)](Request req) -> Response {
          return f(req, req);
        };
      } else if constexpr (arity == 3) {
        handler_ = [f = std::move(f)](Request req) -> Response {
          return f(req, req, req);
        };
      } else if constexpr (arity == 4) {
        handler_ = [f = std::move(f)](Request req) -> Response {
          return f(req, req, req, req);
        };
      } else if constexpr (arity == 5) {
        handler_ = [f = std::move(f)](Request req) -> Response {
          return f(req, req, req, req, req);
        };
      }
    }
  }

  Response operator()(Request request) const
    requires(awaitable<Response>)
  {
    co_return co_await handler_(request);
  }

  Response operator()(Request request) const
    requires(!awaitable<Response>)
  {
    return handler_(request);
  }

private:
  std::function<Response(Request)> handler_;
};

FITORIA_NAMESPACE_END
