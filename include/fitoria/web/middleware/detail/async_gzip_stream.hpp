//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#ifndef FITORIA_WEB_MIDDLEWARE_DETAIL_ASYNC_GZIP_STREAM_HPP
#define FITORIA_WEB_MIDDLEWARE_DETAIL_ASYNC_GZIP_STREAM_HPP

#include <fitoria/core/config.hpp>

#if defined(FITORIA_HAS_ZLIB)

#include <fitoria/core/net.hpp>

#include <fitoria/web/async_readable_stream_concept.hpp>
#include <fitoria/web/http/http.hpp>

#include <zlib.h>

FITORIA_NAMESPACE_BEGIN

namespace web::middleware::detail {

class gzip_stream_base {
protected:
  boost::system::error_code from_native_error(int e)
  {
    if (e == Z_OK) {
      return {};
    }

    return make_error_code(from_native_error_impl(e));
  }

  void to_native_param(boost::beast::zlib::z_params& p)
  {
    FITORIA_ASSERT(stream_);
    stream_->next_in
        = static_cast<unsigned char*>(const_cast<void*>(p.next_in));
    stream_->avail_in = static_cast<unsigned int>(p.avail_in);
    stream_->next_out = static_cast<unsigned char*>(p.next_out);
    stream_->avail_out = static_cast<unsigned int>(p.avail_out);
  }

  void from_native_param(boost::beast::zlib::z_params& p)
  {
    FITORIA_ASSERT(stream_);
    p.next_in = stream_->next_in;
    p.avail_in = stream_->avail_in;
    p.next_out = stream_->next_out;
    p.avail_out = stream_->avail_out;
    p.total_in = stream_->total_in;
    p.total_out = stream_->total_out;
  }

  optional<z_stream> stream_ {};

private:
  boost::beast::zlib::error from_native_error_impl(int e)
  {
    switch (e) {
    case Z_STREAM_END:
      return boost::beast::zlib::error::end_of_stream;
    case Z_NEED_DICT:
      return boost::beast::zlib::error::need_dict;
    case Z_STREAM_ERROR:
      return boost::beast::zlib::error::stream_error;
    case Z_BUF_ERROR:
      return boost::beast::zlib::error::need_buffers;
    case Z_ERRNO:
    case Z_DATA_ERROR:
    case Z_MEM_ERROR:
    case Z_VERSION_ERROR:
    default:
      return boost::beast::zlib::error::general;
    }
  }
};

class gzip_inflate_stream : public gzip_stream_base {
public:
  gzip_inflate_stream() = default;

  ~gzip_inflate_stream()
  {
    if (stream_) {
      ::inflateEnd(&*stream_);
    }
  }

  void write(boost::beast::zlib::z_params& p,
             boost::beast::zlib::Flush flush,
             boost::system::error_code& ec)
  {
    if (!stream_) {
      init(p);
    } else {
      to_native_param(p);
    }
    ec = from_native_error(::inflate(&*stream_, static_cast<int>(flush)));
    from_native_param(p);
  }

private:
  void init(boost::beast::zlib::z_params& p)
  {
    stream_.emplace(z_stream {});
    stream_->zalloc = Z_NULL;
    stream_->zfree = Z_NULL;
    stream_->opaque = Z_NULL;
    to_native_param(p);
    [[maybe_unused]] auto ret = ::inflateInit2(&*stream_, 0xf + 0x10);
    FITORIA_ASSERT(ret == Z_OK);
  }
};

template <async_readable_stream NextLayer>
class async_gzip_inflate_stream {
public:
  using is_async_readable_stream = void;

  template <async_readable_stream NextLayer2>
  async_gzip_inflate_stream(NextLayer2&& next)
      : next_(std::forward<NextLayer2>(next))
  {
  }

  auto size_hint() const noexcept -> optional<std::size_t>
  {
    return nullopt;
  }

