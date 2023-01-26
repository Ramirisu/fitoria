//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef FITORIA_WEB_BASIC_ROUTE_HPP
#define FITORIA_WEB_BASIC_ROUTE_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/web/http.hpp>

#include <string>
#include <vector>

FITORIA_NAMESPACE_BEGIN

template <typename Middleware, typename Handler>
class basic_route {
public:
  basic_route(http::verb method, std::string path, Handler handler)
      : method_(method)
      , path_(std::move(path))
      , handler_(std::move(handler))
  {
  }

  basic_route(http::verb method,
              std::string path,
              std::vector<Middleware> middlewares,
              Handler handler)
      : method_(method)
      , path_(std::move(path))
      , middlewares_(std::move(middlewares))
      , handler_(std::move(handler))
  {
  }

  auto method() const noexcept -> http::verb
  {
    return method_;
  }

  auto path() const noexcept -> const std::string&
  {
    return path_;
  }

  auto middlewares() const noexcept -> const std::vector<Middleware>&
  {
    return middlewares_;
  }

  auto handler() const noexcept -> const Handler&
  {
    return handler_;
  }

  auto rebind_parent(const std::string& parent_path,
                     const std::vector<Middleware>& parent_middlewares) const
      -> basic_route
  {
    auto middlewares = parent_middlewares;
    middlewares.insert(middlewares.end(), middlewares_.begin(),
                       middlewares_.end());
    return basic_route(method_, parent_path + path_, std::move(middlewares),
                       handler_);
  }

  static auto GET(const std::string& path, Handler handler) -> basic_route
  {
    return basic_route(http::verb::get, path, std::move(handler));
  }

  static auto POST(const std::string& path, Handler handler) -> basic_route
  {
    return basic_route(http::verb::post, path, std::move(handler));
  }

  static auto PUT(const std::string& path, Handler handler) -> basic_route
  {
    return basic_route(http::verb::put, path, std::move(handler));
  }

  static auto PATCH(const std::string& path, Handler handler) -> basic_route
  {
    return basic_route(http::verb::patch, path, std::move(handler));
  }

  static auto DELETE_(const std::string& path, Handler handler) -> basic_route
  {
    return basic_route(http::verb::delete_, path, std::move(handler));
  }

  static auto HEAD(const std::string& path, Handler handler) -> basic_route
  {
    return basic_route(http::verb::head, path, std::move(handler));
  }

  static auto OPTIONS(const std::string& path, Handler handler) -> basic_route
  {
    return basic_route(http::verb::options, path, std::move(handler));
  }

private:
  http::verb method_;
  std::string path_;
  std::vector<Middleware> middlewares_;
  Handler handler_;
};

FITORIA_NAMESPACE_END

#endif
