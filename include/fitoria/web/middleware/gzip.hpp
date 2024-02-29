//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#ifndef FITORIA_WEB_MIDDLEWARE_GZIP_HPP
#define FITORIA_WEB_MIDDLEWARE_GZIP_HPP

#include <fitoria/core/config.hpp>

#if defined(FITORIA_HAS_ZLIB)

#include <fitoria/core/lazy.hpp>
#include <fitoria/core/type_traits.hpp>

#include <fitoria/web/http_context.hpp>
#include <fitoria/web/http_response.hpp>
#include <fitoria/web/middleware/detail/async_gzip_stream.hpp>
#include <fitoria/web/middleware_concept.hpp>

FITORIA_NAMESPACE_BEGIN

namespace web::middleware {

template <typename Next>
class gzip_middleware {
  friend class gzip;

public:
  auto operator()(http_context& c) const -> lazy<http_response>
  {
    if (c.request().fields().get(http::field::content_encoding)
        == http::fields::content_encoding::gzip()) {
      auto stream
          = detail::async_gzip_inflate_stream(std::move(c.request().body()));
      if (stream.is_chunked()) {
        c.request().set_stream(std::move(stream));
      } else {
        auto data = co_await async_read_all_as<std::vector<std::byte>>(stream);
        if (!data || !*data) {
          co_return http_response(http::status::bad_request)
              .set_field(http::field::content_type,
                         http::fields::content_type::plaintext())
              .set_body("request body contains invalid gzip stream");
        }
        c.request().set_field(http::field::content_length,
                              std::to_string((*data)->size()));
        c.request().set_stream(async_readable_vector_stream(std::move(**data)));
      }
      c.request().fields().erase(http::field::content_encoding);
    }

    auto res = co_await next_(c);

    if (res.body().size_hint() == std::size_t(0)) {
      co_return res;
    }

    if (res.fields().get(http::field::content_encoding)) {
      co_return res;
    }

    if (auto ac = c.request().fields().get(http::field::accept_encoding); ac
        && ac->find(http::fields::content_encoding::gzip())
            != std::string::npos) {
      auto stream = detail::async_gzip_deflate_stream(std::move(res.body()));
      if (stream.is_chunked()) {
        res.set_stream(std::move(stream));
      } else {
        auto data = co_await async_read_all_as<std::vector<std::byte>>(stream);
        if (!data || !*data) {
          co_return http_response(http::status::internal_server_error)
              .set_field(http::field::content_type,
                         http::fields::content_type::plaintext())
              .set_body("unable to compress response body into gzip stream");
        }
        res.set_stream(async_readable_vector_stream(std::move(**data)));
      }
      res.set_field(http::field::content_encoding,
                    http::fields::content_encoding::gzip());
      if (auto vary = res.fields().get(http::field::vary); vary == "*") {
      } else if (vary && !vary->empty()) {
        *vary += ", ";
        *vary += to_string(http::field::content_encoding);
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

template <typename Next>
gzip_middleware(Next&&) -> gzip_middleware<std::decay_t<Next>>;

class gzip {
public:
  template <uncvref_same_as<gzip> Self, typename Next>
  friend constexpr auto tag_invoke(new_middleware_t, Self&& self, Next&& next)
  {
    return std::forward<Self>(self).new_middleware_impl(
        std::forward<Next>(next));
  }

private:
  template <typename Next>
  auto new_middleware_impl(Next&& next) const
  {
    return gzip_middleware(std::forward<Next>(next));
  }
};

}

FITORIA_NAMESPACE_END

#endif

#endif
