//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include <fitoria/core/config.hpp>

#include <fitoria/core/handler_concept.hpp>

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
    requires(handler_result_awaitable<HandlerTrait>)
  {
    co_return co_await std::invoke(handler_, ctx);
  }

  handler_result_t<HandlerTrait>
  invoke_handler(handler_input_param_t<HandlerTrait>& ctx)
    requires(!handler_result_awaitable<HandlerTrait>)
  {
    std::invoke(handler_, ctx);
  }

  const middlewares_t<HandlerTrait>& middlewares_;
  middlewares_t<HandlerTrait>::const_iterator curr_;
  const handler_t<HandlerTrait>& handler_;
};

FITORIA_NAMESPACE_END
