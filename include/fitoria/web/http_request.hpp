//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#ifndef FITORIA_WEB_HTTP_REQUEST_HPP
#define FITORIA_WEB_HTTP_REQUEST_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/core/expected.hpp>
#include <fitoria/core/json.hpp>

#include <fitoria/web/any_async_readable_stream.hpp>
#include <fitoria/web/async_readable_vector_stream.hpp>
#include <fitoria/web/connection_info.hpp>
#include <fitoria/web/detail/as_json.hpp>
#include <fitoria/web/http.hpp>
#include <fitoria/web/http_fields.hpp>
#include <fitoria/web/path_info.hpp>
#include <fitoria/web/query_map.hpp>
#include <fitoria/web/state_map.hpp>

FITORIA_NAMESPACE_BEGIN

namespace web {

class http_request {
public:
  http_request(http::verb method)
      : method_(method)
  {
  }

  http_request(connection_info conn_info,
               path_info path_info,
               http::verb method,
               query_map query,
               http_fields fields,
               any_async_readable_stream body,
               const std::vector<shared_state_map>& state_maps)
      : conn_info_(std::move(conn_info))
      , path_info_(std::move(path_info))
      , method_(method)
      , query_(std::move(query))
      , fields_(std::move(fields))
      , body_(std::move(body))
      , state_maps_(state_maps)
  {
  }

  const connection_info& conn_info() const noexcept
  {
    return conn_info_;
  }

  path_info& path() noexcept
  {
    return path_info_;
  }

  const path_info& path() const noexcept
  {
    return path_info_;
  }

  http::verb method() const noexcept
  {
    return method_;
  }

  http_request& set_method(http::verb method) & noexcept
  {
    method_ = method;
    return *this;
  }

  http_request&& set_method(http::verb method) && noexcept
  {
    method_ = method;
    return std::move(*this);
  }

  query_map& query() noexcept
  {
    return query_;
  }

  const query_map& query() const noexcept
  {
    return query_;
  }

  http_request& set_query(std::string name, std::string value) &
  {
    query_.set(std::move(name), std::move(value));
    return *this;
  };

  http_request&& set_query(std::string name, std::string value) &&
  {
    query_.set(std::move(name), std::move(value));
    return std::move(*this);
  };

  http_fields& fields() noexcept
  {
    return fields_;
  }

  const http_fields& fields() const noexcept
  {
    return fields_;
  }

  http_request& set_field(std::string name, std::string_view value) &
  {
    fields_.set(std::move(name), value);
    return *this;
  }

  http_request&& set_field(std::string name, std::string_view value) &&
  {
    fields_.set(std::move(name), value);
    return std::move(*this);
  }

  http_request& set_field(http::field name, std::string_view value) &
  {
    fields_.set(name, value);
    return *this;
  }

  http_request&& set_field(http::field name, std::string_view value) &&
  {
    fields_.set(name, value);
    return std::move(*this);
  }

  http_request& insert_field(std::string name, std::string_view value) &
  {
    fields_.insert(std::move(name), value);
    return *this;
  }

  http_request&& insert_field(std::string name, std::string_view value) &&
  {
    fields_.insert(std::move(name), value);
    return std::move(*this);
  }

  http_request& insert_field(http::field name, std::string_view value) &
  {
    fields_.insert(name, value);
    return *this;
  }

  http_request&& insert_field(http::field name, std::string_view value) &&
  {
    fields_.insert(name, value);
    return std::move(*this);
  }

  any_async_readable_stream& body() noexcept
  {
    return body_;
  }

  const any_async_readable_stream& body() const noexcept
  {
    return body_;
  }

  net::awaitable<expected<std::string, net::error_code>> as_string()
  {
    if (auto str = co_await async_read_all_as<std::string>(body()); str) {
      co_return *str;
    }

    co_return std::string();
  }

  template <typename T = boost::json::value>
  net::awaitable<expected<T, net::error_code>> as_json()
  {
    if (fields().get(http::field::content_type)
        != http::fields::content_type::json()) {
      co_return unexpected { make_error_code(error::unexpected_content_type) };
    }

    if (auto str = co_await async_read_all_as<std::string>(body()); str) {
      if (*str) {
        co_return detail::as_json<T>(**str);
      }

      co_return unexpected { (*str).error() };
    }

    co_return detail::as_json<T>("");
  }

  template <std::size_t N>
  http_request& set_body(std::span<const std::byte, N> bytes) &
  {
    body_ = any_async_readable_stream(async_readable_vector_stream(bytes));
    return *this;
  }

  template <std::size_t N>
  http_request&& set_body(std::span<const std::byte, N> bytes) &&
  {
    body_ = any_async_readable_stream(async_readable_vector_stream(bytes));
    return std::move(*this);
  }

  http_request& set_body(std::string_view sv) &
  {
    set_body(std::as_bytes(std::span(sv.begin(), sv.end())));
    return *this;
  }

  http_request&& set_body(std::string_view sv) &&
  {
    set_body(std::as_bytes(std::span(sv.begin(), sv.end())));
    return std::move(*this);
  }

  template <typename T = boost::json::value>
  http_request& set_json(const T& obj) &
  {
    if constexpr (std::is_same_v<T, boost::json::value>) {
      set_field(http::field::content_type, http::fields::content_type::json());
      auto s = boost::json::serialize(obj);
      set_body(std::as_bytes(std::span(s.begin(), s.end())));
    } else {
      set_json(boost::json::value_from(obj));
    }
    return *this;
  }

  template <typename T = boost::json::value>
  http_request&& set_json(const T& obj) &&
  {
    if constexpr (std::is_same_v<T, boost::json::value>) {
      set_field(http::field::content_type, http::fields::content_type::json());
      auto s = boost::json::serialize(obj);
      set_body(std::as_bytes(std::span(s.begin(), s.end())));
    } else {
      set_json(boost::json::value_from(obj));
    }
    return std::move(*this);
  }

  template <async_readable_stream AsyncReadableStream>
  http_request& set_stream(AsyncReadableStream&& stream) &
  {
    body_ = std::forward<AsyncReadableStream>(stream);
    return *this;
  }

  template <async_readable_stream AsyncReadableStream>
  http_request&& set_stream(AsyncReadableStream&& stream) &&
  {
    body_ = std::forward<AsyncReadableStream>(stream);
    return std::move(*this);
  }

  template <typename T>
  optional<T&> state() const
  {
    static_assert(std::same_as<T, std::remove_cvref_t<T>>,
                  "T must not be cvref qualified");
    if (state_maps_) {
      for (auto& state : *state_maps_) {
        if (auto it = state->find(std::type_index(typeid(T)));
            it != state->end()) {
          try {
            return std::any_cast<T&>(it->second);
          } catch (const std::bad_any_cast&) {
          }
        }
      }
    }

    return nullopt;
  }

private:
  connection_info conn_info_;
  path_info path_info_;
  http::verb method_;
  query_map query_;
  http_fields fields_;
  any_async_readable_stream body_ { async_readable_vector_stream::eof() };
  optional<const std::vector<shared_state_map>&> state_maps_;
};

}

FITORIA_NAMESPACE_END

#endif
