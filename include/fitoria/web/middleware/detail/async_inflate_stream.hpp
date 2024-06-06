//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#ifndef FITORIA_WEB_MIDDLEWARE_DETAIL_ASYNC_INFLATE_STREAM_HPP
#define FITORIA_WEB_MIDDLEWARE_DETAIL_ASYNC_INFLATE_STREAM_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/core/dynamic_buffer.hpp>
#include <fitoria/core/net.hpp>

#include <fitoria/web/async_readable_stream_concept.hpp>

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

  auto
  async_read_some() -> awaitable<optional<expected<bytes, std::error_code>>>
  {
    using boost::beast::zlib::error;
    using boost::beast::zlib::Flush;
    using boost::beast::zlib::z_params;

    auto data = co_await next_.async_read_some();
    if (!data) {
      co_return nullopt;
    }

    if (!*data) {
      co_return unexpected { data->error() };
    }
    if ((*data)->empty()) {
      co_return bytes();
    }

    auto readable = dynamic_buffer<bytes>(std::move(**data));

    auto buffer = dynamic_buffer<bytes>();

    for (;;) {
      auto writable
          = buffer.prepare(std::max(readable.size(), std::size_t(65536)));

      auto p = z_params();
      p.next_in = readable.cdata().data();
      p.avail_in = readable.cdata().size();
      p.next_out = writable.data();
      p.avail_out = writable.size();

      boost::system::error_code ec;
      inflater_.write(p, Flush::sync, ec);

      if (ec == error::end_of_stream) {
        ec = {};
      }
      if (ec) {
        co_return unexpected { ec };
      }

      readable.consume(readable.size() - p.avail_in);
      buffer.commit(writable.size() - p.avail_out);

      if (readable.size() == 0 && p.avail_out > 0) {
        break;
      }
    }

    co_return buffer.release();
  }

private:
  NextLayer next_;
  boost::beast::zlib::inflate_stream inflater_;
};

template <typename NextLayer>
async_inflate_stream(NextLayer&&)
    -> async_inflate_stream<std::decay_t<NextLayer>>;
}

FITORIA_NAMESPACE_END

#endif
