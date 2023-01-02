//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include <fitoria/core/config.hpp>

#include <fitoria/core/url.hpp>

#include <fitoria/http_server/error.hpp>
#include <fitoria/http_server/query_map.hpp>

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

  static auto to_segments(std::string_view path)
      -> expected<segments, error_code>
  {
    segments segs;
    while (!path.empty()) {
      if (!path.starts_with('/')) {
        return unexpected { make_error_code(error::route_parse_error) };
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
        return unexpected { make_error_code(error::route_parse_error) };
      }
    }

    return segs;
  }

  static auto escape_segment(std::string_view segment) noexcept
      -> expected<std::string_view, error_code>
  {
    if (segment.starts_with('{')) {
      if (!segment.ends_with('}')) {
        return unexpected { make_error_code(error::route_parse_error) };
      }

      segment.remove_prefix(1);
      segment.remove_suffix(1);
    }

    if (segment.find_first_of("{}") != std::string_view::npos) {
      return unexpected { make_error_code(error::route_parse_error) };
    }

    return segment;
  }

  static auto unescape_segment(std::string_view segment) -> std::string
  {
    std::string s("{");
    s += segment;
    s += "}";
    return s;
  }

  static auto parse_param_map(std::string_view router_path,
                              std::string_view req_path)
      -> expected<query_map, error_code>
  {
    auto router_segs = to_segments(router_path);
    auto req_segs = to_segments(req_path);

    if (!router_segs || !req_segs || router_segs->size() != req_segs->size()) {
      return unexpected { make_error_code(error::route_parse_error) };
    }

    query_map map;
    for (std::size_t i = 0; i < router_segs->size(); ++i) {
      if (router_segs.value()[i].is_param) {
        map[std::string(router_segs.value()[i].escaped)]
            = req_segs.value()[i].original;
      }
    }

    return map;
  }
};

FITORIA_NAMESPACE_END
