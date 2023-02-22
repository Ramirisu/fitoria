//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef FITORIA_WEB_MOCK_HTTP_REQUEST_HPP
#define FITORIA_WEB_MOCK_HTTP_REQUEST_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/core/expected.hpp>
#include <fitoria/core/json.hpp>

#include <fitoria/web/connection_info.hpp>
#include <fitoria/web/http.hpp>
#include <fitoria/web/http_fields.hpp>
#include <fitoria/web/query_map.hpp>
#include <fitoria/web/route_params.hpp>
#include <fitoria/web/state_map.hpp>

FITORIA_NAMESPACE_BEGIN

namespace web {

class mock_http_request {
public:
  mock_http_request(http::verb method)
      : method_(method)
  {
  }

  http::verb method() const noexcept
  {
    return method_;
  }

  mock_http_request& set_method(http::verb method) noexcept
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

  mock_http_request& set_query(std::string name, std::string value)
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

  mock_http_request& set_field(std::string name, std::string_view value)
  {
    fields_.set(std::move(name), value);
    return *this;
  }

  mock_http_request& set_field(http::field name, std::string_view value)
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

  mock_http_request& set_body(std::string body)
  {
    body_ = std::move(body);
    return *this;
  }

  template <typename T = boost::json::value>
  mock_http_request& set_json(const T& obj)
  {
    if constexpr (std::is_same_v<T, boost::json::value>) {
      fields_.set(http::field::content_type,
                  http::fields::content_type::json());
      body_ = boost::json::serialize(obj);
    } else {
      set_json(boost::json::value_from(obj));
    }
    return *this;
  }

  mock_http_request& prepare_payload()
  {
    if (!body().empty() || method_ == http::verb::options
        || method_ == http::verb::post || method_ == http::verb::put) {
      fields_.set(http::field::content_length, std::to_string(body().size()));
    }
    return *this;
  }

private:
  std::string path_;
  http::verb method_;
  query_map query_;
  http_fields fields_;
  std::string body_;
};

}

FITORIA_NAMESPACE_END

#endif
