//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#ifndef FITORIA_WEB_REQUEST_HPP
#define FITORIA_WEB_REQUEST_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/core/expected.hpp>
#include <fitoria/core/json.hpp>

#include <fitoria/http.hpp>
#include <fitoria/mime.hpp>

#include <fitoria/web/any_async_readable_stream.hpp>
#include <fitoria/web/async_readable_vector_stream.hpp>
#include <fitoria/web/connect_info.hpp>
#include <fitoria/web/detail/as_json.hpp>
#include <fitoria/web/path_info.hpp>
#include <fitoria/web/query_map.hpp>
#include <fitoria/web/state_storage.hpp>

FITORIA_NAMESPACE_BEGIN

namespace web {

class http_server;
class request_builder;

class request {
  friend class request_builder;
  friend class http_server;

  connect_info connection_;
  path_info path_;
  http::verb method_;
  http::version version_;
  http::header header_;
  query_map query_;
  any_async_readable_stream body_;
  state_storage states_;

  request(connect_info connection,
          path_info path,
          http::verb method,
          http::version version,
          http::header header,
          query_map query,
          any_async_readable_stream body,
          state_storage states)
      : connection_(std::move(connection))
      , path_(std::move(path))
      , method_(method)
      , version_(version)
      , header_(std::move(header))
      , query_(std::move(query))
      , body_(std::move(body))
      , states_(std::move(states))
  {
  }

public:
  request(const request&) = delete;

  request& operator=(const request&) = delete;

  request(request&&) = default;

