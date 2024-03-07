//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#ifndef FITORIA_WEB_ROUTE_HPP
#define FITORIA_WEB_ROUTE_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/core/fixed_string.hpp>
#include <fitoria/core/utility.hpp>

#include <fitoria/web/compile_time_path_checker.hpp>
#include <fitoria/web/http/http.hpp>
#include <fitoria/web/middleware_concept.hpp>
#include <fitoria/web/path_matcher.hpp>
#include <fitoria/web/routable.hpp>
#include <fitoria/web/state_map.hpp>

#include <tuple>

FITORIA_NAMESPACE_BEGIN

namespace web {

template <basic_fixed_string Path, typename Services, typename Handler>
class route_impl;

template <basic_fixed_string Path, typename... Services, typename Handler>
class route_impl<Path, std::tuple<Services...>, Handler> {
  http::verb method_;
  std::vector<state_map> state_maps_;
  std::tuple<Services...> services_;
  Handler handler_;

public:
  static_assert(compile_time_path_checker::is_valid_route<Path>(),
                "invalid path for route");

  route_impl(http::verb method,
             std::vector<state_map> state_maps,
             std::tuple<Services...> services,
             Handler handler)
      : method_(method)
      , state_maps_(std::move(state_maps))
      , services_(std::move(services))
      , handler_(std::move(handler))
  {
  }

  template <typename State>
  auto state(State&& state)
  {
    auto state_maps = state_maps_;
    if (state_maps.empty()) {
      state_maps.push_back({});
    }
    state_maps.front()[std::type_index(typeid(State))]
        = std::any(std::forward<State>(state));
    return route_impl<Path, std::tuple<Services...>, Handler>(
        method_, std::move(state_maps), services_, handler_);
  }

  template <typename Service>
  auto use(Service&& service) const
  {
    return route_impl<Path,
                      std::tuple<Services..., std::decay_t<Service>>,
                      Handler>(
        method_,
        state_maps_,
        std::tuple_cat(services_,
                       std::tuple { std::forward<Service>(service) }),
        handler_);
  }

  template <basic_fixed_string ParentPath, typename... ParentServices>
  auto rebind_parent(state_map parent_state_map,
                     std::tuple<ParentServices...> parent_services) const
  {
    auto state_maps = state_maps_;
    if (!parent_state_map.empty()) {
      state_maps.push_back(std::move(parent_state_map));
    }
    return route_impl<ParentPath + Path,
                      std::tuple<ParentServices..., Services...>,
                      Handler>(
        method_,
        std::move(state_maps),
        std::tuple_cat(std::move(parent_services), services_),
        handler_);
  }

  template <typename... AdditionalServices>
  auto build(AdditionalServices... additionalServices) const
  {
    return routable(
        method_,
        path_matcher(Path),
        state_maps_,
        std::apply(
            [](auto... ss) { return build_service(std::move(ss)...); },
            reverse_tuple(std::tuple_cat(
                services_,
                std::tuple { std::move(additionalServices)..., handler_ }))));
  }

private:
  template <typename S0>
  static auto build_service(S0 s0)
  {
    return s0;
  }

  template <typename S0, typename S1, typename... S>
  static auto build_service(S0 s0, S1 s1, S... ss)
  {
    if constexpr (sizeof...(S) > 0) {
      return build_service(new_middleware(std::move(s1), std::move(s0)),
                           std::move(ss)...);
    } else {
      return new_middleware(std::move(s1), std::move(s0));
    }
  }
};

class route {
public:
  template <basic_fixed_string Path, typename Handler>
  static auto handle(http::verb method, Handler&& handler)
  {
    static_assert(compile_time_path_checker::is_valid_route<Path>(),
                  "invalid path for route");
    return route_impl<Path, std::tuple<>, std::decay_t<Handler>>(
        method, {}, {}, std::forward<Handler>(handler));
  }

  template <basic_fixed_string Path, typename Handler>
  static auto any(Handler&& handler)
  {
    static_assert(compile_time_path_checker::is_valid_route<Path>(),
                  "invalid path for route");
    return route_impl<Path, std::tuple<>, std::decay_t<Handler>>(
        http::verb::unknown, {}, {}, std::forward<Handler>(handler));
  }

  template <basic_fixed_string Path, typename Handler>
  static auto get(Handler&& handler)
  {
    static_assert(compile_time_path_checker::is_valid_route<Path>(),
                  "invalid path for route");
    return handle<Path>(http::verb::get, std::forward<Handler>(handler));
  }

  template <basic_fixed_string Path, typename Handler>
  static auto post(Handler&& handler)
  {
    static_assert(compile_time_path_checker::is_valid_route<Path>(),
                  "invalid path for route");
    return handle<Path>(http::verb::post, std::forward<Handler>(handler));
  }

  template <basic_fixed_string Path, typename Handler>
  static auto put(Handler&& handler)
  {
    static_assert(compile_time_path_checker::is_valid_route<Path>(),
                  "invalid path for route");
    return handle<Path>(http::verb::put, std::forward<Handler>(handler));
  }

  template <basic_fixed_string Path, typename Handler>
  static auto patch(Handler&& handler)
  {
    static_assert(compile_time_path_checker::is_valid_route<Path>(),
                  "invalid path for route");
    return handle<Path>(http::verb::patch, std::forward<Handler>(handler));
  }

  template <basic_fixed_string Path, typename Handler>
  static auto delete_(Handler&& handler)
  {
    static_assert(compile_time_path_checker::is_valid_route<Path>(),
                  "invalid path for route");
    return handle<Path>(http::verb::delete_, std::forward<Handler>(handler));
  }

  template <basic_fixed_string Path, typename Handler>
  static auto head(Handler&& handler)
  {
    static_assert(compile_time_path_checker::is_valid_route<Path>(),
                  "invalid path for route");
    return handle<Path>(http::verb::head, std::forward<Handler>(handler));
  }

  template <basic_fixed_string Path, typename Handler>
  static auto options(Handler&& handler)
  {
    static_assert(compile_time_path_checker::is_valid_route<Path>(),
                  "invalid path for route");
    return handle<Path>(http::verb::options, std::forward<Handler>(handler));
  }
};
}

FITORIA_NAMESPACE_END

#endif
