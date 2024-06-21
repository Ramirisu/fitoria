//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#ifndef FITORIA_WEB_STATIC_FILE_HPP
#define FITORIA_WEB_STATIC_FILE_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/core/expected.hpp>
#include <fitoria/core/net.hpp>

#include <fitoria/http.hpp>

#include <fitoria/web/async_readable_file_stream.hpp>
#include <fitoria/web/request.hpp>
#include <fitoria/web/to_response.hpp>

FITORIA_NAMESPACE_BEGIN

namespace web {

#if defined(BOOST_ASIO_HAS_FILE)

/// @verbatim embed:rst:leading-slashes
///
/// Provides a wrapper for handling static file response.
///
/// @endverbatim
class static_file {
  struct full_range_only_t { };
  struct full_range_t { };
  struct range_not_satisfiable_t { };

  using range_t = std::variant<full_range_only_t,
                               full_range_t,
                               range_not_satisfiable_t,
                               http::header::range::subrange_t>;

public:
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
  /// Get the reference to the underlying ``stream_file``.
  ///
  /// @endverbatim
  auto file() const noexcept -> const stream_file&
  {
    return file_;
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Get the reference to the underlying ``stream_file``.
  ///
  /// @endverbatim
  auto file() noexcept -> stream_file&
  {
    return file_;
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Release the underlying ``stream_file`` object.
  ///
  /// @endverbatim
  auto release() -> stream_file
  {
    return std::move(file_);
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Open file with read-only permission.
  ///
  /// DESCRIPTION
  ///   Open file with read-only permission. ``Content-Type`` header for
  ///   the response will be obtained from the provided file extension. Note
  ///   that ``Range`` header from the ``request`` will not be handled here, use
  ///   another overload that accepts ``request`` as the parameter to handle the
  ///   header automatically.
  ///
  /// @endverbatim
  static auto open(const executor_type& ex, const std::string& path)
      -> expected<static_file, std::error_code>
  {
    auto file = stream_file(ex);

    boost::system::error_code ec;
    file.open(path, net::file_base::read_only, ec); // NOLINT
    if (ec) {
      return unexpected { ec };
    }

    return static_file(std::move(file),
                       mime::mime_view::from_path(path).value_or(
                           mime::application_octet_stream()),
                       full_range_only_t {});
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Open file with read-only permission.
  ///
  /// DESCRIPTION
  ///   Open file with read-only permission. ``Range`` header from the
  ///   ``request`` will be handled automatically and ``Content-Type`` header
  ///   for the response will be obtained from the provided file extension.
  ///
  /// @endverbatim
  static auto open(const executor_type& ex,
                   const std::string& path,
                   const request& req) -> expected<static_file, std::error_code>
  {
    auto file = stream_file(ex);

    boost::system::error_code ec;
    file.open(path, net::file_base::read_only, ec); // NOLINT
    if (ec) {
      return unexpected { ec };
    }

    if (auto header = req.headers().get(http::field::range); header) {
      if (auto range = http::header::range::parse(*header, file.size()); range
          && iequals(range->unit(), "bytes")
          && (*range)[0].offset + (*range)[0].length <= file.size()) {
        return static_file(std::move(file),
                           mime::mime_view::from_path(path).value_or(
                               mime::application_octet_stream()),
                           (*range)[0]);
      }

      return static_file(std::move(file),
                         mime::mime_view::from_path(path).value_or(
                             mime::application_octet_stream()),
                         range_not_satisfiable_t {});
    }

    return static_file(std::move(file),
                       mime::mime_view::from_path(path).value_or(
                           mime::application_octet_stream()),
                       full_range_t {});
  }

  template <decay_to<static_file> Self>
  friend auto tag_invoke(to_response_t, Self&& self) -> response
  {
    return std::visit(
        overloaded {
            [&](full_range_only_t) {
              return response::ok()
                  .set_header(http::field::content_type, self.content_type())
                  .set_stream_body(async_readable_file_stream(self.release()));
            },
            [&](full_range_t) {
              return response::ok()
                  .set_header(http::field::content_type, self.content_type())
                  .set_header(http::field::accept_ranges, "bytes")
                  .set_stream_body(async_readable_file_stream(self.release()));
            },
            [&](range_not_satisfiable_t) {
              return response::range_not_satisfiable()
                  .set_header(http::field::accept_ranges, "bytes")
                  .set_header(http::field::content_range,
                              fmt::format("bytes */{}", self.file().size()))
                  .build();
            },
            [&](http::header::range::subrange_t range) {
              return response::partial_content()
                  .set_header(http::field::content_type, self.content_type())
                  .set_header(http::field::accept_ranges, "bytes")
                  .set_header(http::field::content_range,
                              fmt::format("bytes {}-{}/{}",
                                          range.offset,
                                          range.offset + range.length - 1,
                                          range.length))
                  .set_stream_body(async_readable_file_stream(
                      self.release(), range.offset, range.length));
            },
        },
        self.range_);
  }

private:
  static_file(stream_file file, mime::mime_view content_type, range_t range)
      : file_(std::move(file))
      , content_type_(std::move(content_type))
      , range_(range)
  {
  }

  stream_file file_;
  mime::mime_view content_type_ = mime::application_octet_stream();
  range_t range_;
};

#endif

}

FITORIA_NAMESPACE_END

#endif
