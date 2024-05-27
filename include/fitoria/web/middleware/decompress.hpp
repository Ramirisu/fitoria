//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#ifndef FITORIA_WEB_MIDDLEWARE_DECOMPRESS_HPP
#define FITORIA_WEB_MIDDLEWARE_DECOMPRESS_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/core/net.hpp>
#include <fitoria/core/strings.hpp>
#include <fitoria/core/type_traits.hpp>
#include <fitoria/core/utility.hpp>

#include <fitoria/web/middleware/detail/async_brotli_inflate_stream.hpp>
#include <fitoria/web/middleware/detail/async_deflate_stream.hpp>
#include <fitoria/web/middleware/detail/async_gzip_stream.hpp>
#include <fitoria/web/request.hpp>
#include <fitoria/web/response.hpp>
#include <fitoria/web/to_middleware.hpp>

FITORIA_NAMESPACE_BEGIN

namespace web::middleware {

template <typename Request, typename Response, typename Next>
class decompress_middleware {
  friend class decompress;

public:
  auto operator()(Request req) const -> Response
  {
    if (auto str = req.header().get(http::field::content_encoding); str) {
      auto encs = split_of(*str, ",");
      std::reverse(encs.begin(), encs.end());

      for (auto& enc : encs) {
        if (iequals(enc, "deflate")) {
          req.set_stream(detail::async_inflate_stream(std::move(req.body())));
#if defined(FITORIA_HAS_ZLIB)
        } else if (iequals(enc, "gzip")) {
          req.set_stream(
              detail::async_gzip_inflate_stream(std::move(req.body())));
#endif
#if defined(FITORIA_HAS_BROTLI)
        } else if (iequals(enc, "brotli")) {
          req.set_stream(
              detail::async_brotli_inflate_stream(std::move(req.body())));
#endif
        } else if (iequals(enc, "identity")) {
        } else {
          break;
        }
        *str = remove_last_encoding(*str);
      }
      if (str->empty()) {
        req.header().erase(http::field::content_encoding);
      } else {
        req.header().set(http::field::content_encoding, *str);
      }
    }

    co_return co_await next_(req);
  }

private:
  template <typename Next2>
  decompress_middleware(Next2&& next)
      : next_(std::forward<Next2>(next))
  {
  }

  static auto remove_last_encoding(std::string_view str) -> std::string_view
  {
    if (auto pos = str.find_last_of(','); pos != std::string_view::npos) {
      str = str.substr(0, pos);
    } else {
      str = {};
    }

    return str;
  }

  Next next_;
};

class decompress {
public:
  template <typename Request,
            typename Response,
            decay_to<decompress> Self,
            typename Next>
  friend auto
  tag_invoke(to_middleware_t<Request, Response>, Self&& self, Next&& next)
  {
    return std::forward<Self>(self)
        .template to_middleware_impl<Request, Response>(
            std::forward<Next>(next));
  }

private:
  template <typename Request, typename Response, typename Next>
  auto to_middleware_impl(Next&& next) const
  {
    return decompress_middleware<Request, Response, std::decay_t<Next>>(
        std::forward<Next>(next));
  }
};

}

FITORIA_NAMESPACE_END

#endif
