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

#if defined(FITORIA_HAS_BOOST_PFR)
#include <boost/pfr.hpp>
#endif

FITORIA_NAMESPACE_BEGIN

namespace web {

#if defined(FITORIA_HAS_BOOST_PFR)

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
      } else if (mime->essence() != mime::application_www_form_urlencoded()) {
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

template <typename T>
class form_of;

#endif

}

FITORIA_NAMESPACE_END

#endif
