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
  using is_async_readable_stream = void;

  template <async_readable_stream NextLayer2>
  async_inflate_stream(NextLayer2&& next)
      : next_(std::forward<NextLayer2>(next))
  {
  }

  auto size_hint() const noexcept -> optional<std::size_t>
  {
    return next_.size_hint();
  }

  auto async_read_some(net::mutable_buffer buffer)
      -> awaitable<expected<std::size_t, std::error_code>>
  {
    namespace zlib = boost::beast::zlib;

    if (buffer_.size() < buffer.size()) {
      auto size
          = co_await next_.async_read_some(buffer_.prepare(buffer.size()));
      if (!size) {
        co_return unexpected { size.error() };
      }
      if (*size == 0) {
        co_return unexpected { make_error_code(zlib::error::stream_error) };
      }

      buffer_.commit(*size);
    }

    zlib::z_params p;
    p.next_in = buffer_.cdata().data();
    p.avail_in = buffer_.cdata().size();
    p.next_out = buffer.data();
    p.avail_out = buffer.size();

    boost::system::error_code ec;
    inflater_.write(p, zlib::Flush::sync, ec);
    FITORIA_ASSERT(ec != zlib::error::stream_error);

    if (ec && ec != zlib::error::need_buffers
        && ec != zlib::error::end_of_stream) {
      co_return unexpected { ec };
    }

    buffer_.consume(buffer_.size() - p.avail_in);

    co_return buffer.size() - p.avail_out;
  }

private:
  NextLayer next_;
  boost::beast::zlib::inflate_stream inflater_;
  boost::beast::flat_buffer buffer_;
};

template <typename NextLayer>
async_inflate_stream(NextLayer&&)
    -> async_inflate_stream<std::decay_t<NextLayer>>;

template <async_readable_stream NextLayer>
class async_deflate_stream {
public:
  using is_async_readable_stream = void;

  template <async_readable_stream NextLayer2>
  async_deflate_stream(NextLayer2&& next)
      : next_(std::forward<NextLayer2>(next))
  {
  }

  auto size_hint() const noexcept -> optional<std::size_t>
  {
    return next_.size_hint();
  }

  auto async_read_some(net::mutable_buffer buffer)
      -> awaitable<expected<std::size_t, std::error_code>>
  {
    namespace zlib = boost::beast::zlib;

    if (buffer_.size() < buffer.size()) {
      auto size
          = co_await next_.async_read_some(buffer_.prepare(buffer.size()));
      if (!size) {
        co_return unexpected { size.error() };
      }
      if (*size == 0) {
        co_return unexpected { make_error_code(zlib::error::stream_error) };
      }

      buffer_.commit(*size);
    }

    zlib::z_params p;
    p.next_in = buffer_.cdata().data();
    p.avail_in = buffer_.cdata().size();
    p.next_out = buffer.data();
    p.avail_out = buffer.size();

    boost::system::error_code ec;
    deflater_.write(p, zlib::Flush::sync, ec);
    FITORIA_ASSERT(ec != zlib::error::stream_error);

    buffer_.consume(buffer_.size() - p.avail_in);

    co_return buffer.size() - p.avail_out;
  }

private:
  NextLayer next_;
  boost::beast::zlib::deflate_stream deflater_;
  boost::beast::flat_buffer buffer_;
};

template <typename NextLayer>
async_deflate_stream(NextLayer&&)
    -> async_deflate_stream<std::decay_t<NextLayer>>;
}

FITORIA_NAMESPACE_END

#endif
