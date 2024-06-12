//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#ifndef FITORIA_WEB_FORM_OF_HPP
#define FITORIA_WEB_FORM_OF_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/web/detail/as_form.hpp>
#include <fitoria/web/detail/extract_query.hpp>

#include <fitoria/web/error.hpp>
#include <fitoria/web/from_request.hpp>

#include <boost/pfr.hpp>

FITORIA_NAMESPACE_BEGIN

namespace web {

#if !defined(FITORIA_DOC)

template <typename T>
class form_of : public T {
public:
  static_assert(not_cvref<T>, "T must not be cvref qualified");

  explicit form_of(T inner)
      : T(std::move(inner))
  {
  }

  friend auto tag_invoke(from_request_t<form_of<T>>, request& req)
      -> awaitable<expected<form_of<T>, response>>
  {
    if (auto mime = req.headers()
                        .get(http::field::content_type)
                        .and_then(mime::mime_view::parse);
        !mime || mime->essence() != mime::application_www_form_urlencoded()) {
      co_return unexpected {
        response::bad_request()
            .set_header(http::field::content_type, mime::text_plain())
            .set_body(
                fmt::format(R"("Content-Type: {}" is expected.)",
                            mime::application_www_form_urlencoded().source()))
      };
    }

    auto body = co_await from_request<std::string>(req);
    if (!body) {
      co_return unexpected { body.error() };
    }

    auto form = detail::as_form(*body);
    if (!form) {
      co_return unexpected {
        response::bad_request()
            .set_header(http::field::content_type, mime::text_plain())
            .set_body("invalid format for request body.")
      };
    }

    if (form->size() != boost::pfr::tuple_size_v<T>) {
      co_return unexpected {
        response::bad_request()
            .set_header(http::field::content_type, mime::text_plain())
            .set_body("numbers of key/value pairs are not expected.")
      };
    }

    if (auto result = detail::extract_query<form_of<T>>::extract(
            *form, std::make_index_sequence<boost::pfr::tuple_size_v<T>> {});
        result) {
      co_return std::move(*result);
    } else {
      co_return unexpected {
        response::bad_request()
            .set_header(http::field::content_type, mime::text_plain())
            .set_body("keys are not matched or value types are invalid.")
      };
    }
  }
};

#else

/// @verbatim embed:rst:leading-slashes
///
/// Extractor for parsing form data into type ``T``.
///
/// @endverbatim
template <typename T>
class form_of;

#endif

}

FITORIA_NAMESPACE_END

#endif
