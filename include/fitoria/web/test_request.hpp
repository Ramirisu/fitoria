//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#ifndef FITORIA_WEB_TEST_REQUEST_HPP
#define FITORIA_WEB_TEST_REQUEST_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/core/expected.hpp>
#include <fitoria/core/json.hpp>

#include <fitoria/http.hpp>
#include <fitoria/mime.hpp>

#include <fitoria/web/any_async_readable_stream.hpp>
#include <fitoria/web/any_body.hpp>
#include <fitoria/web/async_readable_vector_stream.hpp>
#include <fitoria/web/query_map.hpp>

#include <span>

FITORIA_NAMESPACE_BEGIN

namespace web {

class test_request_builder;

/// @verbatim embed:rst:leading-slashes
///
/// A type representing client's incoming request for testing purpose.
///
/// @endverbatim
class test_request {
  friend class test_request_builder;

  http::verb method_ = http::verb::unknown;
  http::version version_ = http::version::v1_1;
  http::header header_;
  query_map query_;
  any_body body_;

  test_request(http::verb method,
               http::version version,
               http::header header,
               query_map query,
               any_body body)
      : method_(method)
      , version_(version)
      , header_(std::move(header))
      , query_(std::move(query))
      , body_(std::move(body))
  {
  }

public:
  test_request() = default;

  test_request(const test_request&) = delete;

  test_request& operator=(const test_request&) = delete;

  test_request(test_request&&) = default;

