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
#include <fitoria/web/match_pattern.hpp>
#include <fitoria/web/state_map.hpp>

#include <string>
#include <tuple>

FITORIA_NAMESPACE_BEGIN

namespace web {

template <typename Services, typename Handler>
struct route_builder;

template <typename... Services, typename Handler>
struct route_builder<std::tuple<Services...>, Handler> {
  http::verb method_;
  match_pattern match_pattern_;
  std::vector<state_map> state_maps_;
  std::tuple<Services...> services_;
  Handler handler_;

  route_builder(http::verb method,
                match_pattern pattern,
                std::vector<state_map> state_maps,
                std::tuple<Services...> services,
                Handler handler)
      : method_(method)
      , match_pattern_(std::move(pattern))
      , state_maps_(std::move(state_maps))
      , services_(std::move(services))
      , handler_(std::move(handler))
  {
  }

public:
  route_builder(http::verb method,
                std::string pattern,
                std::vector<state_map> state_maps,
                std::tuple<Services...> services,
                Handler handler)
      : method_(method)
      , match_pattern_(match_pattern::from_pattern(std::move(pattern)).value())
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
        method_, match_pattern_, std::move(state_maps), services_, handler_);
  }

  template <typename Service>
  auto use(Service service) const
  {
    return route_builder<std::tuple<Services..., Service>, Handler>(
        method_, match_pattern_, state_maps_,
        std::tuple_cat(services_, std::tuple { std::move(service) }), handler_);
  }

  template <typename... ParentServices>
  auto rebind_parent(std::string parent_path,
                     state_map parent_state_map,
                     std::tuple<ParentServices...> parent_services) const
  {
    parent_path += match_pattern_.pattern();
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

  auto pattern() const noexcept -> const match_pattern&
  {
    return match_pattern_;
  }

  auto build() const
    requires(sizeof...(Services) == 0)
  {
    return std::tuple { method_, match_pattern_, state_maps_, handler_ };
  }

  auto build() const
    requires(sizeof...(Services) == 1)
  {
    return std::tuple { method_, match_pattern_, state_maps_,
                        std::get<0>(services_).create(handler_) };
  }

  auto build() const
    requires(sizeof...(Services) == 2)
  {
    return std::tuple { method_, match_pattern_, state_maps_,
                        std::get<0>(services_).create(
                            std::get<1>(services_).create(handler_)) };
  }

  auto build() const
    requires(sizeof...(Services) == 3)
  {
    return std::tuple { method_, match_pattern_, state_maps_,
                        std::get<0>(services_).create(
                            std::get<1>(services_).create(
                                std::get<2>(services_).create(handler_))) };
  }

  auto build() const
    requires(sizeof...(Services) == 4)
  {
    return std::tuple {
      method_, match_pattern_, state_maps_,
      std::get<0>(services_).create(
          std::get<1>(services_).create(std::get<2>(services_).create(
              std::get<3>(services_).create(handler_))))
    };
  }

  auto build() const
    requires(sizeof...(Services) == 5)
  {
    return std::tuple {
      method_, match_pattern_, state_maps_,
      std::get<0>(services_).create(std::get<1>(services_).create(
          std::get<2>(services_).create(std::get<3>(services_).create(
              std::get<4>(services_).create(handler_)))))
    };
  }

  auto build() const
    requires(sizeof...(Services) == 6)
  {
    return std::tuple {
      method_, match_pattern_, state_maps_,
      std::get<0>(services_).create(
          std::get<1>(services_).create(std::get<2>(services_).create(
              std::get<3>(services_).create(std::get<4>(services_).create(
                  std::get<5>(services_).create(handler_))))))
    };
  }

  auto build() const
    requires(sizeof...(Services) == 7)
  {
    return std::tuple {
      method_, match_pattern_, state_maps_,
      std::get<0>(services_).create(std::get<1>(services_).create(
          std::get<2>(services_).create(std::get<3>(services_).create(
              std::get<4>(services_).create(std::get<5>(services_).create(
                  std::get<6>(services_).create(handler_)))))))
    };
  }

  auto build() const
    requires(sizeof...(Services) == 8)
  {
    return std::tuple {
      method_, match_pattern_, state_maps_,
      std::get<0>(services_).create(
          std::get<1>(services_).create(std::get<2>(services_).create(
              std::get<3>(services_).create(std::get<4>(services_).create(
                  std::get<5>(services_).create(std::get<6>(services_).create(
                      std::get<7>(services_).create(handler_))))))))
    };
  }

  auto build() const
    requires(sizeof...(Services) == 9)
  {
    return std::tuple {
      method_, match_pattern_, state_maps_,
      std::get<0>(services_).create(std::get<1>(services_).create(
          std::get<2>(services_).create(std::get<3>(services_).create(
              std::get<4>(services_).create(std::get<5>(services_).create(
                  std::get<6>(services_).create(std::get<7>(services_).create(
                      std::get<8>(services_).create(handler_)))))))))
    };
  }

  auto build() const
    requires(sizeof...(Services) == 10)
  {
    return std::tuple {
      method_, match_pattern_, state_maps_,
      std::get<0>(services_).create(std::get<1>(services_).create(
          std::get<2>(services_).create(std::get<3>(services_).create(
              std::get<4>(services_).create(std::get<5>(services_).create(
                  std::get<6>(services_).create(std::get<7>(services_).create(
                      std::get<8>(services_).create(
                          std::get<9>(services_).create(handler_))))))))))
    };
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
