//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#ifndef FITORIA_WEB_STREAM_FILE_HPP
#define FITORIA_WEB_STREAM_FILE_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/core/net.hpp>

#include <fitoria/web/async_readable_file_stream.hpp>
#include <fitoria/web/to_response.hpp>

FITORIA_NAMESPACE_BEGIN

namespace web {

#if defined(BOOST_ASIO_HAS_FILE)

/// @verbatim embed:rst:leading-slashes
///
/// Provides a stream file object that supports asynchronous read.
///
/// @endverbatim
class stream_file {
  net::stream_file file_;

public:
  stream_file(net::stream_file file)
      : file_(std::move(file))
  {
  }
  auto release() -> net::stream_file
  {
    return std::move(file_);
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Open file with read-only permission.
  ///
  /// @endverbatim
  static auto open_readonly(const executor_type& ex, const std::string& path)
      -> expected<stream_file, std::error_code>
  {
    auto file = net::stream_file(ex);

    boost::system::error_code ec;
    file.open(path, net::file_base::read_only, ec); // NOLINT
    if (ec) {
      return unexpected { ec };
    }

    return stream_file(std::move(file));
  }

  template <decay_to<stream_file> Self>
  friend auto tag_invoke(to_response_t, Self&& self) -> response
  {
    return response::ok()
        .set_header(http::field::content_type, mime::application_octet_stream())
        .set_stream_body(async_readable_file_stream(self.release()));
  }
};

#endif

}

FITORIA_NAMESPACE_END

#endif
