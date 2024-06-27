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

#include <fitoria/core/chrono.hpp>
#include <fitoria/core/expected.hpp>
#include <fitoria/core/net.hpp>

#include <fitoria/http.hpp>

#include <fitoria/web/async_readable_file_stream.hpp>
#include <fitoria/web/request.hpp>
#include <fitoria/web/to_response.hpp>

#include <filesystem>

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
  /// Get ``Content-Type`` that will be used when converting to ``response``.
  ///
  /// @endverbatim
  auto content_type() const noexcept -> const mime::mime_view&
  {
    return content_type_;
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Set the ``Content-Type`` header that will be used when converting to
  /// ``response``.
  ///
  /// @endverbatim
  void set_content_type(mime::mime_view mime) noexcept
  {
    content_type_ = mime;
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Get ``Content-Disposition`` that will be used when converting to
  /// ``response``.
  ///
  /// @endverbatim
  auto content_disposition() const noexcept -> const std::string&
  {
    return content_disposition_;
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
    auto file = open_file(ex, path);
    if (!file) {
      return unexpected { file.error() };
    }

    auto lmd = get_last_modified_time(path);
    if (!lmd) {
      return unexpected { lmd.error() };
    }

    auto ct = mime::mime_view::from_path(path).value_or(
        mime::application_octet_stream());
    auto cd = get_content_disposition(path, ct);

    return static_file(std::move(*file), ct, cd, full_range_only_t {}, *lmd);
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
    auto file = open_file(ex, path);
    if (!file) {
      return unexpected { file.error() };
    }

    auto lmd = get_last_modified_time(path);
    if (!lmd) {
      return unexpected { lmd.error() };
    }

    auto ct = mime::mime_view::from_path(path).value_or(
        mime::application_octet_stream());
    auto cd = get_content_disposition(path, ct);

    if (auto header = req.headers().get(http::field::range); header) {
      if (auto range = http::header::range::parse(*header, file->size()); range
          && cmp_eq_ci(range->unit(), "bytes")
          && (*range)[0].offset + (*range)[0].length <= file->size()) {
        return static_file(std::move(*file), ct, cd, (*range)[0], *lmd);
      }

      return static_file(
          std::move(*file), ct, cd, range_not_satisfiable_t {}, *lmd);
    }

    return static_file(std::move(*file), ct, cd, full_range_t {}, *lmd);
  }

  template <decay_to<static_file> Self>
  friend auto tag_invoke(to_response_t, Self&& self) -> response
  {
    return std::visit(
        overloaded {
            [&](full_range_only_t) {
              return response::ok()
                  .set_header(http::field::last_modified,
                              self.last_modified_time())
                  .set_header(http::field::content_type, self.content_type())
                  .set_header(http::field::content_disposition,
                              self.content_disposition_)
                  .set_stream_body(async_readable_file_stream(self.release()));
            },
            [&](full_range_t) {
              return response::ok()
                  .set_header(http::field::content_type, self.content_type())
                  .set_header(http::field::content_disposition,
                              self.content_disposition_)
                  .set_header(http::field::last_modified,
                              self.last_modified_time())
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
                  .set_header(http::field::last_modified,
                              self.last_modified_time())
                  .set_header(http::field::content_type, self.content_type())
                  .set_header(http::field::content_disposition,
                              self.content_disposition_)
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
  static_file(
      stream_file file,
      mime::mime_view content_type,
      std::string content_disposition,
      range_t range,
      std::chrono::time_point<std::chrono::file_clock> last_modified_time)
      : file_(std::move(file))
      , content_type_(std::move(content_type))
      , content_disposition_(std::move(content_disposition))
      , range_(range)
      , last_modified_time_(last_modified_time)
  {
  }

  static auto open_file(const executor_type& ex, const std::string& path)
      -> expected<stream_file, std::error_code>
  {
    auto file = stream_file(ex);

    boost::system::error_code ec;
    file.open(path, net::file_base::read_only, ec); // NOLINT
    if (ec) {
      return unexpected { ec };
    }

    return file;
  }

  static auto get_last_modified_time(const std::string& path)
      -> expected<std::chrono::time_point<std::chrono::file_clock>,
                  std::error_code>
  {
    std::error_code ec;
    if (auto time = std::filesystem::last_write_time(path, ec); !ec) {
      return time;
    }

    return unexpected { ec };
  }

  static auto get_content_disposition(const std::string& path,
                                      const mime::mime_view& ct) -> std::string
  {
    if (ct.type() == "audio" || ct.type() == "image" || ct.type() == "text"
        || ct.type() == "video" || ct == mime::application_javascript()
        || ct == mime::application_json() || ct == mime::application_wasm()) {
      return fmt::format("inline");
    }

    auto p = std::filesystem::path(path);
    return fmt::format(R"(attachment; filename="{}")", p.filename().string());
  }

  auto last_modified_time() const -> std::string
  {
    return http::header::last_modified(chrono::to_utc(last_modified_time_));
  }

  stream_file file_;
  mime::mime_view content_type_ = mime::application_octet_stream();
  std::string content_disposition_;
  range_t range_;
  std::chrono::time_point<std::chrono::file_clock> last_modified_time_;
};

#endif

}

FITORIA_NAMESPACE_END

#endif
