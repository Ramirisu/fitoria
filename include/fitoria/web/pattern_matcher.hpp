//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef FITORIA_WEB_PATTERN_MATCHER_HPP
#define FITORIA_WEB_PATTERN_MATCHER_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/core/optional.hpp>

#include <fitoria/web/query_map.hpp>

#include <set>
#include <string>
#include <vector>

FITORIA_NAMESPACE_BEGIN

namespace web {

class pattern_matcher {
public:
  enum class segment_kind {
    static_,
    parameterized,
  };

  struct segment_t {
    segment_kind kind;
    std::string value;
  };

  using segments_t = std::vector<segment_t>;

  auto pattern() const noexcept -> const std::string&
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

  static auto from_pattern(std::string pattern) -> optional<pattern_matcher>
  {
    auto pattern_view = std::string_view(pattern);
    segments_t segments;
    std::set<std::string_view> used_params;

    while (!pattern_view.empty()) {
      if (!pattern_view.starts_with('/')) {
        return nullopt;
      }
      pattern_view.remove_prefix(1);

      auto seg = std::string_view();
      if (auto pos = pattern_view.find('/'); pos != std::string_view::npos) {
        seg = pattern_view.substr(0, pos);
        pattern_view.remove_prefix(pos);
      } else {
        seg = pattern_view;
        pattern_view = {};
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

    return pattern_matcher(std::move(pattern), std::move(segments));
  }

  static auto escape_segment(std::string_view seg) noexcept
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

private:
  pattern_matcher(std::string pattern, segments_t segments)
      : pattern_(std::move(pattern))
      , segments_(std::move(segments))
  {
  }

  std::string pattern_;
  segments_t segments_;
};
}

FITORIA_NAMESPACE_END

#endif
