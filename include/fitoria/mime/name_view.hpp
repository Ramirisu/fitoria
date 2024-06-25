//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#ifndef FITORIA_MIME_NAME_VIEW_HPP
#define FITORIA_MIME_NAME_VIEW_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/core/format.hpp>
#include <fitoria/core/strings.hpp>

FITORIA_NAMESPACE_BEGIN

namespace mime {

/// @verbatim embed:rst:leading-slashes
///
/// A non-owning reference to a name or value of an MIME type. Provides
/// case-insensitive comparison operators.
///
/// @endverbatim
class name_view {
public:
  /// @verbatim embed:rst:leading-slashes
  ///
  /// Default construct an empty ``name_view``.
  ///
  /// @endverbatim
  name_view() = default;

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Construct from a raw string.
  ///
  /// @endverbatim
  name_view(const char* s)
      : impl_(s)
  {
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Construct from a string view.
  ///
  /// @endverbatim
  name_view(std::string_view sv)
      : impl_(sv)
  {
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Convert to a string with lower-case ascii characters.
  ///
  /// @endverbatim
  auto to_string() const -> std::string
  {
    auto result = std::string();
    result.resize(impl_.size());
    std::transform(
        impl_.begin(), impl_.end(), result.begin(), [](auto c) -> char {
          return static_cast<char>(std::tolower(c));
        });

    return result;
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Return the underlying string_view.
  ///
  /// @endverbatim
  explicit operator std::string_view() const noexcept
  {
    return impl_;
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Compare ``name_view`` s for equality.
  ///
  /// DESCRIPTION
  ///   Compare ``name_view`` s for equality. It's considered equivalent if
  ///   and only if the ``source`` of ``name_view`` s are case-insensitively
  ///   equivalent.
  ///
  /// @endverbatim
  friend auto operator==(const name_view& lhs, const name_view& rhs) -> bool
  {
    return cmp_eq_ci(lhs.impl_, rhs.impl_);
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Compare ``name_view`` 's equivalence against a string.
  ///
  /// DESCRIPTION
  ///   Compare ``name_view`` 's equivalence against a string. It's considered
  ///   equivalent if and only if the ``source`` of the ``name_view`` are
  ///   case-insensitively equivalent to the string.
  ///
  /// @endverbatim
  friend auto operator==(const name_view& lhs,
                         const std::string_view& rhs) -> bool
  {
    return cmp_eq_ci(lhs.impl_, rhs);
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Compare ``name_view`` 's equivalence against a raw string.
  ///
  /// DESCRIPTION
  ///   Compare ``name_view`` 's equivalence against a raw string. It's
  ///   considered equivalent if and only if the ``source`` of the ``name_view``
  ///   are case-insensitively equivalent to the raw string.
  ///
  /// @endverbatim
  friend auto operator==(const name_view& lhs, const char* rhs) -> bool
  {
    return cmp_eq_ci(lhs.impl_, rhs);
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Three-way compare ``name_view`` s lexicographically.
  ///
  /// DESCRIPTION
  ///   Three-way compare ``name_view`` s lexicographically. The comparison is
  ///   performed case-insensitively.
  ///
  /// @endverbatim
  friend auto operator<=>(const name_view& lhs,
                          const name_view& rhs) -> std::strong_ordering
  {
    return cmp_tw_ci(lhs.impl_, rhs.impl_);
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Three-way compare ``name_view`` lexicographically against a string.
  ///
  /// DESCRIPTION
  ///   Three-way compare ``name_view`` lexicographically against a string. The
  ///   comparison is performed case-insensitively.
  ///
  /// @endverbatim
  friend auto operator<=>(const name_view& lhs,
                          const std::string_view& rhs) -> std::strong_ordering
  {
    return cmp_tw_ci(lhs.impl_, rhs);
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Three-way compare ``name_view`` lexicographically against a raw string.
  ///
  /// DESCRIPTION
  ///   Three-way compare ``name_view`` lexicographically against a raw string.
  ///   The comparison is performed case-insensitively.
  ///
  /// @endverbatim
  friend auto operator<=>(const name_view& lhs,
                          const char* rhs) -> std::strong_ordering
  {
    return cmp_tw_ci(lhs.impl_, rhs);
  }

private:
  std::string_view impl_;
};

}

FITORIA_NAMESPACE_END

template <>
struct fmt::formatter<FITORIA_NAMESPACE::mime::name_view, char>
    : fmt::formatter<std::string_view, char> {
  template <typename FormatContext>
  auto format(const FITORIA_NAMESPACE::mime::name_view& v,
              FormatContext& ctx) const
  {
    return fmt::formatter<std::string_view, char>::format(
        static_cast<std::string_view>(v), ctx);
  }
};

#endif
