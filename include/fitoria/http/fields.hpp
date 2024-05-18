//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#ifndef FITORIA_HTTP_FIELDS_HPP
#define FITORIA_HTTP_FIELDS_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/core/format.hpp>
#include <fitoria/core/optional.hpp>
#include <fitoria/core/utility.hpp>

#include <fitoria/http/field.hpp>

FITORIA_NAMESPACE_BEGIN

namespace http::fields {
namespace content_type {

  /// @verbatim embed:rst:leading-slashes
  ///
  /// ``"text/plain"``
  ///
  /// @endverbatim
  inline std::string_view plaintext() noexcept
  {
    return "text/plain";
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// ``"text/html"``
  ///
  /// @endverbatim
  inline std::string_view html() noexcept
  {
    return "text/html";
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// ``"text/xml"``
  ///
  /// @endverbatim
  inline std::string_view xml() noexcept
  {
    return "text/xml";
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// ``"application/x-www-form-urlencoded"``
  ///
  /// @endverbatim
  inline std::string_view form_urlencoded() noexcept
  {
    return "application/x-www-form-urlencoded";
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// ``"application/json"``
  ///
  /// @endverbatim
  inline std::string_view json() noexcept
  {
    return "application/json";
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// ``"application/octet-stream"``
  ///
  /// @endverbatim
  inline std::string_view octet_stream() noexcept
  {
    return "application/octet-stream";
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// ``"image/jpeg"``
  ///
  /// @endverbatim
  inline std::string_view jpeg() noexcept
  {
    return "image/jpeg";
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// ``"image/png"``
  ///
  /// @endverbatim
  inline std::string_view png() noexcept
  {
    return "image/png";
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// ``"image/gif"``
  ///
  /// @endverbatim
  inline std::string_view gif() noexcept
  {
    return "image/gif";
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// ``"image/svg+xml"``
  ///
  /// @endverbatim
  inline std::string_view svg() noexcept
  {
    return "image/svg+xml";
  }
}

namespace content_encoding {

  /// @verbatim embed:rst:leading-slashes
  ///
  /// ``"identity"``
  ///
  /// @endverbatim
  inline std::string_view identity() noexcept
  {
    return "identity";
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// ``"deflate"``
  ///
  /// @endverbatim
  inline std::string_view deflate() noexcept
  {
    return "deflate";
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// ``"gzip"``
  ///
  /// @endverbatim
  inline std::string_view gzip() noexcept
  {
    return "gzip";
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// ``"brotli"``
  ///
  /// @endverbatim
  inline std::string_view brotli() noexcept
  {
    return "brotli";
  }
}

namespace authorization {

  /// @verbatim embed:rst:leading-slashes
  ///
  /// ``"Bearer: {token}"``
  ///
  /// @endverbatim
  inline auto bearer(std::string_view token) -> std::string
  {
    return fmt::format("Bearer: {}", token);
  }

  inline auto
  parse_bearer(std::string_view str) noexcept -> optional<std::string_view>
  {
    const auto prefix = std::string_view("Bearer: ");
    if (str.starts_with(prefix)) {
      return str.substr(prefix.size());
    }

    return nullopt;
  }
}

namespace expect {

  inline auto one_hundred_continue() noexcept -> std::string_view
  {
    return "100-continue";
  }
}
}

FITORIA_NAMESPACE_END

#endif
