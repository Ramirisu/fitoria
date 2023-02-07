//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef FITORIA_WEB_MIDDLEWARE_DETAIL_DEFLATE_HPP
#define FITORIA_WEB_MIDDLEWARE_DETAIL_DEFLATE_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/core/error.hpp>
#include <fitoria/core/expected.hpp>
#include <fitoria/core/net.hpp>

FITORIA_NAMESPACE_BEGIN

namespace web::middleware::detail {

template <typename R>
expected<R, error_code> deflate_decompress(net::const_buffer in)
{
  if (in.size() == 0) {
    return unexpected { make_error_code(net::zlib::error::stream_error) };
  }

  // set a min buffer size to avoid too many memory reallocations
  // when input size is much smaller than output size
  static constexpr std::size_t min_buff_size = 256;

  R out;
  out.resize(std::max(in.size(), min_buff_size));

  net::zlib::z_params p;
  p.next_in = in.data();
  p.avail_in = in.size();
  p.next_out = out.data();
  p.avail_out = out.size();
  net::zlib::inflate_stream stream;
  for (;;) {
    net::error_code ec;
    stream.write(p, net::zlib::Flush::sync, ec);
    if (ec == net::zlib::error::end_of_stream) {
      break;
    }
    if (ec && ec != net::zlib::error::need_buffers) {
      return unexpected { ec };
    }
    if (p.avail_out == out.size()) {
      // special case to handle invalid input stream
      return unexpected { make_error_code(net::zlib::error::stream_error) };
    }
    if (p.avail_out > 0) {
      break;
    }
    out.resize(2 * p.total_out);
    p.next_out = out.data() + p.total_out;
    p.avail_out = p.total_out;
  }
  out.resize(p.total_out);

  return out;
}

template <typename R>
expected<R, error_code> deflate_compress(net::const_buffer in)
{
  if (in.size() == 0) {
    return unexpected { make_error_code(net::zlib::error::stream_error) };
  }

  R out;
  out.resize(net::zlib::deflate_upper_bound(in.size()));

  net::zlib::z_params p;
  p.next_in = in.data();
  p.avail_in = in.size();
  p.next_out = out.data();
  p.avail_out = out.size();
  net::zlib::deflate_stream stream;
  net::error_code ec;
  stream.write(p, net::zlib::Flush::full, ec);
  if (ec && ec != net::zlib::error::end_of_stream) {
    return unexpected { ec };
  }
  out.resize(p.total_out);

  return out;
}
}

FITORIA_NAMESPACE_END

#endif
