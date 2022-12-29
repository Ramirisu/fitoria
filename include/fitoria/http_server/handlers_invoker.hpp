//
// Copyright (c) 2022 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include <fitoria/core/config.hpp>

#include <fitoria/core/handler_concept.hpp>
#include <fitoria/core/type_traits.hpp>
#include <fitoria/core/utility.hpp>

#include <functional>
#include <variant>

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
      co_return co_await invoke_awaitable(ctx);
    }
  }

  middleware_result_t<HandlerTrait>
  next(middleware_input_param_t<HandlerTrait>& ctx)
    requires(!handler_result_awaitable<HandlerTrait>)
  {
    if (curr_ != middlewares_.end()) {
      (*curr_++)(ctx);
    } else {
      invoke(ctx);
    }
  }

private:
  handler_result_t<HandlerTrait>
  invoke_awaitable(handler_input_param_t<HandlerTrait>& ctx)
    requires(handler_result_awaitable<HandlerTrait>)
  {
    if constexpr (is_specialization_of<handler_t<HandlerTrait>,
                                       std::variant>::value) {
      co_return co_await invoke_awaitable_variant(ctx);
    } else {
      co_return co_await std::invoke(handler_, ctx);
    }
  }

  handler_result_t<HandlerTrait>
  invoke(handler_input_param_t<HandlerTrait>& ctx)
    requires(!handler_result_awaitable<HandlerTrait>)
  {
    if constexpr (is_specialization_of_v<handler_t<HandlerTrait>,
                                         std::variant>) {
      invoke_variant(ctx);
    } else {
      std::invoke(handler_, ctx);
    }
  }

  handler_result_t<HandlerTrait>
  invoke_awaitable_variant(handler_input_param_t<HandlerTrait>& ctx)
    requires(is_specialization_of_v<handler_t<HandlerTrait>, std::variant>)
  {
    static_assert(std::variant_size_v<handler_t<HandlerTrait>> <= 5);
    switch (handler_.index()) {
    case 0:
      co_return co_await std::invoke(std::get<0>(handler_));
      break;
    case 1:
      co_return co_await std::invoke(std::get<1>(handler_), ctx);
      break;
    case 2:
      co_return co_await std::invoke(std::get<2>(handler_), ctx, ctx);
      break;
    case 3:
      co_return co_await std::invoke(std::get<3>(handler_), ctx, ctx, ctx);
      break;
    case 4:
      co_return co_await std::invoke(std::get<4>(handler_), ctx, ctx, ctx, ctx);
      break;
    }
  }

  handler_result_t<HandlerTrait>
  invoke_variant(handler_input_param_t<HandlerTrait>& ctx)
    requires(is_specialization_of_v<handler_t<HandlerTrait>, std::variant>)
  {
    static_assert(std::variant_size_v<handler_t<HandlerTrait>> <= 5);
    switch (handler_.index()) {
    case 0:
      std::invoke(std::get<0>(handler_));
      break;
    case 1:
      std::invoke(std::get<1>(handler_), ctx);
      break;
    case 2:
      std::invoke(std::get<2>(handler_), ctx, ctx);
      break;
    case 3:
      std::invoke(std::get<3>(handler_), ctx, ctx, ctx);
      break;
    case 4:
      std::invoke(std::get<4>(handler_), ctx, ctx, ctx, ctx);
      break;
    }
  }

  const middlewares_t<HandlerTrait>& middlewares_;
  middlewares_t<HandlerTrait>::const_iterator curr_;
  const handler_t<HandlerTrait>& handler_;
};

FITORIA_NAMESPACE_END
