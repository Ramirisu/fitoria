//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#ifndef FITORIA_HTTP_HEADER_RANGE_HPP
#define FITORIA_HTTP_HEADER_RANGE_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/core/from_string.hpp>
#include <fitoria/core/optional.hpp>
#include <fitoria/core/strings.hpp>

#include <string>
#include <string_view>
#include <vector>

FITORIA_NAMESPACE_BEGIN

namespace http::header {

class range {
public:
  struct offset_t {
    std::uint64_t start;
    std::uint64_t length;

    friend bool operator==(const offset_t&, const offset_t&) = default;
  };

  range(std::string unit, std::vector<offset_t> ranges)
      : unit_(std::move(unit))
      , ranges_(std::move(ranges))
  {
  }

  auto unit() const noexcept -> const std::string&
  {
    return unit_;
  }

  auto size() const noexcept -> std::size_t
  {
    return ranges_.size();
  }

  auto at(std::size_t index) const noexcept -> const offset_t&
  {
    return ranges_.at(index);
  }

  auto operator[](std::size_t index) const noexcept -> const offset_t&
  {
    return ranges_[index];
  }

  auto begin()
  {
    return ranges_.begin();
  }

  auto begin() const
  {
    return ranges_.begin();
  }

  auto cbegin() const
  {
    return ranges_.cbegin();
  }

  auto end()
  {
    return ranges_.end();
  }

  auto end() const
  {
    return ranges_.end();
  }

  auto cend() const
  {
    return ranges_.cend();
  }

  static auto parse(std::string_view input,
                    std::uint64_t total_length) noexcept -> optional<range>
  {
    // https://datatracker.ietf.org/doc/html/rfc2616#section-14.35

    auto unit = split_of(input, "=");
    if (unit.size() != 2) {
      return nullopt;
    }

    if (unit[1].starts_with("-")) {
      if (auto suffix = from_string<std::uint64_t>(unit[1].substr(1)); suffix) {
        return range(std::string(unit[0]),
                     { { total_length - *suffix, *suffix } });
      }
    } else {
      if (auto ranges = parse_ranges(unit[1], total_length); ranges) {
        return range(std::string(unit[0]), std::move(*ranges));
      }
    }

    return nullopt;
  }

private:
  static auto parse_ranges(std::string_view input,
                           std::uint64_t total_length) noexcept
      -> optional<std::vector<offset_t>>
  {
    // <range-start>-<range-end>, <range-start>-<range-end>, ...

    auto ranges = std::vector<offset_t>();
    auto tokens = split_of(input, ",");
    for (auto& token : tokens) {
      if (auto offset = split_of(token, "-"); offset.size() == 2) {
        auto start = from_string<std::uint64_t>(offset[0]);
        if (offset[1].empty() && start) {
          ranges.push_back(offset_t { *start, total_length - *start });
          continue;
        }
        auto end = from_string<std::uint64_t>(offset[1]);
        if (start && end && *start <= *end) {
          ranges.push_back(offset_t { *start, *end - *start + 1 });
          continue;
        }
      }

      return nullopt;
    }

    return ranges;
  }

  std::string unit_;
  std::vector<offset_t> ranges_;
};

}

FITORIA_NAMESPACE_END

#endif
