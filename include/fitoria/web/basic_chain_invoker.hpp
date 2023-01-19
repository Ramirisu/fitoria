//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef FITORIA_WEB_BASIC_CHAIN_INVOKER_HPP
#define FITORIA_WEB_BASIC_CHAIN_INVOKER_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/core/coroutine_concept.hpp>

#include <functional>
#include <vector>

FITORIA_NAMESPACE_BEGIN

template <typename Middleware, typename Handler>
class basic_chain_invoker {

public:
  basic_chain_invoker(const std::vector<Middleware>& middlewares,
                      const Handler& handler)
      : middlewares_(middlewares)
      , curr_(middlewares_.begin())
      , handler_(handler)
  {
  }

  typename Middleware::response_type next(typename Middleware::request_type ctx)
    requires(awaitable<typename Middleware::response_type>)
  {
    if (curr_ != middlewares_.end()) {
      co_return co_await (*curr_++)(ctx);
    } else {
      co_return co_await invoke_handler(ctx);
    }
  }

  typename Middleware::response_type next(typename Middleware::request_type ctx)
    requires(!awaitable<typename Middleware::response_type>)
  {
    if (curr_ != middlewares_.end()) {
      return (*curr_++)(ctx);
    } else {
      return invoke_handler(ctx);
    }
  }

private:
  typename Handler::response_type
  invoke_handler(typename Handler::request_type request)
    requires(awaitable<typename Handler::response_type>)
  {
    co_return co_await std::invoke(handler_, request);
  }

  typename Handler::response_type
  invoke_handler(typename Handler::request_type request)
    requires(!awaitable<typename Handler::response_type>)
  {
    return std::invoke(handler_, request);
  }

  const std::vector<Middleware>& middlewares_;
  typename std::vector<Middleware>::const_iterator curr_;
  const Handler& handler_;
};

FITORIA_NAMESPACE_END

#endif
