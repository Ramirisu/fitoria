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

namespace web::middleware {

template <typename Next>
class gzip_service {
  Next next_;

public:
  gzip_service(Next next)
      : next_(std::move(next))
  {
  }

  net::awaitable<http_response> operator()(http_context& c) const
  {
    if (c.request().fields().get(http::field::content_encoding) == "gzip") {
      if (auto plain = detail::gzip_decompress<std::string>(net::const_buffer(
              c.request().body().data(), c.request().body().size()));
          plain) {
        c.request().fields().erase(http::field::content_encoding);
        c.request().fields().set(http::field::content_length,
                                 std::to_string(plain->size()));
        c.request().set_body(std::move(*plain));
      } else {
        co_return http_response(http::status::bad_request)
            .set_field(http::field::content_type,
                       http::fields::content_type::plaintext())
            .set_body("request body is not a valid gzip stream");
      }
    }

    auto res = co_await next_(c);

    if (res.body().empty()) {
      co_return res;
    }

    if (auto ac = c.request().fields().get(http::field::accept_encoding);
        !res.fields().get(http::field::content_encoding) && ac
        && ac->find("gzip") != std::string::npos) {
      if (auto comp = detail::gzip_compress<std::string>(
              net::const_buffer(res.body().data(), res.body().size()));
          comp) {
        res.set_body(std::move(*comp));
        res.fields().set(http::field::content_encoding, "gzip");
        if (auto vary = res.fields().get(http::field::vary); vary == "*") {
        } else if (vary && !vary->empty()) {
          *vary += ", ";
          *vary += to_string(http::field::content_encoding);
        } else {
          res.set_field(http::field::vary,
                        to_string(http::field::content_encoding));
        }
      } else {
        co_return http_response(http::status::internal_server_error)
            .set_field(http::field::content_type,
                       http::fields::content_type::plaintext())
            .set_body("failed to compress response body into gzip stream");
      }
    }

    co_return res;
  }
};

class gzip {
public:
  template <typename Next>
  auto create(Next next) const
  {
    return gzip_service(std::move(next));
  }
};

}

FITORIA_NAMESPACE_END

#endif

#endif
