//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#ifndef FITORIA_WEB_DETAIL_EXTRACT_QUERY_HPP
#define FITORIA_WEB_DETAIL_EXTRACT_QUERY_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/core/expected.hpp>
#include <fitoria/core/from_string.hpp>

#include <fitoria/web/error.hpp>

#if defined(FITORIA_HAS_BOOST_PFR)
#include <boost/pfr.hpp>
#endif

FITORIA_NAMESPACE_BEGIN

namespace web::detail {

template <typename Extractor>
class extract_query;

#if defined(FITORIA_HAS_BOOST_PFR)

template <template <typename...> class Extractor, typename T>
class extract_query<Extractor<T>> {
public:
  template <typename Map, std::size_t... Is>
  static auto extract(const Map& map, std::index_sequence<Is...>)
      -> expected<Extractor<T>, std::error_code>
  {
    auto result = T();
    auto ec = std::error_code();

    if ((try_assign_field_index<Map, Is>(map, result, ec) && ...)) {
      return Extractor<T>(std::move(result));
    }

    return unexpected { ec };
  }

private:
  template <typename Map, std::size_t I>
  static bool
  try_assign_field_index(const Map& map, T& result, std::error_code& ec)
  {
    auto value = map.get(boost::pfr::get_name<I, T>());
    if (!value) {
      ec = make_error_code(error::extractor_field_name_not_found);
      return false;
    }

    auto str = from_string<std::decay_t<decltype(boost::pfr::get<I>(result))>>(
        *value);
    if (!str) {
      ec = str.error();
      return false;
    }

    boost::pfr::get<I>(result) = std::move(*str);
    return true;
  }
};

#endif

}

FITORIA_NAMESPACE_END

#endif
