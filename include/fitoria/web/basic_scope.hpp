//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef FITORIA_WEB_BASIC_SCOPE_HPP
#define FITORIA_WEB_BASIC_SCOPE_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/web/basic_route.hpp>

FITORIA_NAMESPACE_BEGIN

template <typename Middleware, typename Handler>
class basic_scope {
public:
  using route_type = basic_route<Middleware, Handler>;

  basic_scope(std::string path)
      : path_(std::move(path))
  {
  }

  auto use(Middleware middleware) -> basic_scope&
  {
    middlewares_.push_back(std::move(middleware));
    return *this;
  }

  auto route(const route_type& route) -> basic_scope&
  {
    std::string path = path_;
    path += route.path();
    auto middlewares = middlewares_;
    middlewares.insert(middlewares.end(), route.middlewares().begin(),
                       route.middlewares().end());
    routes_.push_back(route_type(route.method(), std::move(path),
                                 std::move(middlewares), route.handler()));
    return *this;
  }

  auto route(http::verb method, const std::string& path, Handler handler)
      -> basic_scope&
  {
    routes_.push_back(
        route_type(method, path_ + path, middlewares_, std::move(handler)));
    return *this;
  }

  auto sub_scope(basic_scope scope) -> basic_scope&
  {
    for (auto& routes : scope.routes_) {
      routes_.push_back(routes.rebind_parent(path_, middlewares_));
    }

    return *this;
  }

  auto routes() const noexcept -> const std::vector<route_type>&
  {
    return routes_;
  }

private:
  std::string path_;
  std::vector<Middleware> middlewares_;
  std::vector<route_type> routes_;
};

FITORIA_NAMESPACE_END

#endif
