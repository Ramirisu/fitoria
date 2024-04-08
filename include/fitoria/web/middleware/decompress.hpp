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
#include <fitoria/core/type_traits.hpp>

#include <fitoria/web/detail/string.hpp>

#include <fitoria/web/http_request.hpp>
#include <fitoria/web/http_response.hpp>
#include <fitoria/web/middleware/detail/async_deflate_stream.hpp>
#include <fitoria/web/middleware/detail/async_gzip_stream.hpp>
#include <fitoria/web/middleware_concept.hpp>

FITORIA_NAMESPACE_BEGIN

namespace web::middleware {

template <typename Request, typename Response, typename Next>
class decompress_middleware {
  friend class decompress;

public:
  auto operator()(Request req) const -> Response
  {
    if (auto str = req.fields().get(http::field::content_encoding); str) {
      auto encodings = web::detail::split_of(*str, ",");
      std::reverse(encodings.begin(), encodings.end());

      for (auto& encoding : encodings) {
        if (encoding == http::fields::content_encoding::deflate()) {
          req.set_stream(detail::async_inflate_stream(std::move(req.body())));
#if defined(FITORIA_HAS_ZLIB)
        } else if (encoding == http::fields::content_encoding::gzip()) {
          req.set_stream(
              detail::async_gzip_inflate_stream(std::move(req.body())));
#endif
        } else if (encoding == http::fields::content_encoding::identity()) {
        } else {
          break;
        }
        *str = remove_last_encoding(*str);
      }
      if (str->empty()) {
        req.fields().erase(http::field::content_encoding);
      } else {
        req.fields().set(http::field::content_encoding, *str);
      }
    }

    auto res = co_await next_(req);

    co_return res;
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
  tag_invoke(new_middleware_t<Request, Response>, Self&& self, Next&& next)
  {
    return std::forward<Self>(self)
        .template new_middleware_impl<Request, Response>(
            std::forward<Next>(next));
  }

private:
  template <typename Request, typename Response, typename Next>
  auto new_middleware_impl(Next&& next) const
  {
    return decompress_middleware<Request, Response, std::decay_t<Next>>(
        std::forward<Next>(next));
  }
};

}

FITORIA_NAMESPACE_END

#endif
