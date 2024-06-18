//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#ifndef FITORIA_WEB_NAMED_FILE_HPP
#define FITORIA_WEB_NAMED_FILE_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/core/expected.hpp>
#include <fitoria/core/net.hpp>

#include <fitoria/web/async_readable_file_stream.hpp>
#include <fitoria/web/to_response.hpp>

FITORIA_NAMESPACE_BEGIN

namespace web {

#if defined(BOOST_ASIO_HAS_FILE)

/// @verbatim embed:rst:leading-slashes
///
/// Provides a stream file wrapper for response.
///
/// @endverbatim
class named_file {
  net::stream_file file_;
  mime::mime_view content_type_ = mime::application_octet_stream();

public:
  named_file(net::stream_file file, mime::mime_view content_type)
      : file_(std::move(file))
      , content_type_(std::move(content_type))
  {
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Get ``Content-Type`` that will be used when converting to the
  /// ``response``.
  ///
  /// @endverbatim
  auto content_type() const noexcept -> const mime::mime_view&
  {
    return content_type_;
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Set the ``Content-Type`` header that will be used when converting to the
  /// ``response``.
  ///
  /// @endverbatim
  void set_content_type(mime::mime_view mime) noexcept
  {
    content_type_ = mime;
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Release the underlying ``net::stream_file`` object.
  ///
  /// @endverbatim
  auto release() -> net::stream_file
  {
    return std::move(file_);
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Open file with read-only permission.
  ///
  /// DESCRIPTION
  ///   Open file with read-only permission. ``Content-Type`` header will be
  ///   obtained from the file extension.
  ///
  /// @endverbatim
  static auto open_readonly(const executor_type& ex, const std::string& path)
      -> expected<named_file, std::error_code>
  {
    auto file = net::stream_file(ex);

    boost::system::error_code ec;
    file.open(path, net::file_base::read_only, ec); // NOLINT
    if (ec) {
      return unexpected { ec };
    }

    return named_file(std::move(file),
                      mime::mime_view::from_path(path).value_or(
                          mime::application_octet_stream()));
  }

  template <decay_to<named_file> Self>
  friend auto tag_invoke(to_response_t, Self&& self) -> response
  {
    return response::ok()
        .set_header(http::field::content_type, self.content_type())
        .set_stream_body(async_readable_file_stream(self.release()));
  }
};

#endif

}

FITORIA_NAMESPACE_END

#endif
