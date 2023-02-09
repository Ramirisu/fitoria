//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef FITORIA_WEB_ROUTE_HPP
#define FITORIA_WEB_ROUTE_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/web/http/http.hpp>
#include <fitoria/web/pattern_matcher.hpp>
#include <fitoria/web/service.hpp>
#include <fitoria/web/state_map.hpp>

#include <string>
#include <tuple>

FITORIA_NAMESPACE_BEGIN

namespace web {

template <typename Services, typename Handler>
class route_builder;

template <typename... Services, typename Handler>
class route_builder<std::tuple<Services...>, Handler> {
  http::verb method_;
  pattern_matcher matcher_;
  std::vector<state_map> state_maps_;
  std::tuple<Services...> services_;
  Handler handler_;

public:
  route_builder(http::verb method,
                std::string pattern,
                std::vector<state_map> state_maps,
                std::tuple<Services...> services,
                Handler handler)
      : method_(method)
      , matcher_(pattern_matcher::from_pattern(std::move(pattern)).value())
      , state_maps_(std::move(state_maps))
      , services_(std::move(services))
      , handler_(std::move(handler))
  {
  }

  route_builder(http::verb method,
                pattern_matcher matcher,
                std::vector<state_map> state_maps,
                std::tuple<Services...> services,
                Handler handler)
      : method_(method)
      , matcher_(std::move(matcher))
      , state_maps_(std::move(state_maps))
      , services_(std::move(services))
      , handler_(std::move(handler))
  {
  }

  template <typename State>
  auto state(State state)
  {
    auto state_maps = state_maps_;
    if (state_maps.empty()) {
      state_maps.push_back({});
    }
    state_maps.front()[std::type_index(typeid(State))]
        = std::any(std::move(state));
    return route_builder<std::tuple<Services...>, Handler>(
        method_, matcher_, std::move(state_maps), services_, handler_);
  }

  template <typename Service>
  auto use(Service service) const
  {
    return route_builder<std::tuple<Services..., Service>, Handler>(
        method_, matcher_, state_maps_,
        std::tuple_cat(services_, std::tuple { std::move(service) }), handler_);
  }

  template <typename... ParentServices>
  auto rebind_parent(std::string parent_path,
                     state_map parent_state_map,
                     std::tuple<ParentServices...> parent_services) const
  {
    parent_path += matcher_.pattern();
    auto state_maps = state_maps_;
    if (!parent_state_map.empty()) {
      state_maps.push_back(std::move(parent_state_map));
    }
    return route_builder<std::tuple<ParentServices..., Services...>, Handler>(
        method_, std::move(parent_path), std::move(state_maps),
        std::tuple_cat(std::move(parent_services), services_), handler_);
  }

  auto method() const noexcept
  {
    return method_;
  }

  auto matcher() const noexcept -> const pattern_matcher&
  {
    return matcher_;
  }

  auto build() const
  {
    return std::tuple { method_, matcher_, state_maps_,
                        build_service(std::tuple_cat(
                            services_, std::tuple { handler_ })) };
  }

private:
  template <typename... S>
  static auto build_service(const std::tuple<S...>& s)
  {
    return build_service(s, std::make_index_sequence<sizeof...(S)> {});
  }

  template <typename... S, std::size_t... Is>
  static auto build_service(const std::tuple<S...>& s,
                            std::index_sequence<Is...>)
  {
    return build_service_impl(std::get<sizeof...(S) - Is - 1>(s)...);
  }

  template <typename S0>
  static auto build_service_impl(S0&& s0)
  {
    return s0;
  }

  template <typename S0, typename S1, typename... S>
  static auto build_service_impl(S0&& s0, S1&& s1, S&&... s)
  {
    if constexpr (sizeof...(S) > 0) {
      return build_service_impl(
          tag_invoke(make_service, std::forward<S1>(s1), std::forward<S0>(s0)),
          std::forward<S>(s)...);
    } else {
      return tag_invoke(make_service, std::forward<S1>(s1),
                        std::forward<S0>(s0));
    }
  }
};

class route {
public:
  template <typename Handler>
  static auto handle(http::verb method, std::string path, Handler handler)
  {
    return route_builder<std::tuple<>, Handler>(method, std::move(path), {}, {},
                                                std::move(handler));
  }

  template <typename Handler>
  static auto GET(std::string path, Handler handler)
  {
    return handle(http::verb::get, std::move(path), std::move(handler));
  }

  template <typename Handler>
  static auto POST(std::string path, Handler handler)
  {
    return handle(http::verb::post, std::move(path), std::move(handler));
  }

  template <typename Handler>
  static auto PUT(std::string path, Handler handler)
  {
    return handle(http::verb::put, std::move(path), std::move(handler));
  }

  template <typename Handler>
  static auto PATCH(std::string path, Handler handler)
  {
    return handle(http::verb::patch, std::move(path), std::move(handler));
  }

  template <typename Handler>
  static auto DELETE_(std::string path, Handler handler)
  {
    return handle(http::verb::delete_, std::move(path), std::move(handler));
  }

  template <typename Handler>
  static auto HEAD(std::string path, Handler handler)
  {
    return handle(http::verb::head, std::move(path), std::move(handler));
  }

  template <typename Handler>
  static auto OPTIONS(std::string path, Handler handler)
  {
    return handle(http::verb::options, std::move(path), std::move(handler));
  }
};
}

FITORIA_NAMESPACE_END

#endif
