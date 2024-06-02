//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#ifndef FITORIA_WEB_JSON_OF_HPP
#define FITORIA_WEB_JSON_OF_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/core/expected.hpp>

#include <fitoria/web/detail/as_json.hpp>
#include <fitoria/web/error.hpp>
#include <fitoria/web/from_request.hpp>

FITORIA_NAMESPACE_BEGIN

namespace web {

template <typename T>
class json_of : public T {
public:
  static_assert(not_cvref<T>, "T must not be cvref qualified");

  json_of(T value)
      : T(std::move(value))
  {
  }

  friend auto tag_invoke(from_request_t<json_of<T>>, request& req)
      -> awaitable<expected<json_of<T>, response>>
  {
    if (auto ct = req.header().get(http::field::content_type); !ct) {
      co_return unexpected { response::bad_request()
                                 .set_header(http::field::content_type,
                                             mime::text_plain())
                                 .set_body("\"Content-Type\" is expected.") };
    } else {
      if (auto mime = mime::mime_view::parse(*ct); !mime) {
        co_return unexpected {
          response::bad_request()
              .set_header(http::field::content_type, mime::text_plain())
              .set_body("invalid MIME format for \"Content-Type\".")
        };
      } else if (mime->essence() != mime::application_json()) {
        co_return unexpected {
          response::bad_request()
              .set_header(http::field::content_type, mime::text_plain())
              .set_body("\"Content-Type\" is not matched.")
        };
      }
    }

    auto body = co_await from_request<std::string>(req);
    if (!body) {
      co_return unexpected { body.error() };
    }

    if (auto form = detail::as_json<T>(*body); form) {
      co_return std::move(*form);
    } else {
      co_return unexpected {
        response::bad_request()
            .set_header(http::field::content_type, mime::text_plain())
            .set_body("invalid format for request body.")
      };
    }
  }
};

}

FITORIA_NAMESPACE_END

#endif
