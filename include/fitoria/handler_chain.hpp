//
// Copyright (c) 2022 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include <fitoria/core/config.hpp>

#include <functional>

FITORIA_NAMESPACE_BEGIN

template <typename Context>
concept handler_chain_context = requires(Context& ctx) { ctx.next(); };

template <typename HandlerTrait>
class handler_chain {

public:
  using handler_type = typename HandlerTrait::handler_type;
  using handlers_type = typename HandlerTrait::handlers_type;

  handler_chain(const handlers_type& handlers)
      : handlers_(handlers)
  {
  }

  template <handler_chain_context Context>
  void start(Context& ctx)
  {
    curr_ = handlers_.begin();
    if (curr_ == handlers_.end()) {
      return;
    }

    std::invoke(*curr_, ctx);
  }

  template <handler_chain_context Context>
  void next(Context& ctx)
  {
    ++curr_;
    if (curr_ == handlers_.end()) {
      return;
    }

    std::invoke(*curr_, ctx);
  }

private:
  const handlers_type& handlers_;
  typename std::remove_cvref_t<decltype(handlers_)>::const_iterator curr_;
};

FITORIA_NAMESPACE_END
