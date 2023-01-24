//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef FITORIA_WEB_MIDDLEWARE_GZIP_HPP
#define FITORIA_WEB_MIDDLEWARE_GZIP_HPP

#if defined(FITORIA_HAS_ZLIB)

#include <fitoria/core/config.hpp>

#include <fitoria/web/middleware/detail/gzip.hpp>

#include <fitoria/web/http_context.hpp>
#include <fitoria/web/http_response.hpp>

FITORIA_NAMESPACE_BEGIN

namespace middleware {

class gzip {
public:
  net::awaitable<http_response> operator()(http_context& c) const
  {
    if (c.request().headers().get(http::field::content_encoding) == "gzip") {
      if (auto plain = decompress<std::string>(net::const_buffer(
              c.request().body().data(), c.request().body().size()));
          plain) {
        c.request().headers().erase(http::field::content_encoding);
        c.request().headers().set(http::field::content_length,
                                  std::to_string(plain->size()));
        c.request().set_body(std::move(*plain));
      } else {
        co_return http_response(http::status::bad_request)
            .set_header(http::field::content_type,
                        http::fields::content_type::plaintext())
            .set_body("request body is not a valid gzip stream");
      }
    }

    auto res = co_await c.next();

    if (auto ac = c.request().headers().get(http::field::accept_encoding);
        !res.headers().get(http::field::content_encoding) && ac
        && ac->find("gzip") != std::string::npos) {
      if (auto comp = compress<std::string>(
              net::const_buffer(res.body().data(), res.body().size()));
          comp) {
        res.set_body(std::move(*comp));
        res.headers().set(http::field::content_encoding, "gzip");
      } else {
        co_return http_response(http::status::internal_server_error)
            .set_header(http::field::content_type,
                        http::fields::content_type::plaintext())
            .set_body("failed to compress response body into gzip stream");
      }
    }

    co_return res;
  }

  template <typename R>
  static expected<R, error_code> decompress(net::const_buffer in)
  {
    if (in.size() == 0) {
      return R();
    }

    return detail::gzip_inflate<R>(in);
  }

  template <typename R>
  static expected<R, error_code> compress(net::const_buffer in)
  {
    return detail::gzip_deflate<R>(in);
  }
};
}

FITORIA_NAMESPACE_END

#endif

#endif
