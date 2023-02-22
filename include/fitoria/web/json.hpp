//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef FITORIA_WEB_JSON_HPP
#define FITORIA_WEB_JSON_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/core/expected.hpp>
#include <fitoria/core/json.hpp>

#include <fitoria/web/error.hpp>
#include <fitoria/web/from_http_request.hpp>

FITORIA_NAMESPACE_BEGIN

namespace web {

template <typename T = boost::json::value>
expected<T, error_code> as_json(std::string_view text)
{
  boost::json::error_code ec;
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

template <typename T>
class json : public T {
public:
  using fitoria_extractor_json_base_type = T;

  json(T value)
      : T(std::move(value))
  {
  }

  friend auto tag_invoke(from_http_request_t<json<T>>, http_request& req)
      -> lazy<expected<json<T>, error_code>>
  {
    if (req.fields().get(http::field::content_type)
        != http::fields::content_type::json()) {
      co_return unexpected { make_error_code(error::unexpected_content_type) };
    }

    auto str = co_await tag_invoke(from_http_request_t<std::string> {}, req);
    if (!str) {
      co_return unexpected { str.error() };
    }

    co_return as_json<T>(*str);
  }
};

}

FITORIA_NAMESPACE_END

#endif