  request& operator=(request&&) = default;

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Get connection information.
  ///
  /// @endverbatim
  auto connection() const noexcept -> const connect_info&
  {
    return connection_;
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Get path matching information
  ///
  /// @endverbatim
  auto path() const noexcept -> const path_info&
  {
    return path_;
  }

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
  /// Get query string.
  ///
  /// @endverbatim
  auto query() const noexcept -> const query_map&
  {
    return query_;
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
  /// Get body.
  ///
  /// @endverbatim
  auto body() noexcept -> any_async_readable_stream&
  {
    return body_;
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Get body.
  ///
  /// @endverbatim
  auto body() const noexcept -> const any_async_readable_stream&
  {
    return body_;
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Get associated states.
  ///
  /// @endverbatim
  template <typename T>
  auto state() const noexcept -> optional<T&>
  {
    return states_.state<T>();
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Create the builder for furthur modification.
  ///
  /// DESCRIPTION
  ///     Create the builder for furthur modification. Note that current object
  ///     is no longer usable after calling this function.
  ///
  /// @endverbatim
  auto builder() -> request_builder;
};

class request_builder {
  friend class request;

  connect_info connection_;
  path_info path_;
  http::verb method_;
  http::version version_;
  http::header header_;
  query_map query_;
  any_async_readable_stream body_;
  state_storage states_;

  request_builder(connect_info connection,
                  path_info path,
                  http::verb method,
                  http::version version,
                  http::header header,
                  query_map query,
                  any_async_readable_stream body,
                  state_storage states)
      : connection_(std::move(connection))
      , path_(std::move(path))
      , method_(method)
      , version_(version)
      , header_(std::move(header))
      , query_(std::move(query))
      , body_(std::move(body))
      , states_(std::move(states))
  {
  }

public:
  /// @verbatim embed:rst:leading-slashes
  ///
  /// Set HTTP method.
  ///
  /// @endverbatim
  auto set_method(http::verb method) & noexcept -> request_builder&
  {
    method_ = method;
    return *this;
  }

  auto set_method(http::verb method) && noexcept -> request_builder&&
  {
    method_ = method;
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
                  std::string_view value) & -> request_builder&
  {
    header_.set(name, value);
    return *this;
  }

  auto set_header(std::string_view name,
                  std::string_view value) && -> request_builder&&
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
                  std::string_view value) & -> request_builder&
  {
    header_.set(name, value);
    return *this;
  }

  auto set_header(http::field name,
                  std::string_view value) && -> request_builder&&
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
                     std::string_view value) & -> request_builder&
  {
    header_.insert(name, value);
    return *this;
  }

  auto insert_header(std::string_view name,
                     std::string_view value) && -> request_builder&&
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
                     std::string_view value) & -> request_builder&
  {
    header_.insert(name, value);
    return *this;
  }

  auto insert_header(http::field name,
                     std::string_view value) && -> request_builder&&
  {
    header_.insert(name, value);
    return std::move(*this);
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Get HTTP header map.
  ///
  /// @endverbatim
  auto header() noexcept -> http::header&
  {
    return header_;
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Set query string.
  ///
  /// @endverbatim
  auto set_query(std::string name, std::string value) & -> request_builder&
  {
    query_.set(std::move(name), std::move(value));
    return *this;
  };

  auto set_query(std::string name, std::string value) && -> request_builder&&
  {
    query_.set(std::move(name), std::move(value));
    return std::move(*this);
  };

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Get query string map.
  ///
  /// @endverbatim
  auto query() noexcept -> query_map&
  {
    return query_;
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Set a null body and create the ``response``.
  ///
  /// DESCRIPTION
  ///    Set a null body and create the ``response``. If you do not want to
  ///    modify the existing body, call ``build()`` instead. Note that current
  ///    object is no longer usable after calling this function.
  ///
  /// @endverbatim
  auto set_body() -> request
  {
    body_ = async_readable_vector_stream();
    return build();
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Set a raw body and create the ``request``.
  ///
  /// DESCRIPTION
  ///    Set a raw body and create the ``request``. Note that current object is
  ///    no longer usable after calling this function.
  ///
  /// @endverbatim
  template <std::size_t N>
  auto set_body(std::span<const std::byte, N> bytes) -> request
  {
    body_ = async_readable_vector_stream(bytes);
    return build();
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Set a raw body and create the ``request``.
  ///
  /// DESCRIPTION
  ///    Set a raw body and create the ``request``. Note that current object is
  ///    no longer usable after calling this function.
  ///
  /// @endverbatim
  auto set_body(std::string_view sv) -> request
  {
    set_body(std::as_bytes(std::span(sv.begin(), sv.end())));
    return build();
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Set a raw body and create the ``request``.
  ///
  /// DESCRIPTION
  ///    Set a raw body and create the ``request``. Note that current object is
  ///    no longer usable after calling this function.
  ///
  /// @endverbatim
  template <async_readable_stream AsyncReadableStream>
  auto set_body(AsyncReadableStream&& stream) -> request
  {
    body_ = std::forward<AsyncReadableStream>(stream);
    return build();
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Set a json object as the body and create the ``request``.
  ///
  /// DESCRIPTION
  ///    Set a json object as the body and create the ``request``.
  ///    ``Content-Type: application/json`` will be automatically inserted. Note
  ///    that current object is no longer usable after calling this function.
  ///
  /// @endverbatim
  auto set_json(const boost::json::value& jv) -> request
  {
    auto str = boost::json::serialize(jv);
    set_header(http::field::content_type, mime::application_json());
    set_body(std::as_bytes(std::span(str.begin(), str.end())));
    return build();
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Set an object of type ``T`` that is converiable to a json object as the
  /// body and create the ``request``.
  ///
  /// DESCRIPTION
  ///    Set an object of type ``T`` that is converiable to a json object as the
  ///    the body and create the ``request``. ``Content-Type: application/json``
  ///    will be automatically inserted. Note that current object is no longer
  ///    usable after calling this function.
  ///
  /// @endverbatim
  template <typename T>
    requires(boost::json::has_value_from<T>::value)
  auto set_json(const T& obj) -> request
  {
    return set_json(boost::json::value_from(obj));
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Do not modify the body and create the ``request``.
  ///
  /// DESCRIPTION
  ///     Do not modify the body and create the ``request``. If you want to
  ///     remove existing body, call `set_body()` instead. Note that current
  ///     object is no longer usable after calling this function.
  ///
  /// @endverbatim
  auto build() -> request
  {
    return { std::move(connection_),
             std::move(path_),
             method_,
             version_,
             std::move(header_),
             std::move(query_),
             std::move(body_),
             std::move(states_) };
  }
};

inline auto request::builder() -> request_builder
{
  return { std::move(connection_),
           std::move(path_),
           method_,
           version_,
           std::move(header_),
           std::move(query_),
           std::move(body_),
           std::move(states_) };
}

}

FITORIA_NAMESPACE_END

#endif
