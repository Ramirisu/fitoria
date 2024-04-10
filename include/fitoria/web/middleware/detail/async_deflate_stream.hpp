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

class async_deflate_stream_base {
protected:
  enum flags : std::uint32_t {
    need_more_input_buffer = 1,
    need_more_output_buffer = 2,
  };
};

template <async_readable_stream NextLayer>
class async_inflate_stream : public async_deflate_stream_base {
public:
  using is_async_readable_stream = void;

  template <async_readable_stream NextLayer2>
  async_inflate_stream(NextLayer2&& next)
      : next_(std::forward<NextLayer2>(next))
  {
  }

  auto is_sized() const noexcept -> bool
  {
    return false;
  }

  auto async_read_some(net::mutable_buffer buffer)
      -> awaitable<expected<std::size_t, std::error_code>>
  {
    using boost::beast::zlib::error;
    using boost::beast::zlib::Flush;
    using boost::beast::zlib::z_params;

    FITORIA_ASSERT(buffer.size() > 0);

    if (flag_ == 0) {
      co_return unexpected { make_error_code(net::error::eof) };
    }

    if (flag_ & need_more_input_buffer) {
      auto size
          = co_await next_.async_read_some(buffer_.prepare(buffer.size()));
      if (!size) {
        co_return unexpected { size.error() };
      }

      buffer_.commit(*size);
    }

    auto p = z_params();
    p.next_in = buffer_.cdata().data();
    p.avail_in = buffer_.cdata().size();
    p.next_out = buffer.data();
    p.avail_out = buffer.size();

    boost::system::error_code ec;
    inflater_.write(p, Flush::sync, ec);

    if (ec) {
      if (ec == error::end_of_stream) {
        flag_ &= ~need_more_input_buffer;
        flag_ &= ~need_more_output_buffer;
      } else if (ec == error::need_buffers) {
        if (p.avail_in == 0) {
          flag_ |= need_more_input_buffer;
        }
        if (p.avail_out == 0) {
          flag_ |= need_more_output_buffer;
        }
      } else {
        co_return unexpected { ec };
      }
    }

    buffer_.consume(buffer_.size() - p.avail_in);
    co_return buffer.size() - p.avail_out;
  }

private:
  NextLayer next_;
  boost::beast::zlib::inflate_stream inflater_;
  boost::beast::flat_buffer buffer_;
  std::uint32_t flag_ = need_more_input_buffer;
};

template <typename NextLayer>
async_inflate_stream(NextLayer&&)
    -> async_inflate_stream<std::decay_t<NextLayer>>;

template <async_readable_stream NextLayer>
class async_deflate_stream : public async_deflate_stream_base {
public:
  using is_async_readable_stream = void;

  template <async_readable_stream NextLayer2>
  async_deflate_stream(NextLayer2&& next)
      : next_(std::forward<NextLayer2>(next))
  {
  }

  auto is_sized() const noexcept -> bool
  {
    return false;
  }

  auto async_read_some(net::mutable_buffer buffer)
      -> awaitable<expected<std::size_t, std::error_code>>
  {
    using boost::beast::zlib::error;
    using boost::beast::zlib::Flush;
    using boost::beast::zlib::z_params;

    FITORIA_ASSERT(buffer.size() > 0);

    if (flag_ == 0) {
      co_return unexpected { make_error_code(net::error::eof) };
    }

    if (flag_ & need_more_input_buffer) {
      auto size
          = co_await next_.async_read_some(buffer_.prepare(buffer.size()));
      if (!size) {
        co_return unexpected { size.error() };
      }

      buffer_.commit(*size);
    }

    auto p = z_params();
    p.next_in = buffer_.cdata().data();
    p.avail_in = buffer_.cdata().size();
    p.next_out = buffer.data();
    p.avail_out = buffer.size();

    boost::system::error_code ec;
    deflater_.write(p, Flush::sync, ec);

    if (ec) {
      if (ec == error::end_of_stream) {
        flag_ &= ~need_more_input_buffer;
        flag_ &= ~need_more_output_buffer;
      } else if (ec == error::need_buffers) {
        if (p.avail_in == 0) {
          flag_ |= need_more_input_buffer;
        }
        if (p.avail_out == 0) {
          flag_ |= need_more_output_buffer;
        }
      } else {
        co_return unexpected { ec };
      }
    }

    buffer_.consume(buffer_.size() - p.avail_in);
    co_return buffer.size() - p.avail_out;
  }

private:
  NextLayer next_;
  boost::beast::zlib::deflate_stream deflater_;
  boost::beast::flat_buffer buffer_;
  std::uint32_t flag_ = need_more_input_buffer;
};

template <typename NextLayer>
async_deflate_stream(NextLayer&&)
    -> async_deflate_stream<std::decay_t<NextLayer>>;
}

FITORIA_NAMESPACE_END

#endif
