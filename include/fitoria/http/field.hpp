//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#ifndef FITORIA_HTTP_FIELD_HPP
#define FITORIA_HTTP_FIELD_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/core/format.hpp>
#include <fitoria/core/http.hpp>
#include <fitoria/core/optional.hpp>
#include <fitoria/core/utility.hpp>

FITORIA_NAMESPACE_BEGIN

namespace http {

using boost::beast::http::field;

namespace fields {
  namespace content_type {

    inline std::string_view plaintext() noexcept
    {
      return "text/plain; charset=utf-8";
    }

    inline std::string_view html() noexcept
    {
      return "text/html";
    }

    inline std::string_view xml() noexcept
    {
      return "text/xml";
    }

    inline std::string_view form_urlencoded() noexcept
    {
      return "application/x-www-form-urlencoded";
    }

    inline std::string_view json() noexcept
    {
      return "application/json";
    }

    inline std::string_view octet_stream() noexcept
    {
      return "application/octet-stream";
    }

    inline std::string_view jpeg() noexcept
    {
      return "image/jpeg";
    }

    inline std::string_view png() noexcept
    {
      return "image/png";
    }

    inline std::string_view gif() noexcept
    {
      return "image/gif";
    }

    inline std::string_view svg() noexcept
    {
      return "image/svg+xml";
    }
  }

  namespace content_encoding {

    inline std::string_view identity() noexcept
    {
      return "identity";
    }

    inline std::string_view deflate() noexcept
    {
      return "deflate";
    }

    inline std::string_view gzip() noexcept
    {
      return "gzip";
    }

    inline std::string_view brotli() noexcept
    {
      return "brotli";
    }
  }

  namespace authorization {

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
}

FITORIA_NAMESPACE_END

#endif
