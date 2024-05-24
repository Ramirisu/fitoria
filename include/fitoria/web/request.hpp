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

#include <fitoria/web/any_async_readable_stream.hpp>
#include <fitoria/web/async_readable_vector_stream.hpp>
#include <fitoria/web/connect_info.hpp>
#include <fitoria/web/detail/as_json.hpp>
#include <fitoria/web/path_info.hpp>
#include <fitoria/web/query_map.hpp>
#include <fitoria/web/state_storage.hpp>

FITORIA_NAMESPACE_BEGIN

namespace web {

class request {
public:
  request(http::verb method)
      : method_(method)
  {
  }

  request(connect_info conn_info,
          path_info path_info,
          http::verb method,
          http::version version,
          query_map query,
          http::header header,
          any_async_readable_stream body,
          state_storage states)
      : conn_info_(std::move(conn_info))
      , path_info_(std::move(path_info))
      , method_(method)
      , version_(version)
      , query_(std::move(query))
      , header_(std::move(header))
      , body_(std::move(body))
      , states_(std::move(states))
  {
  }

  auto connection() const noexcept -> const connect_info&
  {
    return conn_info_;
  }

  auto path() noexcept -> path_info&
  {
    return path_info_;
  }

  auto path() const noexcept -> const path_info&
  {
    return path_info_;
  }

  auto method() const noexcept -> http::verb
  {
    return method_;
  }

  auto set_method(http::verb method) & noexcept -> request&
  {
    method_ = method;
    return *this;
  }

  auto set_method(http::verb method) && noexcept -> request&&
  {
    method_ = method;
    return std::move(*this);
  }

  auto version() const noexcept -> const http::version&
  {
    return version_;
  }

  auto query() noexcept -> query_map&
  {
    return query_;
  }

  auto query() const noexcept -> const query_map&
  {
    return query_;
  }

  auto set_query(std::string name, std::string value) & -> request&
  {
    query_.set(std::move(name), std::move(value));
    return *this;
  };

  auto set_query(std::string name, std::string value) && -> request&&
  {
    query_.set(std::move(name), std::move(value));
    return std::move(*this);
  };

  auto header() noexcept -> http::header&
  {
    return header_;
  }

  auto header() const noexcept -> const http::header&
  {
    return header_;
  }

  auto set_header(std::string name, std::string_view value) & -> request&
  {
    header_.set(std::move(name), value);
    return *this;
  }

  auto set_header(std::string name, std::string_view value) && -> request&&
  {
    header_.set(std::move(name), value);
    return std::move(*this);
  }

  auto set_header(http::field name, std::string_view value) & -> request&
  {
    header_.set(name, value);
    return *this;
  }

  auto set_header(http::field name, std::string_view value) && -> request&&
  {
    header_.set(name, value);
    return std::move(*this);
  }

  auto insert_header(std::string name, std::string_view value) & -> request&
  {
    header_.insert(std::move(name), value);
    return *this;
  }

  auto insert_header(std::string name, std::string_view value) && -> request&&
  {
    header_.insert(std::move(name), value);
    return std::move(*this);
  }

  auto insert_header(http::field name, std::string_view value) & -> request&
  {
    header_.insert(name, value);
    return *this;
  }

  auto insert_header(http::field name, std::string_view value) && -> request&&
  {
    header_.insert(name, value);
    return std::move(*this);
  }

  auto body() noexcept -> any_async_readable_stream&
  {
    return body_;
  }

  auto body() const noexcept -> const any_async_readable_stream&
  {
    return body_;
  }

  template <std::size_t N>
  auto set_body(std::span<const std::byte, N> bytes) & -> request&
  {
    body_ = async_readable_vector_stream(bytes);
    return *this;
  }

  template <std::size_t N>
  auto set_body(std::span<const std::byte, N> bytes) && -> request&&
  {
    set_body(bytes);
    return std::move(*this);
  }

  auto set_body(std::string_view sv) & -> request&
  {
    set_body(std::as_bytes(std::span(sv.begin(), sv.end())));
    return *this;
  }

  auto set_body(std::string_view sv) && -> request&&
  {
    set_body(std::as_bytes(std::span(sv.begin(), sv.end())));
    return std::move(*this);
  }

  auto set_json(const boost::json::value& jv) & -> request&
  {
    set_header(http::field::content_type, http::fields::content_type::json());
    auto str = boost::json::serialize(jv);
    set_body(std::as_bytes(std::span(str.begin(), str.end())));
    return *this;
  }

  auto set_json(const boost::json::value& jv) && -> request&&
  {
    set_json(jv);
    return std::move(*this);
  }

  template <typename T>
    requires(boost::json::has_value_from<T>::value)
  auto set_json(const T& obj) & -> request&
  {
    return set_json(boost::json::value_from(obj));
  }

  template <typename T>
    requires(boost::json::has_value_from<T>::value)
  auto set_json(const T& obj) && -> request&&
  {
    set_json(boost::json::value_from(obj));
    return std::move(*this);
  }

  template <async_readable_stream AsyncReadableStream>
  auto set_stream(AsyncReadableStream&& stream) & -> request&
  {
    body_ = std::forward<AsyncReadableStream>(stream);
    return *this;
  }

  template <async_readable_stream AsyncReadableStream>
  auto set_stream(AsyncReadableStream&& stream) && -> request&&
  {
    set_stream(std::forward<AsyncReadableStream>(stream));
    return std::move(*this);
  }

  template <typename T>
  auto state() const -> optional<T&>
  {
    return states_.state<T>();
  }

private:
  connect_info conn_info_;
  path_info path_info_;
  http::verb method_;
  http::version version_;
  query_map query_;
  http::header header_;
  any_async_readable_stream body_ = async_readable_vector_stream();
  state_storage states_;
};

}

FITORIA_NAMESPACE_END

#endif