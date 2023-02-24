//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef FITORIA_WEB_ASYNC_STREAM_HPP
#define FITORIA_WEB_ASYNC_STREAM_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/core/error.hpp>
#include <fitoria/core/expected.hpp>
#include <fitoria/core/lazy.hpp>
#include <fitoria/core/net.hpp>

#include <fitoria/web/http/http.hpp>

#include <algorithm>
#include <cstddef>
#include <span>
#include <vector>

FITORIA_NAMESPACE_BEGIN

namespace web {

// clang-format off
template <typename T>
concept async_readable_stream = requires(T t) {
  { t.is_chunked() } -> std::same_as<bool>;
  { t.async_read_next() } 
    -> std::same_as<lazy<optional<expected<std::vector<std::byte>, net::error_code>>>>;
};
// clang-format on

class async_readable_vector_stream {
public:
  async_readable_vector_stream() = default;

  async_readable_vector_stream(std::vector<std::byte> data)
      : data_(std::move(data))
  {
  }

  template <typename T, std::size_t N>
  async_readable_vector_stream(std::span<T, N> s)
      : data_(std::vector<std::byte>(std::as_bytes(s).begin(),
                                     std::as_bytes(s).end()))
  {
  }

  auto is_chunked() const noexcept -> bool
  {
    return false;
  }

  auto async_read_next()
      -> lazy<optional<expected<std::vector<std::byte>, net::error_code>>>
  {
    co_return data_.and_then(
        [this](auto& data)
            -> optional<expected<std::vector<std::byte>, net::error_code>> {
          auto moved = std::move(data);
          data_.reset();
          return moved;
        });
  }

private:
  optional<std::vector<std::byte>> data_;
};

template <typename Stream>
class async_readable_chunk_stream {
  using request_parser_type = boost::beast::http::request_parser<
      boost::beast::http::vector_body<std::byte>>;

public:
  async_readable_chunk_stream(Stream& stream,
                              request_parser_type& req_parser,
                              net::flat_buffer& buffer,
                              std::vector<std::byte>& chunk,
                              std::chrono::milliseconds timeout)
      : stream_(stream)
      , req_parser_(req_parser)
      , buffer_(buffer)
      , chunk_(chunk)
      , timeout_(timeout)
  {
  }

  auto is_chunked() const noexcept -> bool
  {
    return true;
  }

  auto async_read_next()
      -> lazy<optional<expected<std::vector<std::byte>, net::error_code>>>
  {
    using std::tie;
    auto _ = std::ignore;

    if (req_parser_.is_done()) {
      co_return nullopt;
    }

    net::error_code ec;

    while (!req_parser_.is_done() && !ec) {
      net::get_lowest_layer(stream_).expires_after(timeout_);
      tie(ec, _) = co_await boost::beast::http::async_read(stream_, buffer_,
                                                           req_parser_);
    }

    if (ec && ec != http::error::end_of_chunk) {
      co_return unexpected { ec };
    }

    co_return std::move(chunk_);
  }

private:
  Stream& stream_;
  request_parser_type& req_parser_;
  net::flat_buffer& buffer_;
  std::vector<std::byte>& chunk_;
  std::chrono::milliseconds timeout_;
};

class any_async_readable_stream {
  class base {
  public:
    virtual ~base() = default;
    virtual auto clone() const -> std::shared_ptr<base> = 0;
    virtual auto is_chunked() const noexcept -> bool = 0;
    virtual auto async_read_next()
        -> lazy<optional<expected<std::vector<std::byte>, net::error_code>>>
        = 0;
  };

  template <typename AsyncReadableStream>
  class derived : public base {
  public:
    derived(AsyncReadableStream stream)
        : stream_(std::move(stream))
    {
    }

    auto clone() const -> std::shared_ptr<base> override
    {
      return std::make_shared<derived>(*this);
    }

    auto is_chunked() const noexcept -> bool override
    {
      return stream_.is_chunked();
    }

    auto async_read_next() -> lazy<
        optional<expected<std::vector<std::byte>, net::error_code>>> override
    {
      return stream_.async_read_next();
    }

  private:
    AsyncReadableStream stream_;
  };

public:
  template <async_readable_stream AsyncReadableStream>
  any_async_readable_stream(AsyncReadableStream&& stream)
    requires(!uncvref_same_as<AsyncReadableStream, any_async_readable_stream>)
      : stream_(std::make_shared<derived<std::decay_t<AsyncReadableStream>>>(
          std::forward<AsyncReadableStream>(stream)))
  {
  }

  any_async_readable_stream(const any_async_readable_stream& other)
      : stream_(other.stream_->clone())
  {
  }

  any_async_readable_stream& operator=(const any_async_readable_stream& other)
  {
    if (this != std::addressof(other)) {
      stream_ = other.stream_->clone();
    }

    return *this;
  }

  any_async_readable_stream(any_async_readable_stream&&) = default;

  any_async_readable_stream& operator=(any_async_readable_stream&&) = default;

  auto is_chunked() const noexcept -> bool
  {
    return stream_->is_chunked();
  }

  auto async_read_next()
      -> lazy<optional<expected<std::vector<std::byte>, net::error_code>>>
  {
    return stream_->async_read_next();
  }

private:
  // TODO:
  // GCC 12 bug?
  // using unique_ptr causes segmentation fault during destruction
  std::shared_ptr<base> stream_;
};

template <typename Container, async_readable_stream AsyncReadableStream>
auto async_read_all(AsyncReadableStream&& stream)
    -> lazy<optional<expected<Container, net::error_code>>>
{
  Container container;

  auto next_chunk = co_await stream.async_read_next();
  if (!next_chunk) {
    co_return nullopt;
  }

  do {
    if (!next_chunk.value()) {
      co_return unexpected { next_chunk.value().error() };
    }
    const auto offset = container.size();
    container.resize(offset + next_chunk.value()->size());
    std::copy(next_chunk.value()->begin(), next_chunk.value()->end(),
              std::as_writable_bytes(std::span(container)).begin() + offset);
    next_chunk = co_await stream.async_read_next();
  } while (next_chunk);

  co_return container;
}

}

FITORIA_NAMESPACE_END

#endif
