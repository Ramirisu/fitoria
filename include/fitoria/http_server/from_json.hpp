
//
// Copyright (c) 2022 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include <fitoria/core/config.hpp>

#include <fitoria/core/json.hpp>
#include <fitoria/core/utility.hpp>

#include <fitoria/http_server/http_request.hpp>

FITORIA_NAMESPACE_BEGIN

template <typename T>
class from_json : public expected<T, net::error_code> {
public:
  from_json(const http_request& c)
      : expected<T, net::error_code>(parse(c.body()))
  {
  }

private:
  static expected<T, net::error_code> parse(const std::string& s)
  {
    net::error_code ec;
    auto jv = json::parse(s, ec);
    if (ec) {
      return unexpected<net::error_code>(ec);
    }
    return json::value_to<T>(jv);
  }
};

FITORIA_NAMESPACE_END
