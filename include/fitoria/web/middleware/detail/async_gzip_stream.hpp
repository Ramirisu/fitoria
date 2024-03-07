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
  net::error_code from_native_error(int e)
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
             net::error_code& ec)
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
  template <async_readable_stream NextLayer2>
  async_gzip_inflate_stream(NextLayer2&& next)
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
  gzip_inflate_stream inflater_;
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
             net::error_code& ec)
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
  template <async_readable_stream NextLayer2>
  async_gzip_deflate_stream(NextLayer2&& next)
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

  NextLayer next_;
  gzip_deflate_stream deflater_;
};

template <typename NextLayer>
async_gzip_deflate_stream(NextLayer&&)
    -> async_gzip_deflate_stream<std::decay_t<NextLayer>>;
}

FITORIA_NAMESPACE_END

#endif

#endif
