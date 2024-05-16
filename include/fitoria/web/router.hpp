//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#ifndef FITORIA_WEB_ROUTER_HPP
#define FITORIA_WEB_ROUTER_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/core/unordered_string_map.hpp>

#include <fitoria/http.hpp>

#include <fitoria/web/any_routable.hpp>
#include <fitoria/web/error.hpp>
#include <fitoria/web/path_matcher.hpp>

#include <map>
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
    std::unique_ptr<node> params_;
    std::unordered_map<http::verb, route_type> wildcard_;

    auto try_insert(route_type route, path_tokens_t::size_type token_index)
        -> expected<void, std::error_code>
    {
      if (token_index == route.matcher().tokens().size()) {
        return try_insert_route(std::move(route), routes_);
      }

      auto& token = route.matcher().tokens()[token_index];
      if (token.kind == path_token_kind::static_) {
        return try_insert_static(std::move(route), token.value, token_index);
      }
      if (token.kind == path_token_kind::param) {
        return try_insert_param(std::move(route), token_index);
      }

      FITORIA_ASSERT(token_index == route.matcher().tokens().size() - 1);
      return try_insert_route(std::move(route), wildcard_);
    }

    auto try_insert_static(route_type route,
                           std::string_view token,
                           path_tokens_t::size_type token_index)
        -> expected<void, std::error_code>
    {
      if (token.empty()) {
        return try_insert(std::move(route), token_index + 1);
      }

      for (auto& node : statics_) {
        if (token.starts_with(node.prefix_)) {
          return node.try_insert_static(
              std::move(route), token.substr(node.prefix_.size()), token_index);
        }
      }

      for (std::size_t i = 0; i < statics_.size(); ++i) {
        if (statics_[i].prefix_.starts_with(token)) {
          auto n = node(token);
          statics_[i].prefix_.erase(0, token.size());
          n.statics_.push_back(std::move(statics_[i]));
          std::swap(n, statics_[i]);
          return statics_[i].try_insert(std::move(route), token_index + 1);
        }
      }

      return statics_.emplace_back(token).try_insert(std::move(route),
                                                     token_index + 1);
    }

    auto try_insert_param(route_type route,
                          path_tokens_t::size_type token_index)
        -> expected<void, std::error_code>
    {
      if (!params_) {
        params_ = std::make_unique<node>();
      }

      return params_->try_insert(std::move(route), token_index + 1);
    }

    auto try_insert_route(route_type route,
                          std::unordered_map<http::verb, route_type>& routes)
        -> expected<void, std::error_code>
    {
      if (auto [_, ok] = routes.try_emplace(route.method(), std::move(route));
          ok) {
        return {};
      }

      return unexpected { make_error_code(error::route_already_exists) };
    }

    auto optimize_statics() -> std::size_t
    {
      std::size_t total = 0;

      auto priority = std::multimap<std::size_t, node, std::greater<>>();
      for (auto& node : statics_) {
        auto count = node.optimize();
        total += count;
        priority.insert({ count, std::move(node) });
      }
      statics_.clear();

      for (auto& [_, node] : priority) {
        statics_.push_back(std::move(node));
      }

      return total;
    }

    auto try_find_static(http::verb method, std::string_view path) const
        -> expected<const route_type&, std::error_code>
    {
      for (auto& node : statics_) {
        if (path.starts_with(node.prefix_)) {
          return node.try_find(method, path.substr(node.prefix_.size()));
        }
      }

      return unexpected { make_error_code(error::route_not_exists) };
    }

    auto try_find_param(http::verb method, std::string_view path) const
        -> expected<const route_type&, std::error_code>
    {
      if (params_) {
        if (auto pos = path.find('/'); pos != std::string_view::npos) {
          return params_->try_find(method, path.substr(pos));
        }
        return params_->try_find(method, std::string_view());
      }

      return unexpected { make_error_code(error::route_not_exists) };
    }

    auto
    try_find_route(http::verb method,
                   const std::unordered_map<http::verb, route_type>& routes)
        const -> expected<const route_type&, std::error_code>
    {
      if (auto it = routes.find(method); it != routes.end()) {
        return expected<const route_type&, std::error_code>(it->second);
      }
      if (auto it = routes.find(http::verb::unknown); it != routes.end()) {
        return expected<const route_type&, std::error_code>(it->second);
      }

      return unexpected { make_error_code(error::route_not_exists) };
    }

  public:
    node() = default;

    node(const node&) = delete;

    node& operator=(const node&) = delete;

    node(node&&) = default;

    node& operator=(node&&) = default;

    node(std::string_view prefix)
        : prefix_(std::string(prefix))
    {
    }

    auto try_insert(route_type route) -> expected<void, std::error_code>
    {
      return try_insert(std::move(route), 0);
    }

    auto optimize() -> std::size_t
    {
      std::size_t total = optimize_statics();

      if (!routes_.empty()) {
        ++total;
      }
      if (params_) {
        total += params_->optimize();
      }
      if (!wildcard_.empty()) {
        ++total;
      }

      return total;
    }

    auto try_find(http::verb method, std::string_view path) const
        -> expected<const route_type&, std::error_code>
    {
      if (path.empty()) {
        if (auto res = try_find_route(method, routes_); res) {
          return res;
        }

        return try_find_route(method, wildcard_);
      }

      return try_find_static(method, path)
          .or_else([&](auto&&) { return try_find_param(method, path); })
          .or_else([&](auto&&) { return try_find_route(method, wildcard_); });
    }
  };

  node root_;

public:
  auto try_insert(route_type route) -> expected<void, std::error_code>
  {
    return root_.try_insert(std::move(route));
  }

  auto optimize() -> std::size_t
  {
    return root_.optimize();
  }

  auto try_find(http::verb method, std::string_view path) const
      -> expected<const route_type&, std::error_code>
  {
    return root_.try_find(method, path);
  }
};

}

FITORIA_NAMESPACE_END

#endif
