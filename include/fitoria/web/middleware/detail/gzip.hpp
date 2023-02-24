//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef FITORIA_WEB_MIDDLEWARE_DETAIL_GZIP_HPP
#define FITORIA_WEB_MIDDLEWARE_DETAIL_GZIP_HPP

#if defined(FITORIA_HAS_ZLIB)

#include <fitoria/core/config.hpp>

#include <fitoria/core/error.hpp>
#include <fitoria/core/expected.hpp>
#include <fitoria/core/net.hpp>

#include <memory>

#include <zlib.h>

FITORIA_NAMESPACE_BEGIN

namespace web::middleware::detail {

net::zlib::error to_net_zlib_error(int zlib_error)
{
  switch (zlib_error) {
  case Z_STREAM_END:
    return net::zlib::error::end_of_stream;
  case Z_NEED_DICT:
    return net::zlib::error::need_dict;
  case Z_STREAM_ERROR:
    return net::zlib::error::stream_error;
  case Z_BUF_ERROR:
    return net::zlib::error::need_buffers;
  case Z_ERRNO:
  case Z_DATA_ERROR:
  case Z_MEM_ERROR:
  case Z_VERSION_ERROR:
  default:
    return net::zlib::error::general;
  }
}

template <typename R>
expected<R, error_code> gzip_decompress(net::const_buffer in)
{
  if (in.size() == 0) {
    return unexpected { make_error_code(net::zlib::error::stream_error) };
  }

  // set a min buffer size to avoid too many memory reallocations
  // when input size is much smaller than output size
  static constexpr std::size_t min_buff_size = 256;

  static const auto gzip_flag = 16;
  z_stream stream {};

  stream.zalloc = Z_NULL;
  stream.zfree = Z_NULL;
  stream.opaque = Z_NULL;
  stream.next_in = static_cast<unsigned char*>(const_cast<void*>(in.data()));
  stream.avail_in = static_cast<unsigned int>(in.size());
  if (auto err = ::inflateInit2(&stream, 15 + gzip_flag); err < 0) {
    ::inflateEnd(&stream);
    return unexpected { make_error_code(to_net_zlib_error(err)) };
  }

  R out;
  out.resize(std::max(in.size(), min_buff_size));
  stream.next_out = reinterpret_cast<unsigned char*>(out.data());
  stream.avail_out = static_cast<unsigned int>(out.size());
  for (;;) {
    auto err = ::inflate(&stream, Z_SYNC_FLUSH);
    if (err != Z_OK && err != Z_STREAM_END && err != Z_BUF_ERROR) {
      ::inflateEnd(&stream);
      return unexpected { make_error_code(to_net_zlib_error(err)) };
    }

    if (stream.avail_out > 0) {
      break;
    }

    out.resize(2 * stream.total_out);
    stream.next_out
        = reinterpret_cast<unsigned char*>(out.data() + stream.total_out);
    stream.avail_out = stream.total_out;
  }

  out.resize(stream.total_out);
  ::inflateEnd(&stream);
  return out;
}

template <typename R>
expected<R, error_code> gzip_compress(net::const_buffer in)
{
  if (in.size() == 0) {
    return unexpected { make_error_code(net::zlib::error::stream_error) };
  }

  static const auto gzip_flag = 16;
  z_stream stream {};

  stream.zalloc = Z_NULL;
  stream.zfree = Z_NULL;
  stream.opaque = Z_NULL;
  stream.next_in = static_cast<unsigned char*>(const_cast<void*>(in.data()));
  stream.avail_in = static_cast<unsigned int>(in.size());
  if (auto err = ::deflateInit2(&stream,
                                Z_BEST_COMPRESSION,
                                Z_DEFLATED,
                                15 + gzip_flag,
                                9,
                                Z_DEFAULT_STRATEGY);
      err < 0) {
    ::deflateEnd(&stream);
    return unexpected { make_error_code(to_net_zlib_error(err)) };
  }

  R out;
  out.resize(in.size());
  stream.next_out = reinterpret_cast<unsigned char*>(out.data());
  stream.avail_out = static_cast<unsigned int>(out.size());
  for (;;) {
    auto err = ::deflate(&stream, Z_SYNC_FLUSH);
    if (err != Z_OK && err != Z_STREAM_END && err != Z_BUF_ERROR) {
      ::deflateEnd(&stream);
      return unexpected { make_error_code(to_net_zlib_error(err)) };
    }

    if (stream.avail_out > 0) {
      break;
    }

    out.resize(2 * stream.total_out);
    stream.next_out
        = reinterpret_cast<unsigned char*>(out.data() + stream.total_out);
    stream.avail_out = stream.total_out;
  }

  out.resize(stream.total_out);
  ::deflateEnd(&stream);
  return out;
}
}

FITORIA_NAMESPACE_END

#endif

#endif
