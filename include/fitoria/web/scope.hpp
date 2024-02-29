//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#ifndef FITORIA_WEB_SCOPE_HPP
#define FITORIA_WEB_SCOPE_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/web/route.hpp>

FITORIA_NAMESPACE_BEGIN

namespace web {

template <basic_fixed_string Path, typename Services, typename Routes>
class scope_impl;

template <basic_fixed_string Path, typename... Services, typename... Routes>
class scope_impl<Path, std::tuple<Services...>, std::tuple<Routes...>> {
  state_map state_map_;
  std::tuple<Services...> services_;
  std::tuple<Routes...> routes_;

public:
  scope_impl(state_map state_map,
             std::tuple<Services...> services,
             std::tuple<Routes...> routes)
      : state_map_(std::move(state_map))
      , services_(std::move(services))
      , routes_(std::move(routes))
  {
  }

  template <typename State>
  auto state(State&& state) const
  {
    auto state_map = state_map_;
    state_map[std::type_index(typeid(State))]
        = std::any(std::forward<State>(state));
    return scope_impl<Path, std::tuple<Services...>, std::tuple<Routes...>>(
        std::move(state_map), services_, routes_);
  }

  template <typename Service>
  auto use(Service&& service) const
  {
    return scope_impl<Path,
                      std::tuple<Services..., std::decay_t<Service>>,
                      std::tuple<Routes...>>(
        state_map_,
        std::tuple_cat(services_,
                       std::tuple { std::forward<Service>(service) }),
        routes_);
  }

  template <basic_fixed_string RoutePath,
            typename... RouteServices,
            typename Handler>
  auto
  route(route_builder<RoutePath, std::tuple<RouteServices...>, Handler> r) const
  {
    auto r2 = r.template rebind_parent<Path>(state_map_, services_);

    return scope_impl<Path,
                      std::tuple<Services...>,
                      std::tuple<Routes..., decltype(r2)>> {
      state_map_,
      services_,
      std::tuple_cat(routes_, std::tuple { std::move(r2) }),
    };
  }

  template <basic_fixed_string RoutePath, typename Handler>
  auto handle(http::verb method, Handler&& handler) const
  {
    return route(
        route::handle<RoutePath>(method, std::forward<Handler>(handler)));
  }

  template <basic_fixed_string RoutePath, typename Handler>
  auto GET(Handler&& handler) const
  {
    return handle<RoutePath>(http::verb::get, std::forward<Handler>(handler));
  }

  template <basic_fixed_string RoutePath, typename Handler>
  auto POST(Handler&& handler) const
  {
    return handle<RoutePath>(http::verb::post, std::forward<Handler>(handler));
  }

  template <basic_fixed_string RoutePath, typename Handler>
  auto PUT(Handler&& handler) const
  {
    return handle<RoutePath>(http::verb::put, std::forward<Handler>(handler));
  }

  template <basic_fixed_string RoutePath, typename Handler>
  auto PATCH(Handler&& handler) const
  {
    return handle<RoutePath>(http::verb::patch, std::forward<Handler>(handler));
  }

  template <basic_fixed_string RoutePath, typename Handler>
  auto DELETE_(Handler&& handler) const
  {
    return handle<RoutePath>(http::verb::delete_,
                             std::forward<Handler>(handler));
  }

  template <basic_fixed_string RoutePath, typename Handler>
  auto HEAD(Handler&& handler) const
  {
    return handle<RoutePath>(http::verb::head, std::forward<Handler>(handler));
  }

  template <basic_fixed_string RoutePath, typename Handler>
  auto OPTIONS(Handler&& handler) const
  {
    return handle<RoutePath>(http::verb::options,
                             std::forward<Handler>(handler));
  }

  template <basic_fixed_string ChildPath,
            typename... ChildServices,
            typename... ChildRoutes>
  auto sub_scope(scope_impl<ChildPath,
                            std::tuple<ChildServices...>,
                            std::tuple<ChildRoutes...>> child) const
  {
    auto routes = std::apply(
        [this](auto&&... routes_) {
          return std::tuple { routes_.template rebind_parent<Path>(
              state_map_, services_)... };
        },
        child.routes());

    return scope_impl<Path,
                      std::tuple<Services...>,
                      decltype(std::tuple_cat(routes_, std::move(routes)))> {
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

template <basic_fixed_string Path>
inline auto scope()
{
  static_assert(compile_time_path_checker::is_valid<Path>(),
                "invalid path for scope");
  return scope_impl<Path, std::tuple<>, std::tuple<>>({}, {}, {});
}
}

FITORIA_NAMESPACE_END

#endif
