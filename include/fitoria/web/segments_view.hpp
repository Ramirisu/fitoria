//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef FITORIA_WEB_SEGMENTS_VIEW_HPP
#define FITORIA_WEB_SEGMENTS_VIEW_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/core/expected.hpp>
#include <fitoria/core/optional.hpp>
#include <fitoria/core/url.hpp>

#include <fitoria/web/error.hpp>
#include <fitoria/web/query_map.hpp>

#include <string>
#include <string_view>
#include <vector>

FITORIA_NAMESPACE_BEGIN

struct segment_view {
  bool is_param;
  std::string_view original;
  std::string_view escaped;
};

class segments_view : public std::vector<segment_view> {
public:
  segments_view(std::vector<segment_view> segments)
      : std::vector<segment_view>(std::move(segments))
  {
  }

  static auto from_path(std::string_view path)
      -> expected<segments_view, error_code>
  {
    std::vector<segment_view> segments;
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
        segments.push_back(segment_view {
            original != escaped,
            original,
            escaped.value(),
        });
      } else {
        return unexpected { make_error_code(error::route_parse_error) };
      }
    }

    return segments_view(segments);
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
    auto router_segs = from_path(router_path);
    auto req_segs = from_path(req_path);

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

#endif
