//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#ifndef FITORIA_WEB_PATH_HPP
#define FITORIA_WEB_PATH_HPP

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
class path {
public:
  explicit path(T inner)
      : inner_(std::move(inner))
  {
  }

  auto inner() const noexcept -> const T&
  {
    return inner_;
  }

  friend auto tag_invoke(from_http_request_t<path<T>>, http_request& req)
      -> net::awaitable<expected<path<T>, error_code>>
  {
    co_return unpack_path(
        req.path(), std::make_index_sequence<boost::pfr::tuple_size_v<T>> {});
  }

private:
  template <std::size_t... Is>
  static auto unpack_path(const path_info& path_info,
                          std::index_sequence<Is...>)
      -> expected<path<T>, error_code>
  {
    if (sizeof...(Is) == path_info.size()) {
      T result;
      if ((try_assign_field_index<Is>(path_info, result) && ...)) {
        return path<T>(std::move(result));
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

  T inner_;
};

#else

template <typename T>
class path;

#endif

template <typename... Ts>
class path<std::tuple<Ts...>> {
public:
  explicit path(std::tuple<Ts...> inner)
      : inner_(std::move(inner))
  {
  }

  auto inner() const noexcept -> const std::tuple<Ts...>&
  {
    return inner_;
  }

  friend auto tag_invoke(from_http_request_t<path<std::tuple<Ts...>>>,
                         http_request& req)
      -> net::awaitable<expected<path<std::tuple<Ts...>>, error_code>>
  {
    co_return unpack_path(req.path(),
                          std::make_index_sequence<sizeof...(Ts)> {});
  }

private:
  template <std::size_t... Is>
  static auto unpack_path(const path_info& path_info,
                          std::index_sequence<Is...>)
      -> expected<path<std::tuple<Ts...>>, error_code>
  {
    if (sizeof...(Is) <= path_info.size()) {
      return path<std::tuple<Ts...>>(std::tuple<Ts...> { path_info.at(Is)... });
    }

    return unexpected { make_error_code(error::path_extraction_error) };
  }

  std::tuple<Ts...> inner_;
};

}

FITORIA_NAMESPACE_END

#endif
