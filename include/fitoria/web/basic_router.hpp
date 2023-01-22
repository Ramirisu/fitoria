//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef FITORIA_WEB_BASIC_ROUTER_HPP
#define FITORIA_WEB_BASIC_ROUTER_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/core/optional.hpp>
#include <fitoria/core/unordered_string_map.hpp>

#include <fitoria/web/basic_route.hpp>
#include <fitoria/web/error.hpp>
#include <fitoria/web/http.hpp>
#include <fitoria/web/segments_view.hpp>

#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

FITORIA_NAMESPACE_BEGIN

template <typename Route>
class basic_router {
public:
  using route_type = Route;

private:
  class node {
    friend class basic_router;

  private:
    auto try_insert(const route_type& route,
                    const segments_view& segments,
                    std::size_t segment_index) -> expected<void, error_code>
    {
      if (segment_index == segments.size()) {
        if (route_) {
          return unexpected { make_error_code(error::route_already_exists) };
        }

        route_.emplace(route);
        return {};
      }

      const auto& segment = segments[segment_index];
      if (segment.is_param) {
        if (!param_trees_) {
          param_trees_.emplace(std::make_shared<node>());
        }
        return param_trees_.value()->try_insert(route, segments,
                                                segment_index + 1);
      }

      return path_trees_[std::string(segment.escaped)].try_insert(
          route, segments, segment_index + 1);
    }

    auto try_find(const segments_view& segments,
                  std::size_t segment_index) const noexcept
        -> expected<const route_type&, error_code>
    {
      if (segment_index == segments.size()) {
        return optional<const route_type&>(route_).to_expected_or(
            make_error_code(error::route_not_exists));
      }

      return try_find_path_trees(segments[segment_index].original)
          .to_expected_or(make_error_code(error::route_not_exists))
          .and_then([&](auto&& node) {
            return node.try_find(segments, segment_index + 1);
          })
          .or_else([&](auto&& error) {
            return param_trees_.to_expected_or(error).and_then(
                [&](auto&& node_ptr)
                    -> expected<const route_type&, error_code> {
                  return node_ptr->try_find(segments, segment_index + 1);
                });
          });
    }

    auto try_find_path_trees(std::string_view token) const noexcept
        -> optional<const node&>
    {
      if (auto iter = path_trees_.find(token); iter != path_trees_.end()) {
        return iter->second;
      }

      return nullopt;
    }

    optional<route_type> route_;
    unordered_string_map<node> path_trees_;
    optional<std::shared_ptr<node>> param_trees_;
  };

public:
  auto try_insert(const route_type& route) -> expected<void, error_code>
  {
    return segments_view::from_path(route.path())
        .and_then([&](auto&& segments) {
          return subtrees_[route.method()].try_insert(route, segments, 0);
        });
  }

  auto try_find(http::verb method, std::string_view path) const
      -> expected<const route_type&, error_code>
  {
    return segments_view::from_path(path)
        .transform_error(
            [](auto&&) { return make_error_code(error::route_not_exists); })
        .and_then([&](auto&& segments) {
          return try_find(method)
              .to_expected_or(make_error_code(error::route_not_exists))
              .and_then(
                  [&](auto&& node) { return node.try_find(segments, 0); });
        });
  }

private:
  auto try_find(http::verb method) const noexcept -> optional<const node&>
  {
    if (auto iter = subtrees_.find(method); iter != subtrees_.end()) {
      return iter->second;
    }

    return nullopt;
  }

  std::unordered_map<http::verb, node> subtrees_;
};

FITORIA_NAMESPACE_END

#endif
