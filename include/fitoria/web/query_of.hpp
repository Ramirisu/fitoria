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

#include <fitoria/web/error.hpp>
#include <fitoria/web/from_request.hpp>

#if defined(FITORIA_HAS_BOOST_PFR)
#include <boost/pfr.hpp>
#endif

FITORIA_NAMESPACE_BEGIN

namespace web {

#if defined(FITORIA_HAS_BOOST_PFR)

template <typename T>
class query_of : public T {
public:
  static_assert(not_cvref<T>, "T must not be cvref qualified");

  explicit query_of(T inner)
      : T(std::move(inner))
  {
  }

  friend auto tag_invoke(from_request_t<query_of<T>>, request& req)
      -> awaitable<expected<query_of<T>, std::error_code>>
  {
    if (req.query().size() != boost::pfr::tuple_size_v<T>) {
      co_return unexpected { make_error_code(
          error::extractor_field_count_not_match) };
    }

    co_return detail::extract_query<query_of<T>>::extract(
        req.query(), std::make_index_sequence<boost::pfr::tuple_size_v<T>> {});
  }
};

#else

template <typename T>
class query_of;

#endif

}

FITORIA_NAMESPACE_END

#endif
