//
// Copyright (c) 2022 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include <fitoria/config.hpp>

#include <fitoria/handler.hpp>
#include <fitoria/method.hpp>
#include <fitoria/middleware.hpp>

#include <string>
#include <vector>

FITORIA_NAMESPACE_BEGIN

class router {
  friend class router_group;

public:
  router(methods method,
         std::string path,
         std::vector<middleware_t> middlewares,
         handler_t handler)
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

  friend auto operator==(const router& lhs, const router& rhs) -> bool
  {
    if (lhs.method() == rhs.method() && lhs.path() == rhs.path()
        && lhs.middlewares_.size() == rhs.middlewares_.size()
        && lhs.handler_() == rhs.handler_()) {
      for (std::size_t i = 0; i < lhs.middlewares_.size(); ++i) {
        if (lhs.middlewares_[i](0) != rhs.middlewares_[i](0)) {
          return false;
        }
      }

      return true;
    }

    return false;
  }

private:
  auto bind_parent(const std::string& parent_path,
                   const std::vector<middleware_t>& parent_middlewares) const
      -> router
  {
    auto middlewares = parent_middlewares;
    middlewares.insert(middlewares.end(), middlewares_.begin(),
                       middlewares_.end());
    return router(method_, parent_path + path_, std::move(middlewares),
                  handler_);
  }

  methods method_;
  std::string path_;
  std::vector<middleware_t> middlewares_;
  handler_t handler_;
};

FITORIA_NAMESPACE_END
