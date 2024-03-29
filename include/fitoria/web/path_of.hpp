//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#ifndef FITORIA_WEB_PATH_OF_HPP
#define FITORIA_WEB_PATH_OF_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/web/from_http_request.hpp>

#if defined(FITORIA_HAS_BOOST_PFR)
#include <boost/pfr.hpp>
#endif

#include <tuple>

FITORIA_NAMESPACE_BEGIN

namespace web {

#if defined(FITORIA_HAS_BOOST_PFR)

template <typename T>
class path_of : public T {
public:
  static_assert(std::same_as<T, std::remove_cvref_t<T>>,
                "T must not be cvref qualified");

  explicit path_of(T inner)
      : T(std::move(inner))
  {
  }

  friend auto tag_invoke(from_http_request_t<path_of<T>>, http_request& req)
      -> net::awaitable<expected<path_of<T>, std::error_code>>
  {
    co_return unpack_path(
        req.path(), std::make_index_sequence<boost::pfr::tuple_size_v<T>> {});
  }

private:
  template <std::size_t... Is>
  static auto unpack_path(const path_info& path_info,
                          std::index_sequence<Is...>)
      -> expected<path_of<T>, std::error_code>
  {
    if (sizeof...(Is) == path_info.size()) {
      T result;
      if ((try_assign_field_index<Is>(path_info, result) && ...)) {
        return path_of<T>(std::move(result));
      }
    }

    return unexpected { make_error_code(error::path_extraction_error) };
  }

  template <std::size_t I>
  static bool try_assign_field_index(const path_info& path_info, T& result)
  {
    if (auto value = path_info.get(boost::pfr::get_name<I, T>()); value) {
      boost::pfr::get<I>(result) = *value;
      return true;
    }

    return false;
  }
};

#else

template <typename T>
class path_of;

#endif

template <typename... Ts>
class path_of<std::tuple<Ts...>> : public std::tuple<Ts...> {
public:
  static_assert((std::same_as<Ts, std::remove_cvref_t<Ts>> && ...),
                "Ts... must not be cvref qualified");

  explicit path_of(std::tuple<Ts...> inner)
      : std::tuple<Ts...>(std::move(inner))
  {
  }

  friend auto tag_invoke(from_http_request_t<path_of<std::tuple<Ts...>>>,
                         http_request& req)
      -> net::awaitable<expected<path_of<std::tuple<Ts...>>, std::error_code>>
  {
    co_return unpack_path(req.path(),
                          std::make_index_sequence<sizeof...(Ts)> {});
  }

private:
  template <std::size_t... Is>
  static auto unpack_path(const path_info& path_info,
                          std::index_sequence<Is...>)
      -> expected<path_of<std::tuple<Ts...>>, std::error_code>
  {
    if (sizeof...(Is) == path_info.size()) {
      return path_of<std::tuple<Ts...>>(
          std::tuple<Ts...> { path_info.at(Is)... });
    }

    return unexpected { make_error_code(error::path_extraction_error) };
  }
};

}

FITORIA_NAMESPACE_END

template <typename... Ts>
struct std::tuple_size<FITORIA_NAMESPACE::web::path_of<std::tuple<Ts...>>>
    : std::tuple_size<std::tuple<Ts...>> { };

template <std::size_t I, typename... Ts>
struct std::tuple_element<I, FITORIA_NAMESPACE::web::path_of<std::tuple<Ts...>>>
    : std::tuple_element<I, std::tuple<Ts...>> { };

#endif
