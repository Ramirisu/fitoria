//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#ifndef FITORIA_HTTP_HEADER_IF_NONE_MATCH_HPP
#define FITORIA_HTTP_HEADER_IF_NONE_MATCH_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/core/optional.hpp>
#include <fitoria/core/strings.hpp>

#include <fitoria/http/header/entity_tag.hpp>

FITORIA_NAMESPACE_BEGIN

namespace http::header {

/// @verbatim embed:rst:leading-slashes
///
/// Provides parsing for dealing with HTTP header ``If-None-Match``.
///
/// @endverbatim
class if_none_match {
public:
  /// @verbatim embed:rst:leading-slashes
  ///
  /// Check if it matches any resource, i.e. ``*``.
  ///
  /// @endverbatim
  auto is_any() const noexcept -> bool
  {
    return etags_.empty();
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Get the etags.
  ///
  /// @endverbatim
  auto etags() const noexcept -> const std::vector<entity_tag>&
  {
    return etags_;
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Get an iterator to the beginning of the underlying etags array.
  ///
  /// @endverbatim
  auto begin()
  {
    return etags_.begin();
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Get an iterator to the beginning of the underlying etags array.
  ///
  /// @endverbatim
  auto begin() const
  {
    return etags_.begin();
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Get an iterator to the beginning of the underlying etags array.
  ///
  /// @endverbatim
  auto cbegin() const
  {
    return etags_.cbegin();
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Get an iterator to the end of the underlying etags array.
  ///
  /// @endverbatim
  auto end()
  {
    return etags_.end();
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Get an iterator to the end of the underlying etags array.
  ///
  /// @endverbatim
  auto end() const
  {
    return etags_.end();
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Get an iterator to the end of the underlying etags array.
  ///
  /// @endverbatim
  auto cend() const
  {
    return etags_.cend();
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Parse the string to a ``if_none_match`` instance.
  ///
  /// @endverbatim
  static auto parse(std::string_view input) -> optional<if_none_match>
  {
    input = trim(input);
    if (input == "*") {
      return if_none_match();
    }

    std::vector<entity_tag> etags;
    auto tokens = split_of(input, ",");
    for (auto& token : tokens) {
      auto etag = entity_tag::parse(token);
      if (!etag) {
        return nullopt;
      }

      etags.push_back(std::move(*etag));
    }

    return if_none_match(std::move(etags));
  }

private:
  if_none_match() = default;

  explicit if_none_match(std::vector<entity_tag> etags)
      : etags_(std::move(etags))
  {
  }

  std::vector<entity_tag> etags_;
};

}

FITORIA_NAMESPACE_END

#endif
