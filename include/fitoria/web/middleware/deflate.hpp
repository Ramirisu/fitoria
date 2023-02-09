//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef FITORIA_WEB_MIDDLEWARE_DEFLATE_HPP
#define FITORIA_WEB_MIDDLEWARE_DEFLATE_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/web/middleware/detail/deflate.hpp>

#include <fitoria/web/http_context.hpp>
#include <fitoria/web/http_response.hpp>
#include <fitoria/web/service.hpp>

FITORIA_NAMESPACE_BEGIN

namespace web::middleware {

template <typename Next>
class deflate_service {
  Next next_;

public:
  deflate_service(Next next)
      : next_(std::move(next))
  {
  }

  net::awaitable<http_response> operator()(http_context& c) const
  {
    if (c.request().fields().get(http::field::content_encoding) == "deflate") {
      if (auto plain
          = detail::deflate_decompress<std::string>(net::const_buffer(
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
            .set_body("request body is not a valid deflate stream");
      }
    }

    auto res = co_await next_(c);

    if (res.body().empty()) {
      co_return res;
    }

    if (auto ac = c.request().fields().get(http::field::accept_encoding);
        !res.fields().get(http::field::content_encoding) && ac
        && ac->find("deflate") != std::string::npos) {
      if (auto comp = detail::deflate_compress<std::string>(
              net::const_buffer(res.body().data(), res.body().size()));
          comp) {
        res.set_body(std::move(*comp));
        res.fields().set(http::field::content_encoding, "deflate");
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
            .set_body("failed to compress response body into deflate stream");
      }
    }

    co_return res;
  }
};

class deflate {
public:
  template <typename Next>
  friend constexpr auto
  tag_invoke(tag_t<make_service>, const deflate&, Next&& next)
  {
    return deflate_service(std::forward<Next>(next));
  }
};

}

FITORIA_NAMESPACE_END

#endif
