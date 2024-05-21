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

#include <fitoria/web/path_parser.hpp>
#include <fitoria/web/route.hpp>

FITORIA_NAMESPACE_BEGIN

namespace web {

template <basic_fixed_string Path, typename Middlewares, typename Routes>
class scope_impl;

template <basic_fixed_string Path, typename... Middlewares, typename... Routes>
class scope_impl<Path, std::tuple<Middlewares...>, std::tuple<Routes...>> {
  shared_state_map state_map_;
  std::tuple<Middlewares...> middlewares_;
  std::tuple<Routes...> routes_;

public:
  scope_impl(std::tuple<Middlewares...> middlewares,
             std::tuple<Routes...> routes)
      : state_map_(std::make_shared<state_map>())
      , middlewares_(std::move(middlewares))
      , routes_(std::move(routes))
  {
  }

  scope_impl(shared_state_map state_map,
             std::tuple<Middlewares...> middlewares,
             std::tuple<Routes...> routes)
      : state_map_(std::move(state_map))
      , middlewares_(std::move(middlewares))
      , routes_(std::move(routes))
  {
  }

  template <typename State>
  auto use_state(State&& state) const
  {
    using state_type = std::decay_t<State>;
    static_assert(std::copy_constructible<state_type>);

    auto state_map = state_map_;
    (*state_map)[std::type_index(typeid(state_type))]
        = std::any(std::forward<State>(state));

    return scope_impl<Path, std::tuple<Middlewares...>, std::tuple<Routes...>>(
        std::move(state_map), middlewares_, routes_);
  }

  template <typename Middleware>
  auto use(Middleware&& mw) const
  {
    return scope_impl<Path,
                      std::tuple<Middlewares..., std::decay_t<Middleware>>,
                      std::tuple<Routes...>>(
        state_map_,
        std::tuple_cat(middlewares_,
                       std::tuple { std::forward<Middleware>(mw) }),
        routes_);
  }

  template <basic_fixed_string RoutePath,
            typename... RouteServices,
            typename Handler>
  auto serve(
      route_impl<RoutePath, std::tuple<RouteServices...>, Handler> route) const
  {
    auto new_route
        = route.template rebind_parent<Path>(state_map_, middlewares_);

    return scope_impl<Path,
                      std::tuple<Middlewares...>,
                      std::tuple<Routes..., decltype(new_route)>> {
      state_map_,
      middlewares_,
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
              state_map_, middlewares_)... };
        },
        child.routes());

    return scope_impl<Path,
                      std::tuple<Middlewares...>,
                      decltype(std::tuple_cat(routes_, std::move(routes)))> {
      state_map_,
      middlewares_,
      std::tuple_cat(routes_, std::move(routes)),
    };
  }

  auto routes() const
  {
    return routes_;
  }
};

/// @verbatim embed:rst:leading-slashes
///
/// Provides the way to create scopes.
///
/// Description
///     A path and must be specified for construction. And users can optionally
///     bind it with ``state`` s and ``middleware`` s. Note that ``state`` s
///     bound for a ``scope`` will be shared between all ``route`` s under the
///     same ``scope``, whereas ``middleware`` s will be distinct instances for
///     each ``route``.
///
/// @endverbatim
template <basic_fixed_string Path = "">
inline auto scope()
{
  static_assert(path_parser<false>().parse<Path>(), "invalid path for scope");
  return scope_impl<Path, std::tuple<>, std::tuple<>>({}, {});
}
}

FITORIA_NAMESPACE_END

#endif
