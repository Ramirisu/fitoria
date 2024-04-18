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

#include <fitoria/web/from_request.hpp>

#if defined(FITORIA_HAS_BOOST_PFR)
#include <boost/pfr.hpp>
#endif

FITORIA_NAMESPACE_BEGIN

namespace web {

#if defined(FITORIA_HAS_BOOST_PFR)

template <typename T>
class query_of {
public:
  explicit query_of(T inner)
      : inner_(std::move(inner))
  {
  }

  auto get() const noexcept -> const T&
  {
    return inner_;
  }

  friend auto tag_invoke(from_request_t<query_of<T>>, request& req)
      -> awaitable<expected<query_of<T>, std::error_code>>
  {
    co_return unpack_query(
        req.query(), std::make_index_sequence<boost::pfr::tuple_size_v<T>> {});
  }

private:
  template <std::size_t... Is>
  static auto unpack_query(const query_map& map, std::index_sequence<Is...>)
      -> expected<query_of<T>, std::error_code>
  {
    if (sizeof...(Is) == map.size()) {
      T result;
      if ((try_assign_field_index<Is>(map, result) && ...)) {
        return query_of<T>(std::move(result));
      }
    }

    return unexpected { make_error_code(error::path_extraction_error) };
  }

  template <std::size_t I>
  static bool try_assign_field_index(const query_map& map, T& result)
  {
    if (auto value = map.get(boost::pfr::get_name<I, T>()); value) {
      boost::pfr::get<I>(result) = *value;
      return true;
    }

    return false;
  }

  T inner_;
};

#else

template <typename T>
class query_of;

#endif

}

FITORIA_NAMESPACE_END

#endif
