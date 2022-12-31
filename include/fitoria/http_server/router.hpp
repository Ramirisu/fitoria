//
// Copyright (c) 2022 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include <fitoria/core/config.hpp>

#include <fitoria/core/handler_concept.hpp>
#include <fitoria/core/http.hpp>

#include <fitoria/http_server/http_handler_trait.hpp>

#include <string>
#include <vector>

FITORIA_NAMESPACE_BEGIN

template <typename HandlerTrait>
class basic_router {
public:
  basic_router(http::verb method,
               std::string path,
               handler_t<HandlerTrait> handler)
      : method_(method)
      , path_(std::move(path))
      , handler_(std::move(handler))
  {
  }

  basic_router(http::verb method,
               std::string path,
               middlewares_t<HandlerTrait> middlewares,
               handler_t<HandlerTrait> handler)
      : method_(method)
      , path_(std::move(path))
      , middlewares_(std::move(middlewares))
      , handler_(std::move(handler))
  {
  }

  auto method() const noexcept -> http::verb
  {
    return method_;
  }

  auto path() const noexcept -> std::string_view
  {
    return path_;
  }

  auto middlewares() const noexcept -> const middlewares_t<HandlerTrait>&
  {
    return middlewares_;
  }

  auto handler() const noexcept -> const handler_t<HandlerTrait>&
  {
    return handler_;
  }

  auto
  rebind_parent(const std::string& parent_path,
                const middlewares_t<HandlerTrait>& parent_middlewares) const
      -> basic_router
  {
    auto middlewares = parent_middlewares;
    middlewares.insert(middlewares.end(), middlewares_.begin(),
                       middlewares_.end());
    return basic_router(method_, parent_path + path_, std::move(middlewares),
                        handler_);
  }

  friend auto operator==(const basic_router& lhs, const basic_router& rhs)
      -> bool
  {
    if (lhs.method() == rhs.method() && lhs.path() == rhs.path()
        && lhs.middlewares().size() == rhs.middlewares().size()) {
      for (std::size_t i = 0; i < lhs.middlewares().size(); ++i) {
        if (!middleware_compare_t<HandlerTrait>()(lhs.middlewares()[i],
                                                  rhs.middlewares()[i])) {
          return false;
        }
      }

      if (!handler_compare_t<HandlerTrait>()(lhs.handler(), rhs.handler())) {
        return false;
      }

      return true;
    }

    return false;
  }

private:
  http::verb method_;
  std::string path_;
  middlewares_t<HandlerTrait> middlewares_;
  handler_t<HandlerTrait> handler_;
};

using router = basic_router<http_handler_trait>;

FITORIA_NAMESPACE_END
