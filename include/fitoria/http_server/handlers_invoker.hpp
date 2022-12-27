//
// Copyright (c) 2022 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include <fitoria/core/config.hpp>

#include <fitoria/core/handler_concept.hpp>
#include <fitoria/core/utility.hpp>

#include <fitoria/http_server/http_error.hpp>

#include <functional>
#include <variant>

FITORIA_NAMESPACE_BEGIN

template <typename HandlerTrait>
class handlers_invoker {
public:
  handlers_invoker(const handlers_t<HandlerTrait>& handlers)
      : handlers_(handlers)
  {
  }

  template <typename Context>
  handler_result_t<HandlerTrait> start(Context& ctx)
    requires(handler_result_awaitable<HandlerTrait>)
  {
    curr_ = handlers_.begin();
    FITORIA_ASSERT(curr_ != handlers_.end());
    co_return co_await invoke_awaitable(ctx);
  }

  template <typename Context>
  handler_result_t<HandlerTrait> start(Context& ctx)
    requires(!handler_result_awaitable<HandlerTrait>)
  {
    curr_ = handlers_.begin();
    FITORIA_ASSERT(curr_ != handlers_.end());
    invoke(ctx);
  }

  template <typename Context>
  handler_result_t<HandlerTrait> next(Context& ctx)
    requires(handler_result_awaitable<HandlerTrait>)
  {
    ++curr_;
    FITORIA_ASSERT(curr_ != handlers_.end());
    co_return co_await invoke_awaitable(ctx);
  }

  template <typename Context>
  handler_result_t<HandlerTrait> next(Context& ctx)
    requires(!handler_result_awaitable<HandlerTrait>)
  {
    ++curr_;
    FITORIA_ASSERT(curr_ != handlers_.end());
    invoke(ctx);
  }

private:
  template <typename Context>
  handler_result_t<HandlerTrait> invoke_awaitable(Context& ctx)
    requires(handler_result_awaitable<HandlerTrait>)
  {
    if constexpr (is_specialization_of<handler_t<HandlerTrait>,
                                       std::variant>::value) {
      co_return co_await invoke_awaitable_variant(ctx);
    } else {
      co_return co_await std::invoke(*curr_, ctx);
    }
  }

  template <typename Context>
  handler_result_t<HandlerTrait> invoke(Context& ctx)
    requires(!handler_result_awaitable<HandlerTrait>)
  {
    if constexpr (is_specialization_of<handler_t<HandlerTrait>,
                                       std::variant>::value) {
      invoke_variant(ctx);
    } else {
      std::invoke(*curr_, ctx);
    }
  }

  template <typename Context>
  handler_result_t<HandlerTrait> invoke_awaitable_variant(Context& ctx)
    requires(is_specialization_of<handler_t<HandlerTrait>, std::variant>::value)
  {
    static_assert(std::variant_size_v<handler_t<HandlerTrait>> <= 5);
    switch (curr_->index()) {
    case 0:
      co_return co_await std::invoke(std::get<0>(*curr_));
      break;
    case 1:
      co_return co_await std::invoke(std::get<1>(*curr_), ctx);
      break;
    case 2:
      co_return co_await std::invoke(std::get<2>(*curr_), ctx, ctx);
      break;
    case 3:
      co_return co_await std::invoke(std::get<3>(*curr_), ctx, ctx, ctx);
      break;
    case 4:
      co_return co_await std::invoke(std::get<4>(*curr_), ctx, ctx, ctx, ctx);
      break;
    }
  }

  template <typename Context>
  handler_result_t<HandlerTrait> invoke_variant(Context& ctx)
    requires(is_specialization_of<handler_t<HandlerTrait>, std::variant>::value)
  {
    static_assert(std::variant_size_v<handler_t<HandlerTrait>> <= 5);
    switch (curr_->index()) {
    case 0:
      std::invoke(std::get<0>(*curr_));
      break;
    case 1:
      std::invoke(std::get<1>(*curr_), ctx);
      break;
    case 2:
      std::invoke(std::get<2>(*curr_), ctx, ctx);
      break;
    case 3:
      std::invoke(std::get<3>(*curr_), ctx, ctx, ctx);
      break;
    case 4:
      std::invoke(std::get<4>(*curr_), ctx, ctx, ctx, ctx);
      break;
    }
  }

  const handlers_t<HandlerTrait>& handlers_;
  typename std::remove_cvref_t<decltype(handlers_)>::const_iterator curr_;
};

FITORIA_NAMESPACE_END
