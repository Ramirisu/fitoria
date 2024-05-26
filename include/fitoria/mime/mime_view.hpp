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
#include <fitoria/core/utility.hpp>

#include <string_view>
#include <system_error>

FITORIA_NAMESPACE_BEGIN

namespace mime {

class mime_view {
public:
  auto source() const noexcept -> std::string_view
  {
    return source_;
  }

  auto type() const noexcept -> std::string_view
  {
    return type_;
  }

  auto subtype() const noexcept -> std::string_view
  {
    return subtype_;
  }

  auto suffix() const noexcept -> std::string_view
  {
    return suffix_;
  }

  operator std::string_view() const noexcept
  {
    return source();
  }

  static auto
  parse(std::string_view str) -> expected<mime_view, std::error_code>
  {
    // https://datatracker.ietf.org/doc/html/rfc6838#section-4.2
    auto temp = str;
    auto type = temp.substr(0, temp.find('/'));
    if (!is_valid_name(type)) {
      return unexpected { make_error_code(std::errc::invalid_argument) };
    }
    temp.remove_prefix(type.size());
    if (temp.empty()) {
      return mime_view(str, type, {}, {});
    }
    temp.remove_prefix(1);

    auto subtype = temp.substr(0, temp.find('+'));
    if (!is_valid_name(subtype)) {
      return unexpected { make_error_code(std::errc::invalid_argument) };
    }
    temp.remove_prefix(subtype.size());
    if (temp.empty()) {
      return mime_view(str, type, subtype, {});
    }
    temp.remove_prefix(1);

    auto suffix = temp.substr(0, temp.find(';'));
    if (!is_valid_name(suffix)) {
      return unexpected { make_error_code(std::errc::invalid_argument) };
    }

    // TODO: parse `parameters`
    return mime_view(str, type, subtype, suffix);
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
            std::string_view type,
            std::string_view subtype,
            std::string_view suffix)
      : source_(source)
      , type_(type)
      , subtype_(subtype)
      , suffix_(suffix)
  {
  }

  static auto is_valid_name(std::string_view str) -> bool
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
  std::string_view type_;
  std::string_view subtype_;
  std::string_view suffix_;
};

inline auto application_javascript() -> mime_view
{
  return *mime_view::parse("application/javascript");
}

inline auto application_json() -> mime_view
{
  return *mime_view::parse("application/json");
}

inline auto application_octet_stream() -> mime_view
{
  return *mime_view::parse("application/octet-stream");
}

inline auto application_pdf() -> mime_view
{
  return *mime_view::parse("application/pdf");
}

inline auto application_www_form_urlencoded() -> mime_view
{
  return *mime_view::parse("application/x-www-form-urlencoded");
}

inline auto image_bmp() -> mime_view
{
  return *mime_view::parse("image/bmp");
}

inline auto image_gif() -> mime_view
{
  return *mime_view::parse("image/gif");
}

inline auto image_jpeg() -> mime_view
{
  return *mime_view::parse("image/jpeg");
}

inline auto image_png() -> mime_view
{
  return *mime_view::parse("image/png");
}

inline auto image_svg() -> mime_view
{
  return *mime_view::parse("image/svg+xml");
}

inline auto text_html() -> mime_view
{
  return *mime_view::parse("text/html");
}

inline auto text_javascript() -> mime_view
{
  return *mime_view::parse("text/javascript");
}

inline auto text_plain() -> mime_view
{
  return *mime_view::parse("text/plain");
}

inline auto text_xml() -> mime_view
{
  return *mime_view::parse("text/xml");
}

}

FITORIA_NAMESPACE_END

#endif
