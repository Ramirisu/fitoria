
//
// Copyright (c) 2022 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include <fitoria/core/config.hpp>

#include <fitoria/core/error.hpp>
#include <fitoria/core/expected.hpp>
#include <fitoria/core/http.hpp>
#include <fitoria/core/url.hpp>

#include <fitoria/http_server/http_request.hpp>
#include <fitoria/http_server/query_map.hpp>

FITORIA_NAMESPACE_BEGIN

class from_post_form : public expected<query_map, error_code> {
public:
  from_post_form(const http_request& req)
      : expected<query_map, error_code>(parse(req))
  {
  }

private:
  static expected<query_map, error_code> parse(const http_request& req)
  {
    auto res = urls::parse_query(req.body());
    if (!res) {
      return unexpected<error_code>(res.error());
    }

    auto params = static_cast<urls::params_view>(res.value());

    query_map map;
    for (auto it = params.begin(); it != params.end(); ++it) {
      auto kv = *it;
      if (kv.has_value) {
        map.set(kv.key, kv.value);
      }
    }

    return map;
  }
};

FITORIA_NAMESPACE_END
