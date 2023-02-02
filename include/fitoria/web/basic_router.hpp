//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef FITORIA_WEB_BASIC_ROUTER_HPP
#define FITORIA_WEB_BASIC_ROUTER_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/core/expected_ext.hpp>
#include <fitoria/core/optional_ext.hpp>
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
                    const segments_view& segs,
                    std::size_t seg_idx) -> expected<void, error_code>
    {
      if (seg_idx == segs.size()) {
        if (route_) {
          return unexpected { make_error_code(error::route_already_exists) };
        }

        route_.emplace(route);
        return {};
      }

      auto& segment = segs[seg_idx];
      if (segment.is_param) {
        if (!param_tree_) {
          param_tree_.emplace(std::make_shared<node>());
        }
        return param_tree_.value()->try_insert(route, segs, seg_idx + 1);
      }

      return subtrees_[std::string(segment.escaped)].try_insert(route, segs,
                                                                seg_idx + 1);
    }

    auto try_find(const segments_view& segs, std::size_t seg_idx) const noexcept
        -> expected<const route_type&, error_code>
    {
      if (seg_idx == segs.size()) {
        return to_expected(optional<const route_type&>(route_),
                           make_error_code(error::route_not_exists));
      }

      return to_expected(try_find_path_trees(segs[seg_idx].original),
                         make_error_code(error::route_not_exists))
          .and_then(
              [&](auto&& node) { return node.try_find(segs, seg_idx + 1); })
          .or_else([&](auto&& error) {
            return to_expected(param_tree_, error)
                .and_then([&](auto&& node_ptr) {
                  return node_ptr->try_find(segs, seg_idx + 1);
                });
          });
    }

    auto try_find_path_trees(std::string_view token) const noexcept
        -> optional<const node&>
    {
      if (auto iter = subtrees_.find(token); iter != subtrees_.end()) {
        return iter->second;
      }

      return nullopt;
    }

    optional<route_type> route_;
    unordered_string_map<node> subtrees_;
    optional<std::shared_ptr<node>> param_tree_;
  };

public:
  auto try_insert(const route_type& route) -> expected<void, error_code>
  {
    return segments_view::from_path(route.path()).and_then([&](auto&& segs) {
      return subtrees_[route.method()].try_insert(route, segs, 0);
    });
  }

  auto try_find(http::verb method, std::string_view path) const
      -> expected<const route_type&, error_code>
  {
    return segments_view::from_path(path)
        .transform_error(
            [](auto&&) { return make_error_code(error::route_not_exists); })
        .and_then([&](auto&& segs) {
          return to_expected(try_find(method),
                             make_error_code(error::route_not_exists))
              .and_then([&](auto&& node) { return node.try_find(segs, 0); });
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
