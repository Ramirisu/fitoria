//
// Copyright (c) 2022 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include <fitoria/core/config.hpp>

#include <fitoria/core/url.hpp>
#include <fitoria/core/utility.hpp>

#include <fitoria/http_server/router_error.hpp>

#include <string>
#include <string_view>
#include <vector>

FITORIA_NAMESPACE_BEGIN

class route {
public:
  struct segment {
    bool is_param;
    std::string_view original;
    std::string_view escaped;
  };

  using segments = std::vector<segment>;

  static auto to_segments(std::string_view path) noexcept
      -> expected<segments, router_error>
  {
    segments segs;
    while (!path.empty()) {
      if (!path.starts_with('/')) {
        return unexpected<router_error>(router_error::parse_path_error);
      }
      path.remove_prefix(1);

      std::string_view original;
      if (auto next_pos = path.find('/'); next_pos != std::string_view::npos) {
        original = path.substr(0, next_pos);
        path = path.substr(next_pos);
      } else {
        original = path;
        path = {};
      }

      if (auto escaped = escape_segment(original);
          escaped && !escaped.value().empty()) {
        segs.push_back(segment {
            original != escaped,
            original,
            escaped.value(),
        });
      } else {
        return unexpected<router_error>(router_error::parse_path_error);
      }
    }

    return segs;
  }

  static auto escape_segment(std::string_view segment) noexcept
      -> expected<std::string_view, router_error>
  {
    if (segment.starts_with('{')) {
      if (!segment.ends_with('}')) {
        return unexpected<router_error>(router_error::parse_path_error);
      }

      segment.remove_prefix(1);
      segment.remove_suffix(1);
    }

    if (segment.find_first_of("{}") != std::string_view::npos) {
      return unexpected<router_error>(router_error::parse_path_error);
    }

    return segment;
  }

  static auto unescape_segment(std::string_view segment) noexcept -> std::string
  {
    std::string s("{");
    s += segment;
    s += "}";
    return s;
  }

  static auto parse_param_map(std::string_view router_path,
                              std::string_view req_path) noexcept
      -> expected<unordered_string_map<std::string>, router_error>
  {
    auto router_segs = to_segments(router_path);
    auto req_segs = to_segments(req_path);

    if (!router_segs || !req_segs || router_segs->size() != req_segs->size()) {
      return unexpected<router_error>(router_error::parse_path_error);
    }

    unordered_string_map<std::string> map;
    for (std::size_t i = 0; i < router_segs->size(); ++i) {
      if (router_segs.value()[i].is_param) {
        map[std::string(router_segs.value()[i].escaped)]
            = req_segs.value()[i].original;
      }
    }

    return map;
  }

  static auto to_unordered_string_map(urls::params_view params) noexcept
      -> unordered_string_map<std::string>
  {
    unordered_string_map<std::string> map;
    for (auto param : params) {
      if (param.has_value) {
        map[param.key] = param.value;
      }
    }

    return map;
  }
};

FITORIA_NAMESPACE_END
