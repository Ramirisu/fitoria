//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef FITORIA_WEB_EXTRACTOR_JSON_HPP
#define FITORIA_WEB_EXTRACTOR_JSON_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/core/expected.hpp>
#include <fitoria/core/json.hpp>

#include <fitoria/web/error.hpp>

FITORIA_NAMESPACE_BEGIN

namespace web::extractor {
namespace detail {

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
      if (auto res = json::try_value_to<T>(jv); res) {
        return res.value();
      } else {
        return unexpected { res.error() };
      }
    }
  }
}

template <typename T>
class json : public T {
public:
  using fitoria_extractor_json_base_type = T;

  json(T value)
      : T(std::move(value))
  {
  }
};

}

FITORIA_NAMESPACE_END

#endif
