//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include <fitoria/core/config.hpp>

#include <fitoria/core/coroutine_concept.hpp>

#include <functional>
#include <type_traits>

FITORIA_NAMESPACE_BEGIN

template <typename Request, typename Response>
class basic_handler {
public:
  using request_type = Request;
  using response_type = Response;

  template <typename F>
  basic_handler(F&& f)
    requires(!std::is_same_v<std::remove_cvref_t<F>, basic_handler>
             && std::is_invocable_r_v<Response, F, Request>)
      : handler_(std::forward<F>(f))
  {
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
