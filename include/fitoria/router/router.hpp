//
// Copyright (c) 2022 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include <fitoria/core/config.hpp>

#include <fitoria/core/method.hpp>

#include <string>
#include <vector>

FITORIA_NAMESPACE_BEGIN

template <typename HandlerTrait>
class router {
  using handler_compare = typename HandlerTrait::compare;

public:
  using handler_type = typename HandlerTrait::handler_type;
  using handlers_type = typename HandlerTrait::handlers_type;

  router(methods method, std::string path, handlers_type handlers)
      : method_(method)
      , path_(std::move(path))
      , handlers_(std::move(handlers))
  {
  }

  auto method() const noexcept -> methods
  {
    return method_;
  }

  auto path() const noexcept -> const std::string&
  {
    return path_;
  }

  auto handlers() const noexcept -> const handlers_type&
  {
    return handlers_;
  }

  auto rebind_parent(const std::string& parent_path,
                     const handlers_type& parent_handlers) const -> router
  {
    auto handlers = parent_handlers;
    handlers.insert(handlers.end(), handlers_.begin(), handlers_.end());
    return router(method_, parent_path + path_, std::move(handlers));
  }

  friend auto operator==(const router& lhs, const router& rhs) -> bool
  {
    handler_compare compare;
    if (lhs.method() == rhs.method() && lhs.path() == rhs.path()
        && lhs.handlers().size() == rhs.handlers().size()) {
      for (std::size_t i = 0; i < lhs.handlers().size(); ++i) {
        if (!compare(lhs.handlers()[i], rhs.handlers()[i])) {
          return false;
        }
      }

      return true;
    }

    return false;
  }

private:
  methods method_;
  std::string path_;
  handlers_type handlers_;
};

FITORIA_NAMESPACE_END