  auto async_read_some(net::mutable_buffer buffer)
      -> awaitable<expected<std::size_t, std::error_code>>
  {
    namespace zlib = boost::beast::zlib;

    auto hint = next_.size_hint() ? *next_.size_hint() : 4096;
    if (hint == 0 && buffer_.size() == 0) {
      co_return unexpected { make_error_code(net::error::eof) };
    }

    if (hint > 0) {
      auto size = co_await next_.async_read_some(buffer_.prepare(hint));
      if (!size) {
        co_return unexpected { size.error() };
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

    if (ec == zlib::error::need_buffers || ec == zlib::error::end_of_stream) {
      ec = {};
    }
    if (ec) {
      co_return unexpected { ec };
    }

    buffer_.consume(buffer_.size() - p.avail_in);

    co_return buffer.size() - p.avail_out;
  }

private:
  NextLayer next_;
  gzip_inflate_stream inflater_;
  boost::beast::flat_buffer buffer_;
};

template <typename NextLayer>
async_gzip_inflate_stream(NextLayer&&)
    -> async_gzip_inflate_stream<std::decay_t<NextLayer>>;

class gzip_deflate_stream : public gzip_stream_base {
public:
  gzip_deflate_stream() = default;

  ~gzip_deflate_stream()
  {
    if (stream_) {
      ::deflateEnd(&*stream_);
    }
  }

  void write(boost::beast::zlib::z_params& p,
             boost::beast::zlib::Flush flush,
             boost::system::error_code& ec)
  {
    if (!stream_) {
      init(p);
    } else {
      to_native_param(p);
    }
    ec = from_native_error(::deflate(&*stream_, static_cast<int>(flush)));
    from_native_param(p);
  }

private:
  void init(boost::beast::zlib::z_params& p)
  {
    stream_.emplace(z_stream {});
    stream_->zalloc = Z_NULL;
    stream_->zfree = Z_NULL;
    stream_->opaque = Z_NULL;
    to_native_param(p);
    [[maybe_unused]] auto ret = ::deflateInit2(&*stream_,
                                               Z_BEST_COMPRESSION,
                                               Z_DEFLATED,
                                               0xf + 0x10,
                                               9,
                                               Z_DEFAULT_STRATEGY);
    FITORIA_ASSERT(ret == Z_OK);
  }
};

template <async_readable_stream NextLayer>
class async_gzip_deflate_stream {
public:
  using is_async_readable_stream = void;

  template <async_readable_stream NextLayer2>
  async_gzip_deflate_stream(NextLayer2&& next)
      : next_(std::forward<NextLayer2>(next))
  {
  }

  auto size_hint() const noexcept -> optional<std::size_t>
  {
    return nullopt;
  }

  auto async_read_some(net::mutable_buffer buffer)
      -> awaitable<expected<std::size_t, std::error_code>>
  {
    namespace zlib = boost::beast::zlib;

    auto hint = next_.size_hint() ? *next_.size_hint() : 4096;
    if (hint == 0 && buffer_.size() == 0) {
      co_return unexpected { make_error_code(net::error::eof) };
    }

    if (hint > 0) {
      auto size = co_await next_.async_read_some(buffer_.prepare(hint));
      if (!size) {
        co_return unexpected { size.error() };
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

    if (ec == zlib::error::need_buffers || ec == zlib::error::end_of_stream) {
      ec = {};
    }
    if (ec) {
      co_return unexpected { ec };
    }

    buffer_.consume(buffer_.size() - p.avail_in);

    co_return buffer.size() - p.avail_out;
  }

  NextLayer next_;
  gzip_deflate_stream deflater_;
  boost::beast::flat_buffer buffer_;
};

template <typename NextLayer>
async_gzip_deflate_stream(NextLayer&&)
    -> async_gzip_deflate_stream<std::decay_t<NextLayer>>;
}

FITORIA_NAMESPACE_END

#endif

#endif
