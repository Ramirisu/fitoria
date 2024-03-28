//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#ifndef FITORIA_WEB_AS_FORM_HPP
#define FITORIA_WEB_AS_FORM_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/core/expected.hpp>
#include <fitoria/core/url.hpp>

#include <fitoria/web/error.hpp>
#include <fitoria/web/query_map.hpp>

FITORIA_NAMESPACE_BEGIN

namespace web {

inline expected<query_map, std::error_code> as_form(std::string_view text)
{
  auto res = boost::urls::parse_query(text);
  if (!res) {
    return unexpected { res.error() };
  }

  auto params = static_cast<boost::urls::params_view>(res.value());

  query_map map;
  for (auto it = params.begin(); it != params.end(); ++it) {
    auto kv = *it;
    if (kv.has_value) {
      map.set(kv.key, kv.value);
    }
  }

  return map;
}

}

FITORIA_NAMESPACE_END

#endif
