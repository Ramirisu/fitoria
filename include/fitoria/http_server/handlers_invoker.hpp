//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include <fitoria/core/config.hpp>

#include <fitoria/core/handler_concept.hpp>
#include <fitoria/core/type_traits.hpp>

#include <functional>

FITORIA_NAMESPACE_BEGIN

template <typename HandlerTrait>
class handlers_invoker {

public:
  handlers_invoker(const middlewares_t<HandlerTrait>& middlewares,
                   const handler_t<HandlerTrait>& handler)
      : middlewares_(middlewares)
      , curr_(middlewares_.begin())
      , handler_(handler)
  {
  }

  middleware_result_t<HandlerTrait>
  next(middleware_input_param_t<HandlerTrait>& ctx)
    requires(handler_result_awaitable<HandlerTrait>)
  {
    if (curr_ != middlewares_.end()) {
      co_return co_await (*curr_++)(ctx);
    } else {
      co_return co_await invoke_awaitable_handler(ctx);
    }
  }

  middleware_result_t<HandlerTrait>
  next(middleware_input_param_t<HandlerTrait>& ctx)
    requires(!handler_result_awaitable<HandlerTrait>)
  {
    if (curr_ != middlewares_.end()) {
      (*curr_++)(ctx);
    } else {
      invoke_handler(ctx);
    }
  }

private:
  handler_result_t<HandlerTrait>
  invoke_awaitable_handler(handler_input_param_t<HandlerTrait>& ctx)
    requires(handler_result_awaitable<HandlerTrait>
             && !is_variant_v<handler_t<HandlerTrait>>)
  {
    co_return co_await std::invoke(handler_, ctx);
  }

  handler_result_t<HandlerTrait>
  invoke_handler(handler_input_param_t<HandlerTrait>& ctx)
    requires(!handler_result_awaitable<HandlerTrait>
             && !is_variant_v<handler_t<HandlerTrait>>)
  {
    std::invoke(handler_, ctx);
  }

  handler_result_t<HandlerTrait>
  invoke_awaitable_handler(handler_input_param_t<HandlerTrait>& ctx)
    requires(handler_result_awaitable<HandlerTrait>
             && is_variant_v<handler_t<HandlerTrait>>)
  {
    static_assert(std::variant_size_v<handler_t<HandlerTrait>> <= 5);
    switch (handler_.index()) {
    case 0:
      co_return co_await std::invoke(std::get<0>(handler_));
    case 1:
      co_return co_await std::invoke(std::get<1>(handler_), ctx);
    case 2:
      co_return co_await std::invoke(std::get<2>(handler_), ctx, ctx);
    case 3:
      co_return co_await std::invoke(std::get<3>(handler_), ctx, ctx, ctx);
    case 4:
      co_return co_await std::invoke(std::get<4>(handler_), ctx, ctx, ctx, ctx);
    default:
      break;
    }
  }

  handler_result_t<HandlerTrait>
  invoke_handler(handler_input_param_t<HandlerTrait>& ctx)
    requires(!handler_result_awaitable<HandlerTrait>
             && is_variant_v<handler_t<HandlerTrait>>)
  {
    static_assert(std::variant_size_v<handler_t<HandlerTrait>> <= 5);
    switch (handler_.index()) {
    case 0:
      std::invoke(std::get<0>(handler_));
      return;
    case 1:
      std::invoke(std::get<1>(handler_), ctx);
      return;
    case 2:
      std::invoke(std::get<2>(handler_), ctx, ctx);
      return;
    case 3:
      std::invoke(std::get<3>(handler_), ctx, ctx, ctx);
      return;
    case 4:
      std::invoke(std::get<4>(handler_), ctx, ctx, ctx, ctx);
      return;
    default:
      break;
    }
  }

  const middlewares_t<HandlerTrait>& middlewares_;
  middlewares_t<HandlerTrait>::const_iterator curr_;
  const handler_t<HandlerTrait>& handler_;
};

FITORIA_NAMESPACE_END
