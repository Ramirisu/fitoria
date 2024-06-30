//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#ifndef FITORIA_HTTP_HEADER_DATE_HPP
#define FITORIA_HTTP_HEADER_DATE_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/core/chrono.hpp>
#include <fitoria/core/format.hpp>
#include <fitoria/core/optional.hpp>
#include <fitoria/core/strings.hpp>

#include <chrono>
#include <sstream>
#include <string>

FITORIA_NAMESPACE_BEGIN

namespace http::header {

/// @verbatim embed:rst:leading-slashes
///
/// Provides parsing and serialization for dealing with date.
///
/// .. seealso::
///
///    https://datatracker.ietf.org/doc/html/rfc9110#name-date-time-formats
///
/// @endverbatim
class date {
public:
  /// @verbatim embed:rst:leading-slashes
  ///
  /// Construct by an UTC time.
  ///
  /// @endverbatim
  explicit date(std::chrono::time_point<chrono::utc_clock> time)
      : time_(time)
  {
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Construct by a FILE time.
  ///
  /// @endverbatim
  explicit date(std::chrono::time_point<std::chrono::file_clock> time)
      : time_(chrono::to_utc(time))
  {
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Get UTC time.
  ///
  /// @endverbatim
  auto utc_time() -> std::chrono::time_point<chrono::utc_clock>
  {
    return time_;
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Get FILE time.
  ///
  /// @endverbatim
  auto file_time() -> std::chrono::time_point<std::chrono::file_clock>
  {
    return chrono::from_utc(time_);
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Convert to string using ``rfc9110`` date and time format.
  ///
  /// @endverbatim
  auto to_string() -> std::string
  {
    return fmt::format("{:%a, %d %b %Y %T} GMT",
                       std::chrono::floor<std::chrono::seconds>(time_));
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Parse a string using ``rfc9110`` date and time format.
  ///
  /// @endverbatim
  static auto parse(std::string_view input) -> optional<date>
  {
    auto s = std::string(trim(input));
    auto iss = std::istringstream(std::string(s));
    auto time = std::chrono::time_point<chrono::utc_clock>();
    iss >> std::chrono::parse("%a, %d %b %Y %T GMT", time);
    if (iss.good()) {
      return date(time);
    }

    return nullopt;
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Compare two ``date`` s for equality.
  ///
  /// @endverbatim
  friend auto operator==(const date&, const date&) -> bool = default;

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Three-way compare two ``date`` s.
  ///
  /// @endverbatim
  friend auto operator<=>(const date&, const date&) = default;

private:
  std::chrono::time_point<chrono::utc_clock> time_;
};

}

FITORIA_NAMESPACE_END

#endif
