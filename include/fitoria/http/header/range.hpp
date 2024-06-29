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

/// @verbatim embed:rst:leading-slashes
///
/// Provides parsing for dealing with HTTP header ``ETag``.
///
/// @endverbatim
class range {
public:
  struct subrange_t {
    std::uint64_t offset;
    std::uint64_t length;

    friend bool operator==(const subrange_t&, const subrange_t&) = default;
  };

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Constructor.
  ///
  /// @endverbatim
  range(std::string unit, std::vector<subrange_t> subranges)
      : unit_(std::move(unit))
      , subranges_(std::move(subranges))
  {
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Get the unit.
  ///
  /// @endverbatim
  auto unit() const noexcept -> const std::string&
  {
    return unit_;
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Get the count of subranges.
  ///
  /// @endverbatim
  auto size() const noexcept -> std::size_t
  {
    return subranges_.size();
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Get specific index of element from the underlying subrange array.
  ///
  /// @endverbatim
  auto at(std::size_t index) const noexcept -> const subrange_t&
  {
    return subranges_.at(index);
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Get specific index of element from the underlying subrange array.
  ///
  /// @endverbatim
  auto operator[](std::size_t index) const noexcept -> const subrange_t&
  {
    return subranges_[index];
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Get an iterator to the beginning of the underlying subrange array.
  ///
  /// @endverbatim
  auto begin()
  {
    return subranges_.begin();
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Get an iterator to the beginning of the underlying subrange array.
  ///
  /// @endverbatim
  auto begin() const
  {
    return subranges_.begin();
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Get an iterator to the beginning of the underlying subrange array.
  ///
  /// @endverbatim
  auto cbegin() const
  {
    return subranges_.cbegin();
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Get an iterator to the end of the underlying subrange array.
  ///
  /// @endverbatim
  auto end()
  {
    return subranges_.end();
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Get an iterator to the end of the underlying subrange array.
  ///
  /// @endverbatim
  auto end() const
  {
    return subranges_.end();
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Get an iterator to the end of the underlying subrange array.
  ///
  /// @endverbatim
  auto cend() const
  {
    return subranges_.cend();
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Parse the string to a ``range`` instance.
  ///
  /// @endverbatim
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
      if (auto subranges = parse_subranges(unit[1], total_length); subranges) {
        return range(std::string(unit[0]), std::move(*subranges));
      }
    }

    return nullopt;
  }

private:
  static auto parse_subranges(std::string_view input,
                              std::uint64_t total_length) noexcept
      -> optional<std::vector<subrange_t>>
  {
    // <range-start>-<range-end>, <range-start>-<range-end>, ...

    auto subranges = std::vector<subrange_t>();
    auto tokens = split_of(input, ",");
    for (auto& token : tokens) {
      if (auto subrange = split_of(token, "-"); subrange.size() == 2) {
        auto start = from_string<std::uint64_t>(subrange[0]);
        if (subrange[1].empty() && start) {
          subranges.push_back(subrange_t { *start, total_length - *start });
          continue;
        }
        auto end = from_string<std::uint64_t>(subrange[1]);
        if (start && end && *start <= *end) {
          subranges.push_back(subrange_t { *start, *end - *start + 1 });
          continue;
        }
      }

      return nullopt;
    }

    return subranges;
  }

  std::string unit_;
  std::vector<subrange_t> subranges_;
};

}

FITORIA_NAMESPACE_END

#endif
