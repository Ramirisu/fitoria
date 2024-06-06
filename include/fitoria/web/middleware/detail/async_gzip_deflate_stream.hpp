//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#ifndef FITORIA_WEB_MIDDLEWARE_DETAIL_ASYNC_GZIP_DEFLATE_STREAM_HPP
#define FITORIA_WEB_MIDDLEWARE_DETAIL_ASYNC_GZIP_DEFLATE_STREAM_HPP

#include <fitoria/core/config.hpp>

#if defined(FITORIA_HAS_ZLIB)

#include <fitoria/core/dynamic_buffer.hpp>

#include <fitoria/web/middleware/detail/gzip_stream.hpp>

#include <fitoria/web/async_readable_stream_concept.hpp>

FITORIA_NAMESPACE_BEGIN

namespace web::middleware::detail {

template <async_readable_stream NextLayer>
class async_gzip_deflate_stream {
public:
  using is_async_readable_stream = void;

  template <async_readable_stream NextLayer2>
  async_gzip_deflate_stream(NextLayer2&& next)
      : next_(std::forward<NextLayer2>(next))
  {
  }

  auto
  async_read_some() -> awaitable<optional<expected<bytes, std::error_code>>>
  {
    using boost::beast::zlib::error;
    using boost::beast::zlib::Flush;
    using boost::beast::zlib::z_params;

    auto data = co_await next_.async_read_some();
    if (!data) {
      if (finish_) {
        co_return nullopt;
      }

      finish_ = true;

      auto buffer = dynamic_buffer<bytes>();
      auto writable = buffer.prepare(65536);

      auto p = z_params();
      p.next_in = nullptr;
      p.avail_in = 0;
      p.next_out = writable.data();
      p.avail_out = writable.size();

      boost::system::error_code ec;
      deflater_.write(p, Flush::finish, ec);

      FITORIA_ASSERT(ec != error::need_buffers);
      if (ec == error::end_of_stream) {
        ec = {};
      }
      if (ec) {
        co_return unexpected { ec };
      }

      buffer.commit(writable.size() - p.avail_out);
      co_return buffer.release();
    }

    auto& readable = *data;
    if (!readable) {
      co_return unexpected { readable.error() };
    }
    if (readable->empty()) {
      co_return bytes();
    }

    auto buffer = dynamic_buffer<bytes>();
    auto writable
        = buffer.prepare(std::max(readable->size(), std::size_t(65536)));

    auto p = z_params();
    p.next_in = readable->data();
    p.avail_in = readable->size();
    p.next_out = writable.data();
    p.avail_out = writable.size();

    boost::system::error_code ec;
    deflater_.write(p, Flush::none, ec);

    if (ec) {
      co_return unexpected { ec };
    }

    buffer.commit(writable.size() - p.avail_out);
    co_return buffer.release();
  }

  NextLayer next_;
  gzip_deflate_stream deflater_;
  bool finish_ = false;
};

template <typename NextLayer>
async_gzip_deflate_stream(NextLayer&&)
    -> async_gzip_deflate_stream<std::decay_t<NextLayer>>;
}

FITORIA_NAMESPACE_END

#endif

#endif
