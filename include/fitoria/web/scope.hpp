//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
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
  auto serve(
      route_impl<RoutePath, std::tuple<RouteServices...>, Handler> route) const
  {
    auto new_route = route.template rebind_parent<Path>(state_map_, services_);

    return scope_impl<Path,
                      std::tuple<Services...>,
                      std::tuple<Routes..., decltype(new_route)>> {
      state_map_,
      services_,
      std::tuple_cat(routes_, std::tuple { std::move(new_route) }),
    };
  }

  template <basic_fixed_string ChildPath,
            typename... ChildServices,
            typename... ChildRoutes>
  auto serve(scope_impl<ChildPath,
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

template <basic_fixed_string Path = "">
inline auto scope()
{
  static_assert(compile_time_path_checker::is_valid_scope<Path>(),
                "invalid path for scope");
  return scope_impl<Path, std::tuple<>, std::tuple<>>({}, {}, {});
}
}

FITORIA_NAMESPACE_END

#endif
