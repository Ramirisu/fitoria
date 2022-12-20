//
// Copyright (c) 2022 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include <fitoria/core/config.hpp>

#include <fitoria/core/handler_concept.hpp>

#include <functional>

FITORIA_NAMESPACE_BEGIN

namespace detail {

template <typename HandlerTrait>
class handlers_invoker {
public:
  handlers_invoker(const handlers_t<HandlerTrait>& handlers)
      : handlers_(handlers)
  {
  }

  template <typename Context>
  void start(Context& ctx)
  {
    curr_ = handlers_.begin();
    if (curr_ == handlers_.end()) {
      return;
    }

    std::invoke(*curr_, ctx);
  }

  template <typename Context>
  void next(Context& ctx)
  {
    ++curr_;
    if (curr_ == handlers_.end()) {
      return;
    }

    std::invoke(*curr_, ctx);
  }

private:
  const handlers_t<HandlerTrait>& handlers_;
  typename std::remove_cvref_t<decltype(handlers_)>::const_iterator curr_;
};

}

FITORIA_NAMESPACE_END
