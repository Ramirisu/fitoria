//
// Copyright (c) 2022 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include <fitoria/core/config.hpp>

#include <fitoria/core/handler_concept.hpp>
#include <fitoria/core/http.hpp>
#include <fitoria/core/utility.hpp>

#include <fitoria/http_server/router.hpp>
#include <fitoria/http_server/router_error.hpp>

#include <memory>
#include <string>
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

    struct string_hash {
      using is_transparent = void;
      size_t operator()(const char* txt) const
      {
        return std::hash<std::string_view> {}(txt);
      }

      size_t operator()(string_view txt) const
      {
        return std::hash<string_view> {}(txt);
      }

      size_t operator()(const std::string& txt) const
      {
        return std::hash<std::string> {}(txt);
      }
    };

    struct equal_to {
      using is_transparent = void;

      template <class T, class U>
      constexpr auto operator()(T&& lhs, U&& rhs) const
          -> decltype(std::forward<T>(lhs) == std::forward<U>(rhs))
      {
        return std::forward<T>(lhs) == std::forward<U>(rhs);
      }

      constexpr auto operator()(string_view sv, const std::string& s) const
      {
        return sv == string_view(s);
      }
    };

  private:
    auto try_insert(const router_type& r,
                    const std::vector<string_view>& path_tokens,
                    std::size_t path_index) noexcept
        -> expected<void, router_error>
    {
      if (path_index == path_tokens.size()) {
        if (router_) {
          return unexpected<router_error>(router_error::route_already_exists);
        }

        router_.emplace(r);
        return {};
      }

      const auto& token = path_tokens[path_index];
      if (token.starts_with(':')) {
        if (!param_trees_) {
          param_trees_.emplace(std::make_shared<node>());
        }

        return param_trees_.value()->try_insert(r, path_tokens, path_index + 1);
      }

      return path_trees_[std::string(token)].try_insert(r, path_tokens,
                                                        path_index + 1);
    }

    auto try_find(const std::vector<string_view>& path_tokens,
                  std::size_t path_index) const noexcept
        -> expected<const router_type&, router_error>
    {
      if (path_index == path_tokens.size()) {
        return expected<const router_type&, router_error>(router_.value());
      }

      return try_find_path_trees(path_tokens[path_index])
          .to_expected_or(router_error::route_not_exists)
          .and_then([&](auto&& node) {
            return node.try_find(path_tokens, path_index + 1);
          })
          .or_else([&](auto&& error) {
            return param_trees_.to_expected_or(error).and_then(
                [&](auto&& node_ptr)
                    -> expected<const router_type&, router_error> {
                  return node_ptr->try_find(path_tokens, path_index + 1);
                });
          });
    }

    auto try_find_path_trees(string_view token) const noexcept
        -> optional<const node&>
    {
      if (auto iter = path_trees_.find(token); iter != path_trees_.end()) {
        return iter->second;
      }

      return nullopt;
    }

    static auto try_parse_path(string_view path) noexcept
        -> expected<std::vector<string_view>, router_error>
    {
      if (path.empty() || !path.starts_with('/')) {
        return unexpected<router_error>(router_error::parse_path_error);
      }

      auto split
          = [](string_view p) noexcept -> std::tuple<string_view, string_view> {
        p.remove_prefix(1);
        auto pos = p.find('/');
        if (pos == string_view::npos) {
          return { p.substr(0, pos), {} };
        }
        return { p.substr(0, pos), p.substr(pos) };
      };

      std::vector<string_view> tokens;
      while (!path.empty()) {
        string_view token;
        std::tie(token, path) = split(path);
        if (token.empty() || (token.starts_with(':') && token.size() == 1)) {
          return unexpected<router_error>(router_error::parse_path_error);
        }
        tokens.push_back(token);
      }

      return tokens;
    }

    optional<router_type> router_;
    std::unordered_map<std::string, node, string_hash, equal_to> path_trees_;
    optional<std::shared_ptr<node>> param_trees_;
  };

public:
  auto try_insert(const router_type& r) noexcept -> expected<void, router_error>
  {
    return node::try_parse_path(r.path()).and_then([&](auto&& path_tokens) {
      return subtrees_[r.method()].try_insert(r, path_tokens, 0);
    });
  }

  auto try_find(methods method, string_view path) const noexcept
      -> expected<const router_type&, router_error>
  {
    return node::try_parse_path(path).and_then([&](auto&& path_tokens) {
      return try_find(method)
          .to_expected_or(router_error::route_not_exists)
          .and_then([&](auto&& node) { return node.try_find(path_tokens, 0); });
    });
  }

private:
  auto try_find(methods method) const noexcept -> optional<const node&>
  {
    if (auto iter = subtrees_.find(method); iter != subtrees_.end()) {
      return iter->second;
    }

    return nullopt;
  }

  std::unordered_map<methods, node> subtrees_;
};

FITORIA_NAMESPACE_END
