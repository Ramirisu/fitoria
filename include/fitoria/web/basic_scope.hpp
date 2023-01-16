//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include <fitoria/core/config.hpp>

#include <fitoria/web/basic_router.hpp>

FITORIA_NAMESPACE_BEGIN

template <typename Middleware, typename Handler>
class basic_scope {
public:
  using router_type = basic_router<Middleware, Handler>;

  basic_scope(std::string path)
      : path_(std::move(path))
  {
  }

  auto use(Middleware middleware) -> basic_scope&
  {
    middlewares_.push_back(std::move(middleware));
    return *this;
  }

  auto route(const router_type& router) -> basic_scope&
  {
    std::string path = path_;
    path += router.path();
    auto middlewares = middlewares_;
    middlewares.insert(middlewares.end(), router.middlewares().begin(),
                       router.middlewares().end());
    routers_.push_back(router_type(router.method(), std::move(path),
                                   std::move(middlewares), router.handler()));
    return *this;
  }

  auto route(http::verb method, const std::string& path, Handler handler)
      -> basic_scope&
  {
    routers_.push_back(
        router_type(method, path_ + path, middlewares_, std::move(handler)));
    return *this;
  }

  auto sub_group(basic_scope rg) -> basic_scope&
  {
    for (auto& routers : rg.routers_) {
      routers_.push_back(routers.rebind_parent(path_, middlewares_));
    }

    return *this;
  }

  auto routers() const noexcept -> const std::vector<router_type>&
  {
    return routers_;
  }

private:
  std::string path_;
  std::vector<Middleware> middlewares_;
  std::vector<router_type> routers_;
};

FITORIA_NAMESPACE_END
