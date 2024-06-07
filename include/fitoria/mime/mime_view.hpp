//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#ifndef FITORIA_MIME_MIME_VIEW_HPP
#define FITORIA_MIME_MIME_VIEW_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/core/expected.hpp>
#include <fitoria/core/optional.hpp>
#include <fitoria/core/strings.hpp>
#include <fitoria/core/utility.hpp>

#include <fitoria/mime/params_view.hpp>

#include <string_view>
#include <system_error>

FITORIA_NAMESPACE_BEGIN

namespace mime {

/// @verbatim embed:rst:leading-slashes
///
/// A non-owning reference to a valid MIME type.
///
/// @endverbatim
class mime_view {
public:
  /// @verbatim embed:rst:leading-slashes
  ///
  /// Get complete string of the mime.
  ///
  /// DESCRIPTION
  ///     Get complete string of the mime.
  ///
  ///     - ``"text/plain; charset=utf-8"`` -> ``"text/plain; charset=utf-8"``
  ///     - ``"image/svg+xml"`` -> ``"image/svg+xml"``
  ///
  /// @endverbatim
  auto source() const noexcept -> std::string_view
  {
    return source_;
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Get **essense** part of the mime.
  ///
  /// DESCRIPTION
  ///     Get **essense** part of the mime.
  ///
  ///     - ``"text/plain; charset=utf-8"`` -> ``"text/plain"``
  ///     - ``"image/svg+xml"`` -> ``"image/svg+xml"``
  ///
  /// @endverbatim
  auto essence() const noexcept -> std::string_view
  {
    return essence_;
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Get **type** part of the mime.
  ///
  /// DESCRIPTION
  ///     Get **type** part of the mime.
  ///
  ///     - ``"text/plain; charset=utf-8"`` -> ``"text"``
  ///     - ``"image/svg+xml"`` -> ``"image"``
  ///
  /// @endverbatim
  auto type() const noexcept -> std::string_view
  {
    return type_;
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Get **subtype** part of the mime.
  ///
  /// DESCRIPTION
  ///     Get **subtype** part of the mime.
  ///
  ///     - ``"text/plain; charset=utf-8"`` -> ``"plain"``
  ///     - ``"image/svg+xml"`` -> ``"svg"``
  ///
  /// @endverbatim
  auto subtype() const noexcept -> std::string_view
  {
    return subtype_;
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Get **suffix** part of the mime.
  ///
  /// DESCRIPTION
  ///     Get **suffix** part of the mime.
  ///
  ///     - ``"text/plain; charset=utf-8"`` -> ``""``
  ///     - ``"image/svg+xml"`` -> ``"xml"``
  ///
  /// @endverbatim
  auto suffix() const noexcept -> optional<std::string_view>
  {
    return suffix_;
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Get parameters of the mime.
  ///
  /// DESCRIPTION
  ///     Get parameters of the mime.
  ///
  ///     - ``"text/plain; charset=utf-8"`` -> ``{ { "charset", "utf-8" } }``
  ///     - ``"image/svg+xml"`` -> ``{}``
  ///
  /// @endverbatim
  auto params() const noexcept -> const params_view&
  {
    return params_;
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Get complete string of the mime.
  ///
  /// DESCRIPTION
  ///     Get complete string of the mime.
  ///
  ///     - ``"text/plain; charset=utf-8"`` -> ``"text/plain; charset=utf-8"``
  ///     - ``"image/svg+xml"`` -> ``"image/svg+xml"``
  ///
  /// @endverbatim
  operator std::string_view() const noexcept
  {
    return source();
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Parses a string into ``mime_view``.
  ///
  /// @endverbatim
  static auto
  parse(std::string_view str) -> expected<mime_view, std::error_code>
  {
    // https://datatracker.ietf.org/doc/html/rfc2045#section-5.1
    // https://datatracker.ietf.org/doc/html/rfc6838
    // mime-type = type "/" subtype ["+" suffix]* [";" parameter];
    auto tokens = split_of(str, ";");
    auto primary = parse_primary(tokens[0]);
    if (!primary) {
      return unexpected { primary.error() };
    }
    auto essence = tokens[0];
    tokens.erase(tokens.begin());
    if (tokens.empty()) {
      return mime_view(str,
                       essence,
                       std::get<0>(*primary),
                       std::get<1>(*primary),
                       std::get<2>(*primary),
                       {});
    }

    auto params = parse_params(tokens);
    if (!params) {
      return unexpected { params.error() };
    }

    return mime_view(str,
                     essence,
                     std::get<0>(*primary),
                     std::get<1>(*primary),
                     std::get<2>(*primary),
                     std::move(*params));
  }

  friend bool operator==(const mime_view& lhs, const mime_view& rhs) noexcept
  {
    return iequals(lhs.source_, rhs.source_);
  }

  friend bool operator==(const mime_view& lhs, std::string_view rhs) noexcept
  {
    return iequals(lhs.source_, rhs);
  }

private:
  mime_view(std::string_view source,
            std::string_view essence,
            std::string_view type,
            std::string_view subtype,
            optional<std::string_view> suffix,
            params_view params)
      : source_(source)
      , essence_(essence)
      , type_(type)
      , subtype_(subtype)
      , suffix_(suffix)
      , params_(std::move(params))
  {
  }

  static auto parse_primary(std::string_view str) noexcept
      -> expected<std::tuple<std::string_view,
                             std::string_view,
                             optional<std::string_view>>,
                  std::error_code>
  {
    // mime-type = type "/" subtype ["+" suffix]*

    auto types = split_of(str, "/");
    auto type = types[0];
    if (types.size() != 2 || !is_valid_name(type)) {
      return unexpected { make_error_code(std::errc::invalid_argument) };
    }

    auto subtypes = split_of(types[1], "+");
    auto subtype = subtypes[0];
    if (subtypes.size() > 2 || !is_valid_name(subtype)) {
      return unexpected { make_error_code(std::errc::invalid_argument) };
    }
    if (subtypes.size() <= 1) {
      return std::tuple { type, subtype, nullopt };
    }

    auto suffix = subtypes[1];
    if (!is_valid_name(suffix)) {
      return unexpected { make_error_code(std::errc::invalid_argument) };
    }

    return std::tuple { type, subtype, suffix };
  }

  static auto parse_params(std::vector<std::string_view> strs) noexcept
      -> expected<params_view, std::error_code>
  {
    auto params = params_view();
    for (auto& str : strs) {
      if (auto tokens = split_of(str, "="); tokens.size() == 1) {
        params[tokens[0]] = std::string_view();
      } else if (tokens.size() == 2) {
        params[tokens[0]] = tokens[1];
      } else {
        return unexpected { make_error_code(std::errc::invalid_argument) };
      }
    }

    return params;
  }

  static auto is_valid_name(std::string_view str) noexcept -> bool
  {
    if (str.empty() || !std::isalnum(str[0])) {
      return false;
    }
    str.remove_prefix(1);

    return std::all_of(str.begin(), str.end(), [](char c) {
      return std::isalnum(c) || c == '!' || c == '#' || c == '$' || c == '&'
          || c == '-' || c == '^' || c == '_';
    });
  }

  std::string_view source_;
  std::string_view essence_;
  std::string_view type_;
  std::string_view subtype_;
  optional<std::string_view> suffix_;
  params_view params_;
};

/// @verbatim embed:rst:leading-slashes
///
/// ``"application/javascript"``
///
/// @endverbatim
inline auto application_javascript() noexcept -> const mime_view&
{
  static auto m = *mime_view::parse("application/javascript");
  return m;
}

/// @verbatim embed:rst:leading-slashes
///
/// ``"application/json"``
///
/// @endverbatim
inline auto application_json() noexcept -> const mime_view&
{
  static auto m = *mime_view::parse("application/json");
  return m;
}

/// @verbatim embed:rst:leading-slashes
///
/// ``"application/octet-stream"``
///
/// @endverbatim
inline auto application_octet_stream() noexcept -> const mime_view&
{
  static auto m = *mime_view::parse("application/octet-stream");
  return m;
}

/// @verbatim embed:rst:leading-slashes
///
/// ``"application/pdf"``
///
/// @endverbatim
inline auto application_pdf() noexcept -> const mime_view&
{
  static auto m = *mime_view::parse("application/pdf");
  return m;
}

/// @verbatim embed:rst:leading-slashes
///
/// ``"application/x-www-form-urlencoded"``
///
/// @endverbatim
inline auto application_www_form_urlencoded() noexcept -> const mime_view&
{
  static auto m = *mime_view::parse("application/x-www-form-urlencoded");
  return m;
}

/// @verbatim embed:rst:leading-slashes
///
/// ``"image/bmp"``
///
/// @endverbatim
inline auto image_bmp() noexcept -> const mime_view&
{
  static auto m = *mime_view::parse("image/bmp");
  return m;
}

/// @verbatim embed:rst:leading-slashes
///
/// ``"image/gif"``
///
/// @endverbatim
inline auto image_gif() noexcept -> const mime_view&
{
  static auto m = *mime_view::parse("image/gif");
  return m;
}

/// @verbatim embed:rst:leading-slashes
///
/// ``"image/jpeg"``
///
/// @endverbatim
inline auto image_jpeg() noexcept -> const mime_view&
{
  static auto m = *mime_view::parse("image/jpeg");
  return m;
}

/// @verbatim embed:rst:leading-slashes
///
/// ``"image/png"``
///
/// @endverbatim
inline auto image_png() noexcept -> const mime_view&
{
  static auto m = *mime_view::parse("image/png");
  return m;
}

/// @verbatim embed:rst:leading-slashes
///
/// ``"image/svg+xml"``
///
/// @endverbatim
inline auto image_svg() noexcept -> const mime_view&
{
  static auto m = *mime_view::parse("image/svg+xml");
  return m;
}

/// @verbatim embed:rst:leading-slashes
///
/// ``"text/html"``
///
/// @endverbatim
inline auto text_html() noexcept -> const mime_view&
{
  static auto m = *mime_view::parse("text/html");
  return m;
}

/// @verbatim embed:rst:leading-slashes
///
/// ``"text/javascript"``
///
/// @endverbatim
inline auto text_javascript() noexcept -> const mime_view&
{
  static auto m = *mime_view::parse("text/javascript");
  return m;
}

/// @verbatim embed:rst:leading-slashes
///
/// ``"text/plain"``
///
/// @endverbatim
inline auto text_plain() noexcept -> const mime_view&
{
  static auto m = *mime_view::parse("text/plain");
  return m;
}

/// @verbatim embed:rst:leading-slashes
///
/// ``"text/xml"``
///
/// @endverbatim
inline auto text_xml() noexcept -> const mime_view&
{
  static auto m = *mime_view::parse("text/xml");
  return m;
}

}

FITORIA_NAMESPACE_END

#endif
