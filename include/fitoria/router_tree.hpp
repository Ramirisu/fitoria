//
// Copyright (c) 2022 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include <fitoria/config.hpp>

#include <fitoria/method.hpp>
#include <fitoria/router.hpp>
#include <fitoria/router_error.hpp>

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

FITORIA_NAMESPACE_BEGIN

template <typename HandlerTrait>
class router_tree {
public:
  using handler_type = typename HandlerTrait::handler_type;
  using router_type = router<HandlerTrait>;

private:
  class node {
    friend class router_tree;

  private:
    auto try_insert(const router_type& r,
                    const std::vector<string_view>& path_tokens,
                    std::size_t path_index) noexcept
        -> expected<void, router_error>
    {
      if (path_index == path_tokens.size()) {
        if (handlers_) {
          return unexpected<router_error>(router_error::route_already_exists);
        }

        fullpath_ = r.path();
        handlers_ = r.middlewares();
        handlers_.value().push_back(r.handler());
        return {};
      }

      const auto& token = path_tokens[path_index];
      if (token.starts_with(':')) {
        if (!param_trees_) {
          param_trees_ = std::make_unique<node>();
        }

        return param_trees_->try_insert(r, path_tokens, path_index + 1);
      }

      return path_trees_[std::string(token)].try_insert(r, path_tokens,
                                                        path_index + 1);
    }

    static auto try_parse_path(string_view path) noexcept
        -> expected<std::vector<string_view>, router_error>
    {
      if (path.empty() || !path.starts_with('/')) {
        return unexpected<router_error>(router_error::parse_path_error);
      }

      auto split
          = [](string_view p) noexcept -> std::tuple<string_view, string_view> {
        p.remove_prefix(1); // curr '/'
        auto pos = p.find('/'); // next '/'
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

    std::string fullpath_;
    optional<std::vector<handler_type>> handlers_;
    std::unordered_map<std::string, node> path_trees_;
    std::unique_ptr<node> param_trees_;
  };

public:
  auto try_insert(const router_type& r) -> expected<void, router_error>
  {
    auto path_tokens = node::try_parse_path(r.path());
    if (!path_tokens) {
      return unexpected<router_error>(path_tokens.error());
    }

    return subtrees_[r.method()].try_insert(r, path_tokens.value(), 0);
  }

private:
  std::unordered_map<methods, node> subtrees_;
};

FITORIA_NAMESPACE_END
