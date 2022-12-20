//
// Copyright (c) 2022 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include <fitoria/core/config.hpp>

#include <fitoria/router/router.hpp>

FITORIA_NAMESPACE_BEGIN

template <typename HandlerTrait>
class router_group {
public:
  using router_type = router<HandlerTrait>;

  router_group(std::string path)
      : path_(std::move(path))
  {
  }

  auto use(handler_t<HandlerTrait> middleware) -> router_group&
  {
    middlewares_.push_back(std::move(middleware));
    return *this;
  }

  auto route(methods method,
             const std::string& path,
             handler_t<HandlerTrait> handler) -> router_group&
  {
    auto handlers = middlewares_;
    handlers.push_back(std::move(handler));
    routers_.push_back(router_type(method, path_ + path, std::move(handlers)));
    return *this;
  }

  auto sub_group(router_group rg) -> router_group&
  {
    for (auto& routers : rg.routers_) {
      routers_.push_back(routers.rebind_parent(path_, middlewares_));
    }

    return *this;
  }

  auto get_all_routers() const -> std::vector<router_type>
  {
    return routers_;
  }

private:
  std::string path_;
  handlers_t<HandlerTrait> middlewares_;
  std::vector<router_type> routers_;
};

FITORIA_NAMESPACE_END
