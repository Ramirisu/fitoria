//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#ifndef FITORIA_WEB_MIDDLEWARE_DETAIL_GZIP_STREAM_HPP
#define FITORIA_WEB_MIDDLEWARE_DETAIL_GZIP_STREAM_HPP

#include <fitoria/core/config.hpp>

#if defined(FITORIA_HAS_ZLIB)

#include <fitoria/core/net.hpp>

#include <system_error>

#include <zlib.h>

FITORIA_NAMESPACE_BEGIN

namespace web::middleware::detail {

class gzip_stream_base {
protected:
  enum wbits {
    max_wbits = 15,
    gzip_header = 16,
  };

  auto from_native_error(int e) -> boost::system::error_code
  {
    using boost::beast::zlib::error;

    switch (e) {
    case Z_OK:
      return {};
    case Z_STREAM_END:
      return make_error_code(error::end_of_stream);
    case Z_NEED_DICT:
      return make_error_code(error::need_dict);
    case Z_STREAM_ERROR:
      return make_error_code(error::stream_error);
    case Z_BUF_ERROR:
      return make_error_code(error::need_buffers);
    case Z_ERRNO:
    case Z_DATA_ERROR:
    case Z_MEM_ERROR:
    case Z_VERSION_ERROR:
    default:
      break;
    }

    return make_error_code(error::general);
  }

  void to_native(boost::beast::zlib::z_params& p)
  {
    FITORIA_ASSERT(stream_);
    stream_->next_in
        = static_cast<unsigned char*>(const_cast<void*>(p.next_in));
    stream_->avail_in = static_cast<unsigned int>(p.avail_in);
    stream_->next_out = static_cast<unsigned char*>(p.next_out);
    stream_->avail_out = static_cast<unsigned int>(p.avail_out);
  }

  void from_native(boost::beast::zlib::z_params& p)
  {
    FITORIA_ASSERT(stream_);
    p.next_in = stream_->next_in;
    p.avail_in = stream_->avail_in;
    p.next_out = stream_->next_out;
    p.avail_out = stream_->avail_out;
    p.total_in = stream_->total_in;
    p.total_out = stream_->total_out;
  }

  int to_native(boost::beast::zlib::Flush flush)
  {
    using boost::beast::zlib::Flush;

    switch (flush) {
    case Flush::none:
      return Z_NO_FLUSH;
    case Flush::block:
      return Z_BLOCK;
    case Flush::partial:
      return Z_PARTIAL_FLUSH;
    case Flush::sync:
      return Z_SYNC_FLUSH;
    case Flush::full:
      return Z_FULL_FLUSH;
    case Flush::finish:
      return Z_FINISH;
    case Flush::trees:
      return Z_TREES;
    default:
      break;
    }

    return Z_NO_FLUSH;
  }

  std::unique_ptr<z_stream> stream_;
};

class gzip_inflate_stream : public gzip_stream_base {
public:
  gzip_inflate_stream()
  {
    auto s = std::make_unique<z_stream>();
    s->zalloc = Z_NULL;
    s->zfree = Z_NULL;
    s->opaque = Z_NULL;
    if (auto e = ::inflateInit2(&*s, max_wbits | gzip_header); e != Z_OK) {
      FITORIA_THROW_OR(std::system_error(from_native_error(e)),
                       std::terminate());
    }

    stream_ = std::move(s);
  }

  ~gzip_inflate_stream()
  {
    if (stream_) {
      ::inflateEnd(&*stream_);
    }
  }

  gzip_inflate_stream(const gzip_inflate_stream&) = delete;

  gzip_inflate_stream& operator=(const gzip_inflate_stream&) = delete;

  gzip_inflate_stream(gzip_inflate_stream&&) = default;

  gzip_inflate_stream& operator=(gzip_inflate_stream&&) = default;

  void write(boost::beast::zlib::z_params& p,
             boost::beast::zlib::Flush flush,
             boost::system::error_code& ec)
  {
    to_native(p);
    ec = from_native_error(::inflate(&*stream_, to_native(flush)));
    from_native(p);
  }
};

class gzip_deflate_stream : public gzip_stream_base {
public:
  gzip_deflate_stream()
  {
    auto s = std::make_unique<z_stream>();
    s->zalloc = Z_NULL;
    s->zfree = Z_NULL;
    s->opaque = Z_NULL;
    if (auto e = ::deflateInit2(&*s,
                                Z_BEST_COMPRESSION,
                                Z_DEFLATED,
                                max_wbits | gzip_header,
                                9,
                                Z_DEFAULT_STRATEGY);
        e != Z_OK) {
      FITORIA_THROW_OR(std::system_error(from_native_error(e)),
                       std::terminate());
    }

    stream_ = std::move(s);
  }

  ~gzip_deflate_stream()
  {
    if (stream_) {
      ::deflateEnd(&*stream_);
    }
  }

  gzip_deflate_stream(const gzip_deflate_stream&) = delete;

  gzip_deflate_stream& operator=(const gzip_deflate_stream&) = delete;

  gzip_deflate_stream(gzip_deflate_stream&&) = default;

  gzip_deflate_stream& operator=(gzip_deflate_stream&&) = default;

  void write(boost::beast::zlib::z_params& p,
             boost::beast::zlib::Flush flush,
             boost::system::error_code& ec)
  {
    to_native(p);
    ec = from_native_error(::deflate(&*stream_, to_native(flush)));
    from_native(p);
  }
};

}

FITORIA_NAMESPACE_END

#endif

#endif
