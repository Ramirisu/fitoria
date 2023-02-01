//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef FITORIA_WEB_HTTP_REQUEST_HPP
#define FITORIA_WEB_HTTP_REQUEST_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/core/expected.hpp>
#include <fitoria/core/json.hpp>

#include <fitoria/web/connection_info.hpp>
#include <fitoria/web/http.hpp>
#include <fitoria/web/http_fields.hpp>
#include <fitoria/web/query_map.hpp>
#include <fitoria/web/route_params.hpp>

FITORIA_NAMESPACE_BEGIN

class http_request {
public:
  http_request() = default;

  http_request(connection_info conn_info,
               route_params route_params,
               std::string path,
               http::verb method,
               query_map query,
               http_fields fields,
               std::string body)
      : conn_info_(std::move(conn_info))
      , route_params_(std::move(route_params))
      , path_(std::move(path))
      , method_(method)
      , query_(std::move(query))
      , fields_(std::move(fields))
      , body_(std::move(body))
  {
  }

  const connection_info& conn_info() const noexcept
  {
    return conn_info_;
  }

  const class route_params& route_params() const noexcept
  {
    return route_params_;
  }

  operator const class route_params &() const noexcept
  {
    return route_params_;
  }

  http::verb method() const noexcept
  {
    return method_;
  }

  http_request& set_method(http::verb method) noexcept
  {
    method_ = method;
    return *this;
  }

  const std::string& path() const noexcept
  {
    return path_;
  }

  query_map& query() noexcept
  {
    return query_;
  }

  const query_map& query() const noexcept
  {
    return query_;
  }

  operator query_map&() noexcept
  {
    return query_;
  }

  operator const query_map&() const noexcept
  {
    return query_;
  }

  http_request& set_query(std::string name, std::string value)
  {
    query_.set(std::move(name), std::move(value));
    return *this;
  };

  http_fields& fields() noexcept
  {
    return fields_;
  }

  const http_fields& fields() const noexcept
  {
    return fields_;
  }

  operator http_fields&() noexcept
  {
    return fields_;
  }

  operator const http_fields&() const noexcept
  {
    return fields_;
  }

  http_request& set_field(std::string name, std::string_view value)
  {
    fields_.set(std::move(name), value);
    return *this;
  }

  http_request& set_field(http::field name, std::string_view value)
  {
    fields_.set(name, value);
    return *this;
  }

  std::string& body() noexcept
  {
    return body_;
  }

  const std::string& body() const noexcept
  {
    return body_;
  }

  operator std::string&() noexcept
  {
    return body_;
  }

  operator const std::string&() const noexcept
  {
    return body_;
  }

  http_request& set_body(std::string body)
  {
    body_ = std::move(body);
    return *this;
  }

  template <typename T = json::value>
  http_request& set_json(const T& obj)
  {
    if constexpr (std::is_same_v<T, json::value>) {
      fields_.set(http::field::content_type,
                  http::fields::content_type::json());
      body_ = json::serialize(obj);
    } else {
      set_json(json::value_from(obj));
    }
    return *this;
  }

  http_request& prepare_payload()
  {
    if (!body().empty() || method_ == http::verb::options
        || method_ == http::verb::post || method_ == http::verb::put) {
      fields_.set(http::field::content_length, std::to_string(body().size()));
    }
    return *this;
  }

private:
  connection_info conn_info_;
  class route_params route_params_;
  std::string path_;
  http::verb method_;
  query_map query_;
  http_fields fields_;
  std::string body_;
};

FITORIA_NAMESPACE_END

#endif
