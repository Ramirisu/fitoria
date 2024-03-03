//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#ifndef FITORIA_WEB_ROUTER_HPP
#define FITORIA_WEB_ROUTER_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/core/unordered_string_map.hpp>

#include <fitoria/web/any_routable.hpp>
#include <fitoria/web/error.hpp>
#include <fitoria/web/http.hpp>
#include <fitoria/web/pattern_matcher.hpp>

#include <memory>
#include <unordered_map>

FITORIA_NAMESPACE_BEGIN

namespace web {

template <typename Request, typename Response>
class router {
public:
  using route_type = any_routable<Request, Response>;

  class node {
    std::unordered_map<http::verb, route_type> routes_;
    unordered_string_map<node> static_subnodes_;
    std::shared_ptr<node> param_subnode_;

    auto try_insert(const route_type& route, segments_t::size_type seg_index)
        -> expected<void, error_code>
    {
      if (seg_index == route.segments().size()) {
        if (auto [_, ok] = routes_.insert({ route.method(), route }); ok) {
          return {};
        }
        return unexpected { make_error_code(error::route_already_exists) };
      }

      auto& seg = route.segments()[seg_index];
      if (seg.kind == segment_kind::parameterized) {
        if (!param_subnode_) {
          param_subnode_ = std::make_unique<node>();
        }
        return param_subnode_->try_insert(route, seg_index + 1);
      }

      return static_subnodes_[seg.value].try_insert(route, seg_index + 1);
    }

    auto try_find(http::verb method,
                  const segments_t& segs,
                  segments_t::size_type seg_index) const
        -> expected<const route_type&, error_code>
    {
      if (seg_index == segs.size()) {
        if (auto it = routes_.find(method); it != routes_.end()) {
          return it->second;
        }
        if (auto it = routes_.find(http::verb::unknown); it != routes_.end()) {
          return it->second;
        }
        return unexpected { make_error_code(error::route_not_exists) };
      }

      if (auto it = static_subnodes_.find(segs[seg_index].value);
          it != static_subnodes_.end()) {
        if (auto result = it->second.try_find(method, segs, seg_index + 1);
            result) {
          return result;
        }
      }

      if (param_subnode_) {
        return param_subnode_->try_find(method, segs, seg_index + 1);
      }

      return unexpected { make_error_code(error::route_not_exists) };
    }

  public:
    auto try_insert(const route_type& route) -> expected<void, error_code>
    {
      return try_insert(route, 0);
    }

    auto try_find(http::verb method, std::string_view path) const
        -> expected<const route_type&, error_code>
    {
      return parse_pattern(path).and_then(
          [&](auto&& segs) -> expected<const route_type&, error_code> {
            return try_find(method, segs, 0);
          });
    }
  };

  node root_;

public:
  auto try_insert(const route_type& route) -> expected<void, error_code>
  {
    return root_.try_insert(route);
  }

  auto try_find(http::verb method, std::string_view path) const
      -> expected<const route_type&, error_code>
  {
    return root_.try_find(method, path);
  }
};
}

FITORIA_NAMESPACE_END

#endif
