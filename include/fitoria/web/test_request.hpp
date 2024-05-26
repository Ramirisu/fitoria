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

#include <fitoria/web/any_async_readable_stream.hpp>
#include <fitoria/web/any_body.hpp>
#include <fitoria/web/async_readable_vector_stream.hpp>
#include <fitoria/web/query_map.hpp>

#include <span>

FITORIA_NAMESPACE_BEGIN

namespace web {

class test_request_builder;

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

  auto method() const noexcept -> http::verb
  {
    return method_;
  }

  auto version() const noexcept -> http::version
  {
    return version_;
  }

  auto header() noexcept -> http::header&
  {
    return header_;
  }

  auto header() const noexcept -> const http::header&
  {
    return header_;
  }

  auto query() noexcept -> query_map&
  {
    return query_;
  }

  auto query() const noexcept -> const query_map&
  {
    return query_;
  }

  auto body() noexcept -> any_body&
  {
    return body_;
  }

  auto body() const noexcept -> const any_body&
  {
    return body_;
  }

  auto builder() -> test_request_builder;

  static auto get() -> test_request_builder;

  static auto post() -> test_request_builder;

  static auto put() -> test_request_builder;

  static auto patch() -> test_request_builder;

  static auto delete_() -> test_request_builder;

  static auto head() -> test_request_builder;

  static auto options() -> test_request_builder;
};

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

  auto build() -> test_request
  {
    return {
      method_, version_, std::move(header_), std::move(query_), std::move(body_)
    };
  }

  template <std::size_t N>
  auto set_body(std::span<const std::byte, N> bytes) -> test_request
  {
    body_ = any_body(any_body::sized { bytes.size() },
                     async_readable_vector_stream(bytes));
    return build();
  }

  auto set_body(std::string_view sv) -> test_request
  {
    // TODO: plaintext?
    return set_body(std::as_bytes(std::span(sv.begin(), sv.end())));
  }

  auto set_json(const boost::json::value& jv) -> test_request
  {
    set_header(http::field::content_type, http::fields::content_type::json());
    auto str = boost::json::serialize(jv);
    return set_body(std::as_bytes(std::span(str.begin(), str.end())));
  }

  template <typename T>
    requires(boost::json::has_value_from<T>::value)
  auto set_json(const T& obj) -> test_request
  {
    return set_json(boost::json::value_from(obj));
  }

  template <async_readable_stream AsyncReadableStream>
  auto set_stream(AsyncReadableStream&& stream) -> test_request
  {
    body_ = any_body(any_body::chunked(),
                     std::forward<AsyncReadableStream>(stream));
    return build();
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
