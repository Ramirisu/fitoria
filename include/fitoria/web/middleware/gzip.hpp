//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#ifndef FITORIA_WEB_MIDDLEWARE_GZIP_HPP
#define FITORIA_WEB_MIDDLEWARE_GZIP_HPP

#include <fitoria/core/config.hpp>

#if defined(FITORIA_HAS_ZLIB)

#include <fitoria/core/net.hpp>
#include <fitoria/core/type_traits.hpp>

#include <fitoria/web/http_request.hpp>
#include <fitoria/web/http_response.hpp>
#include <fitoria/web/middleware/detail/async_gzip_stream.hpp>
#include <fitoria/web/middleware_concept.hpp>

FITORIA_NAMESPACE_BEGIN

namespace web::middleware {

template <typename Request, typename Response, typename Next>
class gzip_middleware {
  friend class gzip;

public:
  auto operator()(Request req) const -> Response
  {
    if (req.fields().get(http::field::content_encoding)
        == http::fields::content_encoding::gzip()) {
      req.set_stream(detail::async_gzip_inflate_stream(std::move(req.body())));
      req.fields().erase(http::field::content_encoding);
    }

    auto res = co_await next_(req);

    if (res.body().size_hint() == std::size_t(0)) {
      co_return res;
    }

    if (res.fields().get(http::field::content_encoding)) {
      co_return res;
    }

    if (auto ac = req.fields().get(http::field::accept_encoding); ac
        && ac->find(http::fields::content_encoding::gzip())
            != std::string::npos) {
      res.set_stream(detail::async_gzip_deflate_stream(std::move(res.body())));
      res.set_field(http::field::content_encoding,
                    http::fields::content_encoding::gzip());
      if (auto vary = res.fields().get(http::field::vary); vary == "*") {
      } else if (vary && !vary->empty()) {
        auto new_vary = std::string(*vary);
        new_vary += ", ";
        new_vary += to_string(http::field::content_encoding);
        res.set_field(http::field::vary, new_vary);
      } else {
        res.set_field(http::field::vary,
                      to_string(http::field::content_encoding));
      }
    }

    co_return res;
  }

private:
  template <typename Next2>
  gzip_middleware(Next2&& next)
      : next_(std::forward<Next2>(next))
  {
  }

  Next next_;
};

class gzip {
public:
  template <typename Request,
            typename Response,
            decay_to<gzip> Self,
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
    return gzip_middleware<Request, Response, std::decay_t<Next>>(
        std::forward<Next>(next));
  }
};

}

FITORIA_NAMESPACE_END

#endif

#endif
