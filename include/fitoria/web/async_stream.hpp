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

#include <cstddef>
#include <span>
#include <vector>

FITORIA_NAMESPACE_BEGIN

namespace web {

// clang-format off
template <typename T>
concept async_readable_stream = requires(T t) {
  { t.read_next() } -> std::same_as<lazy<expected<std::vector<std::byte>, net::error_code>>>;
  { t.read_all() } -> std::same_as<lazy<expected<std::vector<std::byte>, net::error_code>>>;
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

  auto read_next() -> lazy<expected<std::vector<std::byte>, net::error_code>>
  {
    if (data_) {
      auto data = std::move(*data_);
      data_.reset();
      co_return data;
    }

    co_return unexpected { net::error_code(http::error::end_of_stream) };
  }

  auto read_all() -> lazy<expected<std::vector<std::byte>, net::error_code>>
  {
    return read_next();
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

  auto read_next() -> lazy<expected<std::vector<std::byte>, net::error_code>>
  {
    using std::tie;
    auto _ = std::ignore;

    if (req_parser_.is_done()) {
      co_return unexpected { make_error_code(http::error::end_of_stream) };
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

  auto read_all() -> lazy<expected<std::vector<std::byte>, net::error_code>>
  {
    auto chunk = co_await read_next();
    if (!chunk) {
      co_return chunk;
    }

    std::vector<std::byte> data;
    while (chunk) {
      data.insert(data.end(), chunk->begin(), chunk->end());
      chunk = co_await read_next();
    }

    if (!chunk && chunk.error() != http::error::end_of_stream) {
      co_return chunk;
    }

    co_return data;
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
    virtual auto read_next()
        -> lazy<expected<std::vector<std::byte>, net::error_code>>
        = 0;
    virtual auto read_all()
        -> lazy<expected<std::vector<std::byte>, net::error_code>>
        = 0;
  };

  template <typename StreamBody>
  class derived : public base {
  public:
    derived(StreamBody body)
        : stream_(std::move(body))
    {
    }

    auto read_next()
        -> lazy<expected<std::vector<std::byte>, net::error_code>> override
    {
      return stream_.read_next();
    }

    auto read_all()
        -> lazy<expected<std::vector<std::byte>, net::error_code>> override
    {
      return stream_.read_all();
    }

  private:
    StreamBody stream_;
  };

public:
  template <async_readable_stream AsyncReadableStream>
  any_async_readable_stream(AsyncReadableStream body)
    requires(!uncvref_same_as<AsyncReadableStream, any_async_readable_stream>)
      : stream_(std::make_shared<derived<AsyncReadableStream>>(std::move(body)))
  {
  }

  any_async_readable_stream(const any_async_readable_stream&) = delete;
  any_async_readable_stream& operator=(const any_async_readable_stream&)
      = delete;
  any_async_readable_stream(any_async_readable_stream&&) = default;
  any_async_readable_stream& operator=(any_async_readable_stream&&) = default;

  auto read_next() -> lazy<expected<std::vector<std::byte>, net::error_code>>
  {
    return stream_->read_next();
  }

  auto read_all() -> lazy<expected<std::vector<std::byte>, net::error_code>>
  {
    return stream_->read_all();
  }

private:
  // TODO:
  // GCC 12 bug?
  // using unique_ptr causes segmentation fault during destruction
  std::shared_ptr<base> stream_;
};
}

FITORIA_NAMESPACE_END

#endif
