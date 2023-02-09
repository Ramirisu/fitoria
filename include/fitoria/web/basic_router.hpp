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

#include <fitoria/web/any_routable.hpp>
#include <fitoria/web/error.hpp>
#include <fitoria/web/http.hpp>
#include <fitoria/web/pattern_matcher.hpp>

#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

FITORIA_NAMESPACE_BEGIN

namespace web {

template <typename Route>
class basic_router {
public:
  using route_type = Route;

private:
  class node {
    friend class basic_router;

  private:
    auto try_insert(const route_type& route, std::size_t seg_idx)
        -> expected<void, error_code>
    {
      if (seg_idx == route.matcher().segments().size()) {
        if (route_) {
          return unexpected { make_error_code(error::route_already_exists) };
        }

        route_.emplace(route);
        return {};
      }

      auto& segment = route.matcher().segments()[seg_idx];
      if (segment.kind == pattern_matcher::segment_kind::parameterized) {
        if (!param_tree_) {
          param_tree_.emplace(std::make_shared<node>());
        }
        return param_tree_.value()->try_insert(route, seg_idx + 1);
      }

      return subtrees_[segment.value].try_insert(route, seg_idx + 1);
    }

    auto try_find(const pattern_matcher::segments_t& segs,
                  std::size_t seg_idx) const noexcept
        -> expected<const route_type&, error_code>
    {
      if (seg_idx == segs.size()) {
        return to_expected(optional<const route_type&>(route_),
                           make_error_code(error::route_not_exists));
      }

      return to_expected(try_find_subtrees(segs[seg_idx].value),
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

    auto try_find_subtrees(std::string_view segment) const noexcept
        -> optional<const node&>
    {
      if (auto iter = subtrees_.find(segment); iter != subtrees_.end()) {
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
    return subtrees_[route.method()].try_insert(route, 0);
  }

  auto try_find(http::verb method, std::string path) const
      -> expected<const route_type&, error_code>
  {
    return to_expected(pattern_matcher::from_pattern(path),
                       make_error_code(error::route_not_exists))
        .and_then([&](auto&& pattern) {
          return to_expected(try_find(method),
                             make_error_code(error::route_not_exists))
              .and_then([&](auto&& node) {
                return node.try_find(pattern.segments(), 0);
              });
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

}

FITORIA_NAMESPACE_END

#endif
