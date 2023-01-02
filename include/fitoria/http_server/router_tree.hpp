//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include <fitoria/core/config.hpp>

#include <fitoria/core/handler_concept.hpp>
#include <fitoria/core/http.hpp>
#include <fitoria/core/optional.hpp>
#include <fitoria/core/unordered_string_map.hpp>

#include <fitoria/http_server/error.hpp>
#include <fitoria/http_server/http_handler_trait.hpp>
#include <fitoria/http_server/route.hpp>
#include <fitoria/http_server/router.hpp>

#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

FITORIA_NAMESPACE_BEGIN

template <typename HandlerTrait>
class basic_router_tree {
public:
  using router_type = basic_router<HandlerTrait>;

private:
  class node {
    friend class basic_router_tree;

  private:
    auto try_insert(const router_type& r,
                    const route::segments& segments,
                    std::size_t segment_index) -> expected<void, error_code>
    {
      if (segment_index == segments.size()) {
        if (router_) {
          return unexpected { make_error_code(error::route_already_exists) };
        }

        router_.emplace(r);
        return {};
      }

      const auto& segment = segments[segment_index];
      if (segment.is_param) {
        if (!param_trees_) {
          param_trees_.emplace(std::make_shared<node>());
        }
        return param_trees_.value()->try_insert(r, segments, segment_index + 1);
      }

      return path_trees_[std::string(segment.escaped)].try_insert(
          r, segments, segment_index + 1);
    }

    auto try_find(const route::segments& segments,
                  std::size_t segment_index) const noexcept
        -> expected<const router_type&, error_code>
    {
      if (segment_index == segments.size()) {
        return router_.value();
      }

      return try_find_path_trees(segments[segment_index].original)
          .to_expected_or(make_error_code(error::route_not_exists))
          .and_then([&](auto&& node) {
            return node.try_find(segments, segment_index + 1);
          })
          .or_else([&](auto&& error) {
            return param_trees_.to_expected_or(error).and_then(
                [&](auto&& node_ptr)
                    -> expected<const router_type&, error_code> {
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

    static auto try_parse_path(std::string_view path)
        -> expected<route::segments, error_code>
    {
      if (path.empty()) {
        return unexpected { make_error_code(error::route_parse_error) };
      }

      return route::to_segments(path);
    }

    optional<router_type> router_;
    unordered_string_map<node> path_trees_;
    optional<std::shared_ptr<node>> param_trees_;
  };

public:
  auto try_insert(const router_type& r) -> expected<void, error_code>
  {
    return node::try_parse_path(r.path()).and_then([&](auto&& segments) {
      return subtrees_[r.method()].try_insert(r, segments, 0);
    });
  }

  auto try_find(http::verb method, std::string_view path) const
      -> expected<const router_type&, error_code>
  {
    return node::try_parse_path(path).and_then([&](auto&& segments) {
      return try_find(method)
          .to_expected_or(make_error_code(error::route_not_exists))
          .and_then([&](auto&& node) { return node.try_find(segments, 0); });
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

using router_tree = basic_router_tree<http_handler_trait>;

FITORIA_NAMESPACE_END
