//
// Copyright (c) 2022 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include <fitoria/core/config.hpp>

#include <fitoria/http_server/detail/handler_trait.hpp>

#include <fitoria/http_server/router.hpp>

FITORIA_NAMESPACE_BEGIN

template <typename HandlerTrait>
class basic_router_group {
public:
  using router_type = basic_router<HandlerTrait>;

  basic_router_group(std::string path)
      : path_(std::move(path))
  {
  }

  auto use(handler_t<HandlerTrait> middleware) -> basic_router_group&
  {
    middlewares_.push_back(std::move(middleware));
    return *this;
  }

  auto route(verb method,
             const std::string& path,
             handler_t<HandlerTrait> handler) -> basic_router_group&
  {
    auto handlers = middlewares_;
    handlers.push_back(std::move(handler));
    routers_.push_back(router_type(method, path_ + path, std::move(handlers)));
    return *this;
  }

  auto sub_group(basic_router_group rg) -> basic_router_group&
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

using router_group = basic_router_group<detail::handler_trait>;

FITORIA_NAMESPACE_END
