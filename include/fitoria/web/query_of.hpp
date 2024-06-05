//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#ifndef FITORIA_WEB_QUERY_OF_HPP
#define FITORIA_WEB_QUERY_OF_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/web/detail/extract_query.hpp>

#include <fitoria/web/from_request.hpp>

#include <boost/pfr.hpp>

FITORIA_NAMESPACE_BEGIN

namespace web {

template <typename T>
class query_of : public T {
public:
  static_assert(not_cvref<T>, "T must not be cvref qualified");

  explicit query_of(T inner)
      : T(std::move(inner))
  {
  }

  friend auto tag_invoke(from_request_t<query_of<T>>, request& req)
      -> awaitable<expected<query_of<T>, response>>
  {
    if (req.query().size() != boost::pfr::tuple_size_v<T>) {
      co_return unexpected {
        response::bad_request()
            .set_header(http::field::content_type, mime::text_plain())
            .set_body("numbers of key/value pairs are not expected.")
      };
    }

    if (auto result = detail::extract_query<query_of<T>>::extract(
            req.query(),
            std::make_index_sequence<boost::pfr::tuple_size_v<T>> {});
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

}

FITORIA_NAMESPACE_END

#endif
