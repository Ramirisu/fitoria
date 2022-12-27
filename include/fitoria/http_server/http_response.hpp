//
// Copyright (c) 2022 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include <fitoria/core/config.hpp>

#include <fitoria/core/http.hpp>
#include <fitoria/core/json.hpp>

#include <fitoria/http_server/http_header.hpp>

#include <string>

FITORIA_NAMESPACE_BEGIN

class http_response {
  friend class http_server;

public:
  http_response(status status)
      : status_(status)
  {
  }

  http_header& headers() noexcept
  {
    return header_;
  }

  const http_header& headers() const noexcept
  {
    return header_;
  }

  http_response& set_header(std::string name, std::string value)
  {
    header_.set(std::move(name), std::move(value));
    return *this;
  }

  http_response& set_header(field name, std::string value)
  {
    header_.set(name, std::move(value));
    return *this;
  }

  http_response& set_body(std::string body)
  {
    body_ = std::move(body);
    return *this;
  }

  http_response& set_json(const json::value& jv)
  {
    header_.set(field::content_type, "application/json");
    body_ = json::serialize(jv);
    return *this;
  }

private:
  status status_;
  http_header header_;
  std::string body_;
};

FITORIA_NAMESPACE_END
