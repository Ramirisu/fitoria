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
#include <fitoria/web/path_matcher.hpp>

#include <memory>
#include <unordered_map>
#include <vector>

FITORIA_NAMESPACE_BEGIN

namespace web {

template <typename Request, typename Response>
class router {
public:
  using route_type = any_routable<Request, Response>;

  class node {
    std::string prefix_;
    std::unordered_map<http::verb, route_type> routes_;
    std::vector<node> statics_;
    std::shared_ptr<node> params_;
    std::shared_ptr<node> wildcard_;

    auto try_insert(const route_type& route,
                    path_matcher::tokens_t::size_type token_index)
        -> expected<void, error_code>
    {
      if (token_index == route.matcher().tokens().size()) {
        return try_insert_route(route);
      }

      auto& token = route.matcher().tokens()[token_index];
      if (token.kind == path_matcher::token_kind::static_) {
        return try_insert_static(route, token.value, token_index);
      }
      if (token.kind == path_matcher::token_kind::named_param) {
        return try_insert_param(route, token_index);
      }

      return try_insert_wildcard(route, token_index);
    }

    auto try_insert_static(const route_type& route,
                           std::string_view token,
                           path_matcher::tokens_t::size_type token_index)
        -> expected<void, error_code>
    {
      if (token.empty()) {
        return try_insert(route, token_index + 1);
      }

      for (auto& node : statics_) {
        if (token.starts_with(node.prefix_)) {
          return node.try_insert_static(
              route, token.substr(node.prefix_.size()), token_index);
        }
      }

      for (std::size_t i = 0; i < statics_.size(); ++i) {
        if (statics_[i].prefix_.starts_with(token)) {
          auto n = node(token);
          statics_[i].prefix_.erase(0, token.size());
          n.statics_.push_back(std::move(statics_[i]));
          std::swap(n, statics_[i]);
          return statics_[i].try_insert(route, token_index + 1);
        }
      }

      return statics_.emplace_back(token).try_insert(route, token_index + 1);
    }

    auto try_insert_param(const route_type& route,
                          path_matcher::tokens_t::size_type token_index)
        -> expected<void, error_code>
    {
      if (!params_) {
        params_ = std::make_shared<node>();
      }

      return params_->try_insert(route, token_index + 1);
    }

    auto try_insert_wildcard(const route_type& route,
                             path_matcher::tokens_t::size_type token_index)
        -> expected<void, error_code>
    {
      if (!wildcard_) {
        wildcard_ = std::make_shared<node>();
      }

      return wildcard_->try_insert(route, token_index + 1);
    }

    auto try_insert_route(const route_type& route) -> expected<void, error_code>
    {
      if (auto [_, ok] = routes_.insert({ route.method(), route }); ok) {
        return {};
      }

      return unexpected { make_error_code(error::route_already_exists) };
    }

    auto try_find_static(http::verb method, std::string_view path) const
        -> expected<const route_type&, error_code>
    {
      for (auto& node : statics_) {
        if (path.starts_with(node.prefix_)) {
          return node.try_find(method, path.substr(node.prefix_.size()));
        }
      }

      return unexpected { make_error_code(error::route_not_exists) };
    }

    auto try_find_param(http::verb method, std::string_view path) const
        -> expected<const route_type&, error_code>
    {
      if (params_) {
        if (auto pos = path.find('/'); pos != std::string_view::npos) {
          return params_->try_find(method, path.substr(pos));
        }
        return params_->try_find(method, std::string_view());
      }

      return unexpected { make_error_code(error::route_not_exists) };
    }

    auto try_find_wildcard(http::verb method) const
        -> expected<const route_type&, error_code>
    {
      if (wildcard_) {
        return wildcard_->try_find(method, std::string_view());
      }

      return unexpected { make_error_code(error::route_not_exists) };
    }

  public:
    node() = default;

    node(std::string_view prefix)
        : prefix_(std::string(prefix))
    {
    }

    auto try_insert(const route_type& route) -> expected<void, error_code>
    {
      return try_insert(route, 0);
    }

    auto try_find(http::verb method, std::string_view path) const
        -> expected<const route_type&, error_code>
    {
      if (path.empty()) {
        if (auto it = routes_.find(method); it != routes_.end()) {
          return it->second;
        }
        if (auto it = routes_.find(http::verb::unknown); it != routes_.end()) {
          return it->second;
        }

        return unexpected { make_error_code(error::route_not_exists) };
      }

      return try_find_static(method, path)
          .or_else([&](auto&&) { return try_find_param(method, path); })
          .or_else([&](auto&&) { return try_find_wildcard(method); });
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
