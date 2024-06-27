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

#include <fitoria/core/format.hpp>
#include <fitoria/core/optional.hpp>
#include <fitoria/core/strings.hpp>
#include <fitoria/core/utility.hpp>

#include <fitoria/mime/name_view.hpp>
#include <fitoria/mime/params_view.hpp>

#include <string_view>

FITORIA_NAMESPACE_BEGIN

namespace mime {

/// @verbatim embed:rst:leading-slashes
///
/// A non-owning reference to a valid MIME type.
///
/// @endverbatim
class mime_view {
public:
  mime_view(name_view source,
            name_view essence,
            name_view type,
            name_view subtype,
            optional<name_view> suffix,
            params_view params)
      : source_(source)
      , essence_(essence)
      , type_(type)
      , subtype_(subtype)
      , suffix_(suffix)
      , params_(std::move(params))
  {
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Get complete string of the mime.
  ///
  /// DESCRIPTION
  ///   Get complete string of the mime.
  ///
  ///   - ``"text/plain; charset=utf-8"`` -> ``"text/plain; charset=utf-8"``
  ///   - ``"image/svg+xml"`` -> ``"image/svg+xml"``
  ///
  /// @endverbatim
  auto source() const noexcept -> name_view
  {
    return source_;
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Get **essense** part of the mime.
  ///
  /// DESCRIPTION
  ///   Get **essense** part of the mime.
  ///
  ///   - ``"text/plain; charset=utf-8"`` -> ``"text/plain"``
  ///   - ``"image/svg+xml"`` -> ``"image/svg+xml"``
  ///
  /// @endverbatim
  auto essence() const noexcept -> name_view
  {
    return essence_;
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Get **type** part of the mime.
  ///
  /// DESCRIPTION
  ///   Get **type** part of the mime.
  ///
  ///   - ``"text/plain; charset=utf-8"`` -> ``"text"``
  ///   - ``"image/svg+xml"`` -> ``"image"``
  ///
  /// @endverbatim
  auto type() const noexcept -> name_view
  {
    return type_;
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Get **subtype** part of the mime.
  ///
  /// DESCRIPTION
  ///   Get **subtype** part of the mime.
  ///
  ///   - ``"text/plain; charset=utf-8"`` -> ``"plain"``
  ///   - ``"image/svg+xml"`` -> ``"svg"``
  ///
  /// @endverbatim
  auto subtype() const noexcept -> name_view
  {
    return subtype_;
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Get **suffix** part of the mime.
  ///
  /// DESCRIPTION
  ///   Get **suffix** part of the mime.
  ///
  ///   - ``"text/plain; charset=utf-8"`` -> ``nullopt``
  ///   - ``"image/svg+xml"`` -> ``"xml"``
  ///
  /// @endverbatim
  auto suffix() const noexcept -> optional<name_view>
  {
    return suffix_;
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Get parameters of the mime.
  ///
  /// DESCRIPTION
  ///   Get parameters of the mime.
  ///
  ///   - ``"text/plain; charset=utf-8"`` -> ``{ { "charset", "utf-8" } }``
  ///   - ``"image/svg+xml"`` -> ``{}``
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
  ///   Get complete string of the mime.
  ///
  ///   - ``"text/plain; charset=utf-8"`` -> ``"text/plain; charset=utf-8"``
  ///   - ``"image/svg+xml"`` -> ``"image/svg+xml"``
  ///
  /// @endverbatim
  operator std::string_view() const noexcept
  {
    return static_cast<std::string_view>(source_);
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Parses a string into ``mime_view``.
  ///
  /// @endverbatim
  static auto parse(std::string_view str) -> optional<mime_view>
  {
    // https://datatracker.ietf.org/doc/html/rfc2045#section-5.1
    // https://datatracker.ietf.org/doc/html/rfc6838#section-4.2
    // mime-type = type "/" subtype ["+" suffix]* [";" parameter];

    auto tokens = split_of(str, ";");
    auto primary = parse_primary(tokens[0]);
    if (!primary) {
      return nullopt;
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
      return nullopt;
    }

    return mime_view(str,
                     essence,
                     std::get<0>(*primary),
                     std::get<1>(*primary),
                     std::get<2>(*primary),
                     std::move(*params));
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Get the mime from the file extension.
  ///
  /// @endverbatim
  static auto
  from_extension(std::string_view ext) noexcept -> optional<mime_view>;

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Get the mime from the file path.
  ///
  /// @endverbatim
  static auto from_path(std::string_view path) noexcept -> optional<mime_view>
  {
    if (auto pos = path.rfind('.'); pos != std::string_view::npos) {
      return from_extension(path.substr(pos + 1));
    }

    return nullopt;
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Compare ``mime_view`` s for equality.
  ///
  /// DESCRIPTION
  ///   Compare ``mime_view`` s for equality. It's considered equivalent if
  ///   and only if the ``source`` of ``mime_view`` s are case-insensitively
  ///   equivalent.
  ///
  /// @endverbatim
  friend auto operator==(const mime_view& lhs, const mime_view& rhs) -> bool
  {
    return lhs.source_ == rhs.source_;
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Compare ``mime_view`` 's equivalence against a string.
  ///
  /// DESCRIPTION
  ///   Compare ``mime_view`` 's equivalence against a string. It's considered
  ///   equivalent if and only if the ``source`` of the ``mime_view`` are
  ///   case-insensitively equivalent to the string.
  ///
  /// @endverbatim
  friend auto operator==(const mime_view& lhs,
                         const std::string_view& rhs) -> bool
  {
    return lhs.source_ == rhs;
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Three-way compare ``mime_view`` s lexicographically.
  ///
  /// DESCRIPTION
  ///   Three-way compare ``mime_view`` s lexicographically. The comparison is
  ///   performed case-insensitively.
  ///
  /// @endverbatim
  friend auto operator<=>(const mime_view& lhs,
                          const mime_view& rhs) -> std::strong_ordering
  {
    return lhs.source_ <=> rhs.source_;
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Three-way compare ``mime_view`` lexicographically against a string.
  ///
  /// DESCRIPTION
  ///   Three-way compare ``mime_view`` lexicographically against a string. The
  ///   comparison is performed case-insensitively.
  ///
  /// @endverbatim
  friend auto operator<=>(const mime_view& lhs,
                          const std::string_view& rhs) -> std::strong_ordering
  {
    return lhs.source_ <=> rhs;
  }

private:
  static auto parse_primary(std::string_view str) noexcept
      -> optional<std::tuple<std::string_view,
                             std::string_view,
                             optional<std::string_view>>>
  {
    // https://datatracker.ietf.org/doc/html/rfc6838#section-4.2
    // mime-type = type "/" subtype ["+" suffix]*

    auto types = split_of(str, "/");
    auto type = types[0];
    if (types.size() != 2 || !is_valid_name(type)) {
      return nullopt;
    }

    auto subtypes = split_of(types[1], "+");
    auto subtype = subtypes[0];
    if (subtypes.size() > 2 || !is_valid_name(subtype)) {
      return nullopt;
    }
    if (subtypes.size() <= 1) {
      return std::tuple { type, subtype, nullopt };
    }

    auto suffix = subtypes[1];
    if (!is_valid_name(suffix)) {
      return nullopt;
    }

    return std::tuple { type, subtype, suffix };
  }

  static auto parse_params(std::vector<std::string_view> strs) noexcept
      -> optional<params_view>
  {
    // https://datatracker.ietf.org/doc/html/rfc2045#section-5.1

    auto params = params_view();
    for (auto& str : strs) {
      if (auto tokens = split_of(str, "="); tokens.size() == 2
          && is_valid_name(tokens[0]) && is_valid_name(tokens[1])) {
        params[tokens[0]] = tokens[1];
      } else {
        return nullopt;
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

  name_view source_;
  name_view essence_;
  name_view type_;
  name_view subtype_;
  optional<name_view> suffix_;
  params_view params_;
};

/// @verbatim embed:rst:leading-slashes
///
/// ``"application/javascript"``
///
/// @endverbatim
inline auto application_javascript() noexcept -> mime_view
{
  const auto source = std::string_view("application/javascript");
  return {
    source, source, source.substr(0, 11), source.substr(12), nullopt, {}
  };
}

/// @verbatim embed:rst:leading-slashes
///
/// ``"application/json"``
///
/// @endverbatim
inline auto application_json() noexcept -> mime_view
{
  const auto source = std::string_view("application/json");
  return {
    source, source, source.substr(0, 11), source.substr(12), nullopt, {}
  };
}

/// @verbatim embed:rst:leading-slashes
///
/// ``"application/msgpack"``
///
/// @endverbatim
inline auto application_msgpack() noexcept -> mime_view
{
  const auto source = std::string_view("application/msgpack");
  return {
    source, source, source.substr(0, 11), source.substr(12), nullopt, {}
  };
}

/// @verbatim embed:rst:leading-slashes
///
/// ``"application/octet-stream"``
///
/// @endverbatim
inline auto application_octet_stream() noexcept -> mime_view
{
  const auto source = std::string_view("application/octet-stream");
  return {
    source, source, source.substr(0, 11), source.substr(12), nullopt, {}
  };
}

/// @verbatim embed:rst:leading-slashes
///
/// ``"application/pdf"``
///
/// @endverbatim
inline auto application_pdf() noexcept -> mime_view
{
  const auto source = std::string_view("application/pdf");
  return {
    source, source, source.substr(0, 11), source.substr(12), nullopt, {}
  };
}

/// @verbatim embed:rst:leading-slashes
///
/// ``"application/x-shockwave-flash"``
///
/// @endverbatim
inline auto application_shockwave_flash() noexcept -> mime_view
{
  const auto source = std::string_view("application/x-shockwave-flash");
  return {
    source, source, source.substr(0, 11), source.substr(12), nullopt, {}
  };
}

/// @verbatim embed:rst:leading-slashes
///
/// ``"application/text"``
///
/// @endverbatim
inline auto application_text() noexcept -> mime_view
{
  const auto source = std::string_view("application/text");
  return {
    source, source, source.substr(0, 11), source.substr(12), nullopt, {}
  };
}

/// @verbatim embed:rst:leading-slashes
///
/// ``"application/wasm"``
///
/// @endverbatim
inline auto application_wasm() noexcept -> mime_view
{
  const auto source = std::string_view("application/wasm");
  return {
    source, source, source.substr(0, 11), source.substr(12), nullopt, {}
  };
}

/// @verbatim embed:rst:leading-slashes
///
/// ``"application/x-www-form-urlencoded"``
///
/// @endverbatim
inline auto application_www_form_urlencoded() noexcept -> mime_view
{
  const auto source = std::string_view("application/x-www-form-urlencoded");
  return {
    source, source, source.substr(0, 11), source.substr(12), nullopt, {}
  };
}

/// @verbatim embed:rst:leading-slashes
///
/// ``"application/xml"``
///
/// @endverbatim
inline auto application_xml() noexcept -> mime_view
{
  const auto source = std::string_view("application/xml");
  return {
    source, source, source.substr(0, 11), source.substr(12), nullopt, {}
  };
}

/// @verbatim embed:rst:leading-slashes
///
/// ``"image/bmp"``
///
/// @endverbatim
inline auto image_bmp() noexcept -> mime_view
{
  const auto source = std::string_view("image/bmp");
  return { source, source, source.substr(0, 5), source.substr(6), nullopt, {} };
}

/// @verbatim embed:rst:leading-slashes
///
/// ``"image/gif"``
///
/// @endverbatim
inline auto image_gif() noexcept -> mime_view
{
  const auto source = std::string_view("image/gif");
  return { source, source, source.substr(0, 5), source.substr(6), nullopt, {} };
}

/// @verbatim embed:rst:leading-slashes
///
/// ``"image/jpeg"``
///
/// @endverbatim
inline auto image_jpeg() noexcept -> mime_view
{
  const auto source = std::string_view("image/jpeg");
  return { source, source, source.substr(0, 5), source.substr(6), nullopt, {} };
}

/// @verbatim embed:rst:leading-slashes
///
/// ``"image/png"``
///
/// @endverbatim
inline auto image_png() noexcept -> mime_view
{
  const auto source = std::string_view("image/png");
  return { source, source, source.substr(0, 5), source.substr(6), nullopt, {} };
}

/// @verbatim embed:rst:leading-slashes
///
/// ``"image/svg+xml"``
///
/// @endverbatim
inline auto image_svg() noexcept -> mime_view
{
  const auto source = std::string_view("image/svg+xml");
  return {
    source, source, source.substr(0, 5), source.substr(6, 3), source.substr(10),
    {}
  };
}

/// @verbatim embed:rst:leading-slashes
///
/// ``"image/tiff"``
///
/// @endverbatim
inline auto image_tiff() noexcept -> mime_view
{
  const auto source = std::string_view("image/tiff");
  return { source, source, source.substr(0, 5), source.substr(6), nullopt, {} };
}

/// @verbatim embed:rst:leading-slashes
///
/// ``"multipart/form-data"``
///
/// @endverbatim
inline auto multipart_form_data() noexcept -> mime_view
{
  const auto source = std::string_view("multipart/form-data");
  return {
    source, source, source.substr(0, 9), source.substr(10), nullopt, {}
  };
}

/// @verbatim embed:rst:leading-slashes
///
/// ``"text/css"``
///
/// @endverbatim
inline auto text_css() noexcept -> mime_view
{
  const auto source = std::string_view("text/css");
  return { source, source, source.substr(0, 4), source.substr(5), nullopt, {} };
}

/// @verbatim embed:rst:leading-slashes
///
/// ``"text/csv"``
///
/// @endverbatim
inline auto text_csv() noexcept -> mime_view
{
  const auto source = std::string_view("text/csv");
  return { source, source, source.substr(0, 4), source.substr(5), nullopt, {} };
}

/// @verbatim embed:rst:leading-slashes
///
/// ``"text/html"``
///
/// @endverbatim
inline auto text_html() noexcept -> mime_view
{
  const auto source = std::string_view("text/html");
  return { source, source, source.substr(0, 4), source.substr(5), nullopt, {} };
}

/// @verbatim embed:rst:leading-slashes
///
/// ``"text/javascript"``
///
/// @endverbatim
inline auto text_javascript() noexcept -> mime_view
{
  const auto source = std::string_view("text/javascript");
  return { source, source, source.substr(0, 4), source.substr(5), nullopt, {} };
}

/// @verbatim embed:rst:leading-slashes
///
/// ``"text/plain"``
///
/// @endverbatim
inline auto text_plain() noexcept -> mime_view
{
  const auto source = std::string_view("text/plain");
  return { source, source, source.substr(0, 4), source.substr(5), nullopt, {} };
}

/// @verbatim embed:rst:leading-slashes
///
/// ``"text/xml"``
///
/// @endverbatim
inline auto text_xml() noexcept -> mime_view
{
  const auto source = std::string_view("text/xml");
  return { source, source, source.substr(0, 4), source.substr(5), nullopt, {} };
}

/// @verbatim embed:rst:leading-slashes
///
/// ``"video/x-flv"``
///
/// @endverbatim
inline auto video_flv() noexcept -> mime_view
{
  const auto source = std::string_view("video/x-flv");
  return { source, source, source.substr(0, 5), source.substr(6), nullopt, {} };
}

inline auto
mime_view::from_extension(std::string_view ext) noexcept -> optional<mime_view>
{
  if (cmp_eq_ci(ext, "js")) {
    return application_javascript();
  }
  if (cmp_eq_ci(ext, "json")) {
    return application_json();
  }
  if (cmp_eq_ci(ext, "swf")) {
    return application_shockwave_flash();
  }
  if (cmp_eq_ci(ext, "wasm")) {
    return application_wasm();
  }
  if (cmp_eq_ci(ext, "xml")) {
    return application_xml();
  }
  if (cmp_eq_ci(ext, "bmp")) {
    return image_bmp();
  }
  if (cmp_eq_ci(ext, "gif")) {
    return image_gif();
  }
  if (cmp_eq_ci(ext, "jpe") || cmp_eq_ci(ext, "jpeg")
      || cmp_eq_ci(ext, "jpg")) {
    return image_jpeg();
  }
  if (cmp_eq_ci(ext, "png")) {
    return image_png();
  }
  if (cmp_eq_ci(ext, "svg") || cmp_eq_ci(ext, "svgz")) {
    return image_svg();
  }
  if (cmp_eq_ci(ext, "tif") || cmp_eq_ci(ext, "tiff")) {
    return image_tiff();
  }
  if (cmp_eq_ci(ext, "css")) {
    return text_css();
  }
  if (cmp_eq_ci(ext, "htm") || cmp_eq_ci(ext, "html")
      || cmp_eq_ci(ext, "php")) {
    return text_html();
  }
  if (cmp_eq_ci(ext, "txt")) {
    return text_plain();
  }
  if (cmp_eq_ci(ext, "flv")) {
    return video_flv();
  }

  return nullopt;
}
}

FITORIA_NAMESPACE_END

template <>
struct fmt::formatter<FITORIA_NAMESPACE::mime::mime_view, char>
    : fmt::formatter<std::string_view, char> {
  template <typename FormatContext>
  auto format(const FITORIA_NAMESPACE::mime::mime_view& v,
              FormatContext& ctx) const
  {
    return fmt::formatter<std::string_view, char>::format(
        static_cast<std::string_view>(v), ctx);
  }
};

#endif
