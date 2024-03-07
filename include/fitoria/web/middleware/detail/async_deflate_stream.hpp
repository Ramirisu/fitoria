//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#ifndef FITORIA_WEB_MIDDLEWARE_DETAIL_ASYNC_DEFLATE_STREAM_HPP
#define FITORIA_WEB_MIDDLEWARE_DETAIL_ASYNC_DEFLATE_STREAM_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/core/net.hpp>

#include <fitoria/web/async_readable_stream_concept.hpp>
#include <fitoria/web/http/http.hpp>

FITORIA_NAMESPACE_BEGIN

namespace web::middleware::detail {

template <async_readable_stream NextLayer>
class async_inflate_stream {
public:
  template <async_readable_stream NextLayer2>
  async_inflate_stream(NextLayer2&& next)
      : next_(std::forward<NextLayer2>(next))
  {
  }

  bool is_chunked() const noexcept
  {
    return next_.is_chunked();
  }

  optional<std::size_t> size_hint() const noexcept
  {
    return next_.size_hint();
  }

  auto async_read_next()
      -> lazy<optional<expected<std::vector<std::byte>, net::error_code>>>
  {
    namespace zlib = boost::beast::zlib;

    auto chunk = co_await next_.async_read_next();
    if (!chunk) {
      co_return nullopt;
    }
    if (!*chunk) {
      co_return unexpected { (*chunk).error() };
    }
    if ((*chunk)->empty()) {
      co_return unexpected { make_error_code(zlib::error::stream_error) };
    }

    const auto& in = **chunk;
    std::vector<std::byte> out;
    out.resize(std::max<std::size_t>(in.size(), 16));

    zlib::z_params p;
    p.next_in = in.data();
    p.avail_in = in.size();
    p.next_out = out.data();
    p.avail_out = out.size();

    while (true) {
      net::error_code ec;
      inflater_.write(p, zlib::Flush::sync, ec);
      FITORIA_ASSERT(ec != zlib::error::stream_error);

      if (ec && ec != zlib::error::need_buffers
          && ec != zlib::error::end_of_stream) {
        co_return unexpected { ec };
      }
      if (p.avail_out > 0) {
        break;
      }

      const auto size = out.size();
      out.resize(size * 2);
      p.next_out = out.data() + size;
      p.avail_out = size;
    }

    out.resize(out.size() - p.avail_out);
    co_return out;
  }

private:
  NextLayer next_;
  boost::beast::zlib::inflate_stream inflater_;
};

template <typename NextLayer>
async_inflate_stream(NextLayer&&)
    -> async_inflate_stream<std::decay_t<NextLayer>>;

template <async_readable_stream NextLayer>
class async_deflate_stream {
public:
  template <async_readable_stream NextLayer2>
  async_deflate_stream(NextLayer2&& next)
      : next_(std::forward<NextLayer2>(next))
  {
  }

  bool is_chunked() const noexcept
  {
    return next_.is_chunked();
  }

  optional<std::size_t> size_hint() const noexcept
  {
    return next_.size_hint();
  }

  auto async_read_next()
      -> lazy<optional<expected<std::vector<std::byte>, net::error_code>>>
  {
    namespace zlib = boost::beast::zlib;

    auto chunk = co_await next_.async_read_next();
    if (!chunk) {
      co_return nullopt;
    }
    if (!*chunk) {
      co_return unexpected { (*chunk).error() };
    }
    if ((*chunk)->empty()) {
      co_return unexpected { make_error_code(zlib::error::stream_error) };
    }

    const auto& in = **chunk;
    std::vector<std::byte> out;
    out.resize(std::max<std::size_t>(in.size(), 16));

    zlib::z_params p;
    p.next_in = in.data();
    p.avail_in = in.size();
    p.next_out = out.data();
    p.avail_out = out.size();

    while (true) {
      net::error_code ec;
      deflater_.write(p, zlib::Flush::sync, ec);
      FITORIA_ASSERT(ec != zlib::error::stream_error);

      if (p.avail_out > 0) {
        break;
      }

      const auto size = out.size();
      out.resize(size * 2);
      p.next_out = out.data() + size;
      p.avail_out = size;
    }

    out.resize(out.size() - p.avail_out);
    co_return out;
  }

private:
  NextLayer next_;
  boost::beast::zlib::deflate_stream deflater_;
};

template <typename NextLayer>
async_deflate_stream(NextLayer&&)
    -> async_deflate_stream<std::decay_t<NextLayer>>;

}

FITORIA_NAMESPACE_END

#endif
