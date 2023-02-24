//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef FITORIA_CLIENT_HTTP_RESPONSE_HPP
#define FITORIA_CLIENT_HTTP_RESPONSE_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/web/http.hpp>
#include <fitoria/web/http_fields.hpp>

FITORIA_NAMESPACE_BEGIN

namespace client {

namespace http = web::http;

using web::http_fields;

class http_response {
  using native_response_t
      = boost::beast::http::response<boost::beast::http::string_body>;

public:
  http_response() = default;

  explicit http_response(native_response_t native)
      : version_(static_cast<http::version>(native.version()))
      , status_code_(native.result())
  {
    for (auto& field : native) {
      fields_.insert(field.name(), field.value());
    }
    body_ = std::move(native.body());
  }

  http::version version() const noexcept
  {
    return version_;
  }

  http::status_code status_code() const noexcept
  {
    return status_code_;
  }

  http_fields& fields() noexcept
  {
    return fields_;
  }

  const http_fields& fields() const noexcept
  {
    return fields_;
  }

  std::string& body() noexcept
  {
    return body_;
  }

  const std::string& body() const noexcept
  {
    return body_;
  }

private:
  http::version version_ = http::version::unknown;
  http::status_code status_code_ = http::status::ok;
  http_fields fields_;
  std::string body_;
};

}

FITORIA_NAMESPACE_END

template <typename CharT>
struct FITORIA_NAMESPACE::fmt::
    formatter<FITORIA_NAMESPACE::client::http_response, CharT> {
  auto parse(FITORIA_NAMESPACE::fmt::format_parse_context& ctx)
  {
    auto it = ctx.begin();
    while (it != ctx.end() && *it != '}') {
      ++it;
    }
    if (it != ctx.end()) {
      ++it;
    }

    return it;
  }

  template <typename FormatContext>
  auto format(FITORIA_NAMESPACE::client::http_response res,
              FormatContext& ctx) const
  {
    FITORIA_NAMESPACE::fmt::format_to(
        ctx.out(),
        "HTTP/{} {}\n",
        to_string(res.version()),
        std::string_view(obsolete_reason(res.status_code().value())));
    for (auto& [name, value] : res.fields()) {
      FITORIA_NAMESPACE::fmt::format_to(ctx.out(), "{}: {}\n", name, value);
    }
    FITORIA_NAMESPACE::fmt::format_to(ctx.out(), "\n");
    return FITORIA_NAMESPACE::fmt::format_to(ctx.out(), "{}", res.body());
  }
};

#endif
