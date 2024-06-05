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

#include <fitoria/web/detail/extract_query.hpp>

#include <fitoria/web/error.hpp>
#include <fitoria/web/from_request.hpp>

#include <boost/pfr.hpp>

#include <tuple>

FITORIA_NAMESPACE_BEGIN

namespace web {

template <typename T>
class path_of : public T {
public:
  static_assert(not_cvref<T>, "T must not be cvref qualified");

  explicit path_of(T inner)
      : T(std::move(inner))
  {
  }

  friend auto tag_invoke(from_request_t<path_of<T>>, request& req)
      -> awaitable<expected<path_of<T>, response>>
  {
    if (req.path().size() != boost::pfr::tuple_size_v<T>) {
      co_return unexpected {
        response::internal_server_error()
            .set_header(http::field::content_type, mime::text_plain())
            .set_body("numbers of key/value pairs are not expected.")
      };
    }

    if (auto result = detail::extract_query<path_of<T>>::extract(
            req.path(),
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

template <typename... Ts>
class path_of<std::tuple<Ts...>> : public std::tuple<Ts...> {
public:
  static_assert((not_cvref<Ts> && ...), "Ts... must not be cvref qualified");

  path_of(Ts... ts)
      : std::tuple<Ts...>(std::move(ts)...)
  {
  }

  friend auto tag_invoke(from_request_t<path_of<std::tuple<Ts...>>>,
                         request& req)
      -> awaitable<expected<path_of<std::tuple<Ts...>>, response>>
  {
    if (req.path().size() != sizeof...(Ts)) {
      co_return unexpected {
        response::bad_request()
            .set_header(http::field::content_type, mime::text_plain())
            .set_body("numbers of key/value pairs are not expected.")
      };
    }

    if (auto result
        = extract_path(req.path(), std::make_index_sequence<sizeof...(Ts)> {});
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

private:
  template <std::size_t... Is>
  static auto extract_path(const path_info& path_info,
                           std::index_sequence<Is...>)
      -> expected<path_of<std::tuple<Ts...>>, std::error_code>
  {
    auto args = std::tuple<expected<Ts, std::error_code>...> { from_string<Ts>(
        path_info.at(Is))... };

    if (auto err = get_error_of<0, sizeof...(Is)>(args); err) {
      return unexpected { *err };
    }

    return std::apply(
        [](auto&... ts) -> path_of<std::tuple<Ts...>> {
          return path_of<std::tuple<Ts...>> { std::move(ts.value())... };
        },
        args);
  }

  template <std::size_t I, std::size_t Count>
  static auto
  get_error_of(const std::tuple<expected<Ts, std::error_code>...>& args)
      -> optional<std::error_code>
  {
    if constexpr (I < Count) {
      if (auto arg = std::get<I>(args); !arg) {
        return arg.error();
      }

      return get_error_of<I + 1, Count>(args);
    } else {
      return nullopt;
    }
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
