//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef FITORIA_WEB_SCOPE_HPP
#define FITORIA_WEB_SCOPE_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/web/route.hpp>

FITORIA_NAMESPACE_BEGIN

namespace web {

template <typename Services, typename Routes>
class scope_impl;

template <typename... Services, typename... Routes>
class scope_impl<std::tuple<Services...>, std::tuple<Routes...>> {
  std::string path_;
  state_map state_map_;
  std::tuple<Services...> services_;
  std::tuple<Routes...> routes_;

public:
  scope_impl(std::string path,
             state_map state_map,
             std::tuple<Services...> services,
             std::tuple<Routes...> routes)
      : path_(std::move(path))
      , state_map_(std::move(state_map))
      , services_(std::move(services))
      , routes_(std::move(routes))
  {
  }

  template <typename State>
  auto state(State state) const
  {
    auto state_map = state_map_;
    state_map[std::type_index(typeid(State))] = std::any(std::move(state));
    return scope_impl<std::tuple<Services...>, std::tuple<Routes...>>(
        path_, std::move(state_map), services_, routes_);
  }

  template <typename Service>
  auto use(Service service) const
  {
    return scope_impl<std::tuple<Services..., Service>, std::tuple<Routes...>>(
        path_, state_map_,
        std::tuple_cat(services_, std::tuple { std::move(service) }), routes_);
  }

  template <typename... RouteServices, typename Handler>
  auto route(route_impl<std::tuple<RouteServices...>, Handler> r) const
  {
    auto r2 = r.rebind_parent(path_, state_map_, services_);
    return scope_impl<std::tuple<Services...>,
                      std::tuple<Routes..., decltype(r2)>> {
      path_,
      state_map_,
      services_,
      std::tuple_cat(routes_, std::tuple { std::move(r2) }),
    };
  }

  template <typename Handler>
  auto handle(http::verb method, std::string path, Handler handler) const
  {
    return route(route::handle(method, std::move(path), std::move(handler)));
  }

  template <typename Handler>
  auto GET(std::string path, Handler handler) const
  {
    return handle(http::verb::get, std::move(path), std::move(handler));
  }

  template <typename Handler>
  auto POST(std::string path, Handler handler) const
  {
    return handle(http::verb::post, std::move(path), std::move(handler));
  }

  template <typename Handler>
  auto PUT(std::string path, Handler handler) const
  {
    return handle(http::verb::put, std::move(path), std::move(handler));
  }

  template <typename Handler>
  auto PATCH(std::string path, Handler handler) const
  {
    return handle(http::verb::patch, std::move(path), std::move(handler));
  }

  template <typename Handler>
  auto DELETE_(std::string path, Handler handler) const
  {
    return handle(http::verb::delete_, std::move(path), std::move(handler));
  }

  template <typename Handler>
  auto HEAD(std::string path, Handler handler) const
  {
    return handle(http::verb::head, std::move(path), std::move(handler));
  }

  template <typename Handler>
  auto OPTIONS(std::string path, Handler handler) const
  {
    return handle(http::verb::options, std::move(path), std::move(handler));
  }

  template <typename... ChildServices, typename... ChildRoutes>
  auto sub_scope(scope_impl<std::tuple<ChildServices...>,
                            std::tuple<ChildRoutes...>> child) const
  {
    auto routes = std::apply(
        [this](auto&&... routes_) {
          return std::tuple { routes_.rebind_parent(path_, state_map_,
                                                    services_)... };
        },
        child.routes());

    return scope_impl<std::tuple<Services...>,
                      decltype(std::tuple_cat(routes_, std::move(routes)))> {
      path_,
      state_map_,
      services_,
      std::tuple_cat(routes_, std::move(routes)),
    };
  }

  auto routes() const
  {
    return routes_;
  }
};

inline auto scope(std::string path)
{
  return scope_impl<std::tuple<>, std::tuple<>>(std::move(path), {}, {}, {});
}

}

FITORIA_NAMESPACE_END

#endif
