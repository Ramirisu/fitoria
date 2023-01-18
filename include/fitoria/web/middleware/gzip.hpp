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
      auto dec = decompress<std::string>(net::const_buffer(
          c.request().body().data(), c.request().body().size()));
      c.request().headers().erase(http::field::content_encoding);
      c.request().headers().set(http::field::content_length,
                                std::to_string(dec.size()));
      c.request().set_body(std::move(dec));
    }

    auto res = co_await c.next();

    if (auto ac = c.request().headers().get(http::field::accept_encoding);
        !res.headers().get(http::field::content_encoding) && ac
        && ac->find("gzip") != std::string::npos) {
      res.set_body(compress<std::string>(
          net::const_buffer(res.body().data(), res.body().size())));
      res.headers().set(http::field::content_encoding, "gzip");
    }

    co_return res;
  }

  template <typename R>
  static R decompress(net::const_buffer in)
  {
    namespace bio = boost::iostreams;

    R out;
    bio::filtering_ostream stream;
    stream.push(bio::gzip_decompressor());
    stream.push(detail::sink(out));
    bio::copy(bio::basic_array_source<char>(static_cast<const char*>(in.data()),
                                            in.size()),
              stream);
    return out;
  }

  template <typename R>
  static R compress(net::const_buffer in)
  {
    namespace bio = boost::iostreams;

    R out;
    bio::filtering_ostream stream;
    stream.push(
        bio::gzip_compressor(bio::gzip_params(bio::gzip::best_compression)));
    stream.push(detail::sink(out));
    bio::copy(bio::basic_array_source<char>(static_cast<const char*>(in.data()),
                                            in.size()),
              stream);
    return out;
  }
};
}

FITORIA_NAMESPACE_END

#endif

#endif