  test_request& operator=(test_request&&) = default;

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Get HTTP method.
  ///
  /// @endverbatim
  auto method() const noexcept -> http::verb
  {
    return method_;
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Get HTTP version.
  ///
  /// @endverbatim
  auto version() const noexcept -> const http::version&
  {
    return version_;
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Get HTTP header.
  ///
  /// @endverbatim
  auto header() const noexcept -> const http::header&
  {
    return header_;
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Get query string.
  ///
  /// @endverbatim
  auto query() const noexcept -> const query_map&
  {
    return query_;
  }
  auto body() noexcept -> any_body&
  {
    return body_;
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Get body.
  ///
  /// @endverbatim
  auto body() const noexcept -> const any_body&
  {
    return body_;
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Create the builder for further modification.
  ///
  /// DESCRIPTION
  ///     Create the builder for further modification. Note that current object
  ///     is no longer usable after calling this function.
  ///
  /// @endverbatim
  auto builder() -> test_request_builder;

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Construct a ``test_request_builder`` with specific HTTP method.
  ///
  /// @endverbatim
  static auto get() -> test_request_builder;

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Construct a ``test_request_builder`` with specific HTTP method.
  ///
  /// @endverbatim
  static auto post() -> test_request_builder;

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Construct a ``test_request_builder`` with specific HTTP method.
  ///
  /// @endverbatim
  static auto put() -> test_request_builder;

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Construct a ``test_request_builder`` with specific HTTP method.
  ///
  /// @endverbatim
  static auto patch() -> test_request_builder;

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Construct a ``test_request_builder`` with specific HTTP method.
  ///
  /// @endverbatim
  static auto delete_() -> test_request_builder;

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Construct a ``test_request_builder`` with specific HTTP method.
  ///
  /// @endverbatim
  static auto head() -> test_request_builder;

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Construct a ``test_request_builder`` with specific HTTP method.
  ///
  /// @endverbatim
  static auto options() -> test_request_builder;
};

/// @verbatim embed:rst:leading-slashes
///
/// A type for constructing an instance of ``test_request``.
///
/// @endverbatim
class test_request_builder {
  friend class test_request;

  http::verb method_ = http::verb::unknown;
  http::version version_ = http::version::v1_1;
  http::header header_;
  query_map query_;
  any_body body_;

  test_request_builder(http::verb method,
                       http::version version,
                       http::header fields,
                       query_map query,
                       any_body body)
      : method_(method)
      , version_(version)
      , header_(std::move(fields))
      , query_(std::move(query))
      , body_(std::move(body))
  {
  }

public:
  explicit test_request_builder(http::verb method)
      : method_(method)
  {
  }

  test_request_builder(const test_request_builder&) = delete;

  test_request_builder& operator=(const test_request_builder&) = delete;

  test_request_builder(test_request_builder&&) = default;

  test_request_builder& operator=(test_request_builder&&) = default;

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Set HTTP method.
  ///
  /// @endverbatim
  auto set_method(http::verb method) & noexcept -> test_request_builder&
  {
    method_ = method;
    return *this;
  }

  auto set_method(http::verb method) && noexcept -> test_request_builder&&
  {
    method_ = method;
    return std::move(*this);
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Set HTTP version.
  ///
  /// @endverbatim
  auto set_version(http::version version) & noexcept -> test_request_builder&
  {
    version_ = version;
    return *this;
  }

  auto set_version(http::version version) && noexcept -> test_request_builder&&
  {
    set_version(version);
    return std::move(*this);
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Set HTTP header.
  ///
  /// DESCRIPTION
  ///    Set HTTP header. The input ``name`` will be canonicalized before
  ///    inserting it. Note that any existing header with the same name will be
  ///    removed before the insertion.
  ///
  /// @endverbatim
  auto set_header(std::string_view name,
                  std::string_view value) & -> test_request_builder&
  {
    header_.set(name, value);
    return *this;
  }
  auto set_header(std::string_view name,
                  std::string_view value) && -> test_request_builder&&
  {
    header_.set(name, value);
    return std::move(*this);
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Set HTTP header.
  ///
  /// DESCRIPTION
  ///    Set HTTP header. Note that any existing header with the same name will
  ///    be removed before the insertion.
  ///
  /// @endverbatim
  auto set_header(http::field name,
                  std::string_view value) & -> test_request_builder&
  {
    header_.set(name, value);
    return *this;
  }

  auto set_header(http::field name,
                  std::string_view value) && -> test_request_builder&&
  {
    header_.set(name, value);
    return std::move(*this);
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Insert HTTP header.
  ///
  /// DESCRIPTION
  ///    Insert HTTP header. The input ``name`` will be canonicalized before
  ///    inserting it. Note that any existing header with the same name
  ///    will be kept.
  ///
  /// @endverbatim
  auto insert_header(std::string_view name,
                     std::string_view value) & -> test_request_builder&
  {
    header_.insert(name, value);
    return *this;
  }

  auto insert_header(std::string_view name,
                     std::string_view value) && -> test_request_builder&&
  {
    header_.insert(name, value);
    return std::move(*this);
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Insert HTTP header.
  ///
  /// DESCRIPTION
  ///    Insert HTTP header. Note that any existing header with the same name
  ///    will be kept.
  ///
  /// @endverbatim
  auto insert_header(http::field name,
                     std::string_view value) & -> test_request_builder&
  {
    header_.insert(name, value);
    return *this;
  }

  auto insert_header(http::field name,
                     std::string_view value) && -> test_request_builder&&
  {
    header_.insert(name, value);
    return std::move(*this);
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Set query string.
  ///
  /// @endverbatim
  auto set_query(std::string name, std::string value) & -> test_request_builder&
  {
    query_.set(std::move(name), std::move(value));
    return *this;
  }

  auto set_query(std::string name,
                 std::string value) && -> test_request_builder&&
  {
    set_query(std::move(name), std::move(value));
    return std::move(*this);
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Set a null body and create the ``test_request``.
  ///
  /// DESCRIPTION
  ///    Set a null body and create the ``test_request``. If you do not want to
  ///    modify the existing body, call ``build()`` instead. Note that current
  ///    object is no longer usable after calling this function.
  ///
  /// @endverbatim
  auto set_body() -> test_request
  {
    body_ = any_body();
    return build();
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Set a raw body and create the ``test_request``.
  ///
  /// DESCRIPTION
  ///    Set a raw body and create the ``test_request``. Note that current
  ///    object is no longer usable after calling this function.
  ///
  /// @endverbatim
  template <std::size_t N>
  auto set_body(std::span<const std::byte, N> bytes) -> test_request
  {
    body_ = any_body(any_body::sized { bytes.size() },
                     async_readable_vector_stream(bytes));
    return build();
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Set a raw body and create the ``test_request``.
  ///
  /// DESCRIPTION
  ///    Set a raw body and create the ``test_request``. Note that current
  ///    object is no longer usable after calling this function.
  ///
  /// @endverbatim
  auto set_body(std::string_view sv) -> test_request
  {
    return set_body(std::as_bytes(std::span(sv.begin(), sv.end())));
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Set a raw body and create the ``test_request``.
  ///
  /// DESCRIPTION
  ///    Set a raw body and create the ``test_request``. Note that current
  ///    object is no longer usable after calling this function.
  ///
  /// @endverbatim
  template <async_readable_stream AsyncReadableStream>
  auto set_body(AsyncReadableStream&& stream) -> test_request
  {
    body_ = any_body(any_body::chunked(),
                     std::forward<AsyncReadableStream>(stream));
    return build();
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Set a json object as the body and create the ``test_request``.
  ///
  /// DESCRIPTION
  ///    Set a json object as the body and create the ``test_request``.
  ///    ``Content-Type: application/json`` will be automatically inserted. Note
  ///    that current object is no longer usable after calling this function.
  ///
  /// @endverbatim
  auto set_json(const boost::json::value& jv) -> test_request
  {
    auto str = boost::json::serialize(jv);
    set_header(http::field::content_type, mime::application_json());
    return set_body(std::as_bytes(std::span(str.begin(), str.end())));
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Set an object of type ``T`` that is converiable to a json object as the
  /// body and create the ``test_request``.
  ///
  /// DESCRIPTION
  ///    Set an object of type ``T`` that is converiable to a json object as the
  ///    the body and create the ``test_request``. ``Content-Type:
  ///    application/json`` will be automatically inserted. Note that current
  ///    object is no longer usable after calling this function.
  ///
  /// @endverbatim
  template <typename T>
    requires(boost::json::has_value_from<T>::value)
  auto set_json(const T& obj) -> test_request
  {
    return set_json(boost::json::value_from(obj));
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Set a stream body and create the ``test_request``.
  ///
  /// DESCRIPTION
  ///    Set a stream body and create the ``test_request``. A stream body will
  ///    be sent with ``Transfer-Encoding: chunked``. Note that current object
  ///    is no longer usable after calling this function.
  ///
  /// @endverbatim
  template <async_readable_stream AsyncReadableStream>
  auto set_stream_body(AsyncReadableStream&& stream) -> test_request
  {
    body_ = any_body(any_body::chunked(),
                     std::forward<AsyncReadableStream>(stream));
    return build();
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Do not modify the body and create the ``test_request``.
  ///
  /// DESCRIPTION
  ///     Do not modify the body and create the ``test_request``. If you want to
  ///     remove existing body, call `set_body()` instead. Note that current
  ///     object is no longer usable after calling this function.
  ///
  /// @endverbatim
  auto build() -> test_request
  {
    return {
      method_, version_, std::move(header_), std::move(query_), std::move(body_)
    };
  }
};

inline auto test_request::builder() -> test_request_builder
{
  return {
    method_, version_, std::move(header_), std::move(query_), std::move(body_)
  };
}

inline auto test_request::get() -> test_request_builder
{
  return test_request_builder(http::verb::get);
}

inline auto test_request::post() -> test_request_builder
{
  return test_request_builder(http::verb::post);
}

inline auto test_request::put() -> test_request_builder
{
  return test_request_builder(http::verb::put);
}

inline auto test_request::patch() -> test_request_builder
{
  return test_request_builder(http::verb::patch);
}

inline auto test_request::delete_() -> test_request_builder
{
  return test_request_builder(http::verb::delete_);
}

inline auto test_request::head() -> test_request_builder
{
  return test_request_builder(http::verb::head);
}

inline auto test_request::options() -> test_request_builder
{
  return test_request_builder(http::verb::options);
}

}

FITORIA_NAMESPACE_END

#endif
