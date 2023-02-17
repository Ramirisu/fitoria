//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef FITORIA_WEB_ROUTE_HPP
#define FITORIA_WEB_ROUTE_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/core/fixed_string.hpp>

#include <fitoria/web/compile_time_path_checker.hpp>
#include <fitoria/web/http/http.hpp>
#include <fitoria/web/pattern_matcher.hpp>
#include <fitoria/web/routable.hpp>
#include <fitoria/web/service.hpp>
#include <fitoria/web/state_map.hpp>

#include <string>
#include <tuple>

FITORIA_NAMESPACE_BEGIN

namespace web {

template <basic_fixed_string Path, typename Services, typename Handler>
class route_builder;

template <basic_fixed_string Path, typename... Services, typename Handler>
class route_builder<Path, std::tuple<Services...>, Handler> {
  http::verb method_;
  std::vector<state_map> state_maps_;
  std::tuple<Services...> services_;
  Handler handler_;

public:
  static_assert(compile_time_path_checker::is_valid<Path>(),
                "invalid path for route");

  route_builder(http::verb method,
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
    return route_builder<Path, std::tuple<Services...>, Handler>(
        method_, std::move(state_maps), services_, handler_);
  }

  template <typename Service>
  auto use(Service&& service) const
  {
    return route_builder<Path, std::tuple<Services..., std::decay_t<Service>>,
                         Handler>(
        method_, state_maps_,
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
    return route_builder<ParentPath + Path,
                         std::tuple<ParentServices..., Services...>, Handler>(
        method_, std::move(state_maps),
        std::tuple_cat(std::move(parent_services), services_), handler_);
  }

  template <typename HandlerServiceFactory>
  auto build(HandlerServiceFactory handler_service_factory) const
  {
    return routable(
        method_, pattern_matcher<Path>(), state_maps_,
        build_service(std::tuple_cat(
            services_, std::tuple { handler_service_factory, handler_ })));
  }

  auto build() const
  {
    return routable(
        method_, pattern_matcher<Path>(), state_maps_,
        build_service(std::tuple_cat(services_, std::tuple { handler_ })));
  }

private:
  template <typename... S>
  static auto build_service(std::tuple<S...> s)
  {
    return build_service(s, std::make_index_sequence<sizeof...(S)> {});
  }

  template <typename... S, std::size_t... Is>
  static auto build_service(std::tuple<S...> s, std::index_sequence<Is...>)
  {
    return build_service_impl(std::get<sizeof...(S) - Is - 1>(std::move(s))...);
  }

  template <typename S0>
  static auto build_service_impl(S0&& s0)
  {
    return std::forward<S0>(s0);
  }

  template <typename S0, typename S1, typename... S>
  static auto build_service_impl(S0&& s0, S1&& s1, S&&... s)
  {
    auto service = make_service(std::forward<S1>(s1), std::forward<S0>(s0));

    if constexpr (sizeof...(S) > 0) {
      return build_service_impl(std::move(service), std::forward<S>(s)...);
    } else {
      return service;
    }
  }
};

class route {
public:
  template <basic_fixed_string Path, typename Handler>
  static auto handle(http::verb method, Handler&& handler)
  {
    static_assert(compile_time_path_checker::is_valid<Path>(),
                  "invalid path for route");
    return route_builder<Path, std::tuple<>, std::decay_t<Handler>>(
        method, {}, {}, std::forward<Handler>(handler));
  }

  template <basic_fixed_string Path, typename Handler>
  static auto GET(Handler&& handler)
  {

    static_assert(compile_time_path_checker::is_valid<Path>(),
                  "invalid path for route");
    return handle<Path>(http::verb::get, std::forward<Handler>(handler));
  }

  template <basic_fixed_string Path, typename Handler>
  static auto POST(Handler&& handler)
  {

    static_assert(compile_time_path_checker::is_valid<Path>(),
                  "invalid path for route");
    return handle<Path>(http::verb::post, std::forward<Handler>(handler));
  }

  template <basic_fixed_string Path, typename Handler>
  static auto PUT(Handler&& handler)
  {

    static_assert(compile_time_path_checker::is_valid<Path>(),
                  "invalid path for route");
    return handle<Path>(http::verb::put, std::forward<Handler>(handler));
  }

  template <basic_fixed_string Path, typename Handler>
  static auto PATCH(Handler&& handler)
  {

    static_assert(compile_time_path_checker::is_valid<Path>(),
                  "invalid path for route");
    return handle<Path>(http::verb::patch, std::forward<Handler>(handler));
  }

  template <basic_fixed_string Path, typename Handler>
  static auto DELETE_(Handler&& handler)
  {

    static_assert(compile_time_path_checker::is_valid<Path>(),
                  "invalid path for route");
    return handle<Path>(http::verb::delete_, std::forward<Handler>(handler));
  }

  template <basic_fixed_string Path, typename Handler>
  static auto HEAD(Handler&& handler)
  {

    static_assert(compile_time_path_checker::is_valid<Path>(),
                  "invalid path for route");
    return handle<Path>(http::verb::head, std::forward<Handler>(handler));
  }

  template <basic_fixed_string Path, typename Handler>
  static auto OPTIONS(Handler&& handler)
  {

    static_assert(compile_time_path_checker::is_valid<Path>(),
                  "invalid path for route");
    return handle<Path>(http::verb::options, std::forward<Handler>(handler));
  }
};
}

FITORIA_NAMESPACE_END

#endif
