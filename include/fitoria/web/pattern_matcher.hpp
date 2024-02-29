//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#ifndef FITORIA_WEB_PATTERN_MATCHER_HPP
#define FITORIA_WEB_PATTERN_MATCHER_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/core/fixed_string.hpp>
#include <fitoria/core/optional.hpp>

#include <fitoria/web/query_map.hpp>

#include <set>
#include <string>
#include <vector>

FITORIA_NAMESPACE_BEGIN

namespace web {

enum class segment_kind {
  static_,
  parameterized,
};

struct segment_t {
  segment_kind kind;
  std::string value;
};

using segments_t = std::vector<segment_t>;

inline auto escape_segment(std::string_view seg) noexcept
    -> optional<std::string_view>
{
  if (seg.starts_with('{')) {
    if (!seg.ends_with('}')) {
      return nullopt;
    }

    seg.remove_prefix(1);
    seg.remove_suffix(1);
  }

  if (seg.find_first_of("{}") != std::string_view::npos) {
    return nullopt;
  }

  return seg;
}

inline auto parse_pattern(std::string_view pattern) -> optional<segments_t>
{
  segments_t segments;
  std::set<std::string_view> used_params;

  while (!pattern.empty()) {
    if (!pattern.starts_with('/')) {
      return nullopt;
    }
    pattern.remove_prefix(1);

    auto seg = std::string_view();
    if (auto pos = pattern.find('/'); pos != std::string_view::npos) {
      seg = pattern.substr(0, pos);
      pattern.remove_prefix(pos);
    } else {
      seg = pattern;
      pattern = {};
    }

    if (auto escaped = escape_segment(seg); !escaped) {
      return nullopt;
    } else {
      auto seg_kind = escaped == seg ? segment_kind::static_
                                     : segment_kind::parameterized;
      if (seg_kind == segment_kind::parameterized) {
        if (used_params.contains(*escaped)) {
          return nullopt;
        }
        used_params.insert(*escaped);
      }
      segments.push_back(
          segment_t { .kind = seg_kind, .value = std::string(*escaped) });
    }
  }

  return segments;
}

class pattern_matcher {
public:
  pattern_matcher(std::string_view pattern)
      : pattern_(std::string(pattern))
      , segments_(parse_pattern(pattern).value())
  {
  }

  auto pattern() const noexcept -> std::string_view
  {
    return pattern_;
  }

  auto segments() const noexcept -> const segments_t&
  {
    return segments_;
  }

  auto match(std::string_view input) const -> optional<query_map>
  {
    query_map map;
    for (auto& segment : segments_) {
      if (input.empty() || !input.starts_with('/')) {
        return nullopt;
      }
      input.remove_prefix(1);

      auto seg = std::string_view();
      if (auto pos = input.find('/'); pos != std::string_view::npos) {
        seg = input.substr(0, pos);
        input.remove_prefix(pos);
      } else {
        seg = input;
        input = {};
      }

      if (segment.kind == segment_kind::parameterized) {
        map[segment.value] = seg;
      }
    }

    if (!input.empty()) {
      return nullopt;
    }

    return map;
  }

private:
  std::string pattern_;
  segments_t segments_;
};
}

FITORIA_NAMESPACE_END

#endif
