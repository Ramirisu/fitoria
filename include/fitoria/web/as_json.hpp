//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef FITORIA_WEB_AS_JSON_HPP
#define FITORIA_WEB_AS_JSON_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/core/expected.hpp>
#include <fitoria/core/json.hpp>

#include <fitoria/web/error.hpp>

FITORIA_NAMESPACE_BEGIN

template <typename T = json::value>
expected<T, error_code> as_json(std::string_view text)
{
  json::error_code ec;
  auto jv = json::parse(text, ec);
  if (ec) {
    return unexpected { ec };
  }

  if constexpr (std::is_same_v<T, json::value>) {
    return jv;
  } else {
    return json::value_to<T>(jv);
  }
}

FITORIA_NAMESPACE_END

#endif
