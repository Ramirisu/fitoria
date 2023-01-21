//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef FITORIA_WEB_HTTP_HTTP_HPP
#define FITORIA_WEB_HTTP_HTTP_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/core/format.hpp>
#include <fitoria/core/http.hpp>
#include <fitoria/core/optional.hpp>
#include <fitoria/core/utility.hpp>

FITORIA_NAMESPACE_BEGIN

namespace http {

using boost::beast::http::field;
using boost::beast::http::status;
using boost::beast::http::status_class;
using boost::beast::http::verb;

using boost::beast::http::async_read;
using boost::beast::http::async_read_header;
using boost::beast::http::async_write;
using boost::beast::http::async_write_header;
using boost::beast::http::error;
using boost::beast::http::fields;
using boost::beast::http::message_generator;
using boost::beast::http::request;
using boost::beast::http::request_parser;
using boost::beast::http::request_serializer;
using boost::beast::http::response;
using boost::beast::http::string_body;
using boost::beast::http::to_status_class;

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

namespace authorization {

  inline std::string barear(std::string_view token)
  {
    return fmt::format("Bearer: {}", token);
  }

  inline optional<std::string_view> parse_barear(std::string_view str)
  {
    const auto prefix = std::string_view("Bearer: ");
    if (str.starts_with(prefix)) {
      return str.substr(prefix.size());
    }

    return nullopt;
  }
}

}

FITORIA_NAMESPACE_END

#endif
