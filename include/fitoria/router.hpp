//
// Copyright (c) 2022 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include <fitoria/config.hpp>

#include <fitoria/method.hpp>

#include <string>
#include <vector>

FITORIA_NAMESPACE_BEGIN

template <typename HandlerTrait>
class router {
  using handler_compare = typename HandlerTrait::compare;

public:
  using handler_type = typename HandlerTrait::handler_type;

  router(methods method,
         std::string path,
         std::vector<handler_type> middlewares,
         handler_type handler)
      : method_(method)
      , path_(std::move(path))
      , middlewares_(std::move(middlewares))
      , handler_(std::move(handler))
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

  auto middlewares() const noexcept -> const std::vector<handler_type>&
  {
    return middlewares_;
  }

  auto handler() const noexcept -> const handler_type&
  {
    return handler_;
  }

  auto bind_parent(const std::string& parent_path,
                   const std::vector<handler_type>& parent_middlewares) const
      -> router
  {
    auto middlewares = parent_middlewares;
    middlewares.insert(middlewares.end(), middlewares_.begin(),
                       middlewares_.end());
    return router(method_, parent_path + path_, std::move(middlewares),
                  handler_);
  }

  friend auto operator==(const router& lhs, const router& rhs) -> bool
  {
    handler_compare compare;
    if (lhs.method() == rhs.method() && lhs.path() == rhs.path()
        && lhs.middlewares().size() == rhs.middlewares().size()
        && compare(lhs.handler(), rhs.handler())) {
      for (std::size_t i = 0; i < lhs.middlewares().size(); ++i) {
        if (!compare(lhs.middlewares()[i], rhs.middlewares()[i])) {
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
  std::vector<handler_type> middlewares_;
  handler_type handler_;
};

FITORIA_NAMESPACE_END
