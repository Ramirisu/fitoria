//
// Copyright (c) 2022 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include <fitoria/core/config.hpp>

#include <fitoria/http_server/detail/handlers_invoker.hpp>

#include <functional>
#include <vector>

FITORIA_NAMESPACE_BEGIN

class http_context;

struct handler_trait {
  using handler_t = std::function<void(http_context&)>;
  using handlers_t = std::vector<handler_t>;
  struct handler_compare_t;
};

class http_context {
public:
  explicit http_context(detail::handlers_invoker<handler_trait> invoker)
      : invoker_(std::move(invoker))
  {
  }

  void start()
  {
    invoker_.start(*this);
  }

  void next()
  {
    invoker_.next(*this);
  }

private:
  detail::handlers_invoker<handler_trait> invoker_;
};

FITORIA_NAMESPACE_END
