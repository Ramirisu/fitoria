//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#ifndef FITORIA_WEB_DETAIL_AS_JSON_HPP
#define FITORIA_WEB_DETAIL_AS_JSON_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/core/expected.hpp>
#include <fitoria/core/json.hpp>

FITORIA_NAMESPACE_BEGIN

namespace web::detail {

template <typename T = boost::json::value>
expected<T, std::error_code> as_json(std::string_view text)
{
  boost::system::error_code ec;
  auto jv = boost::json::parse(text, ec);
  if (ec) {
    return unexpected { ec };
  }

  if constexpr (std::is_same_v<T, boost::json::value>) {
    return jv;
  } else {
    if (auto res = boost::json::try_value_to<T>(jv); res) {
      return res.value();
    } else {
      return unexpected { res.error() };
    }
  }
}

}

FITORIA_NAMESPACE_END

#endif
