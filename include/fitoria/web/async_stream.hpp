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
  { t.size_hint() } -> std::same_as<optional<std::size_t>>;
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

  auto size_hint() const noexcept -> optional<std::size_t>
  {
    return data_.transform([](auto& data) { return data.size(); });
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

class any_async_readable_stream {
  class base {
  public:
    virtual ~base() = default;
    virtual auto is_chunked() const noexcept -> bool = 0;
    virtual auto size_hint() const noexcept -> optional<std::size_t> = 0;
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

    auto is_chunked() const noexcept -> bool override
    {
      return stream_.is_chunked();
    }

    auto size_hint() const noexcept -> optional<std::size_t> override
    {
      return stream_.size_hint();
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

  any_async_readable_stream(const any_async_readable_stream&) = delete;

  any_async_readable_stream& operator=(const any_async_readable_stream&)
      = delete;

  any_async_readable_stream(any_async_readable_stream&&) = default;

  any_async_readable_stream& operator=(any_async_readable_stream&&) = default;

  auto is_chunked() const noexcept -> bool
  {
    return stream_->is_chunked();
  }

  auto size_hint() const noexcept -> optional<std::size_t>
  {
    return stream_->size_hint();
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
    std::copy(next_chunk.value()->begin(),
              next_chunk.value()->end(),
              std::as_writable_bytes(std::span(container)).begin() + offset);
    next_chunk = co_await stream.async_read_next();
  } while (next_chunk);

  co_return container;
}

template <typename AsyncWriteStream, async_readable_stream AsyncReadableStream>
auto async_write_each_chunk(AsyncWriteStream&& to,
                            AsyncReadableStream&& from,
                            std::chrono::milliseconds timeout)
    -> lazy<expected<void, net::error_code>>
{
  using boost::beast::http::make_chunk;
  using boost::beast::http::make_chunk_last;
  auto _ = std::ignore;

  net::error_code ec;

  auto chunk = co_await from.async_read_next();
  while (chunk) {
    if (!*chunk) {
      co_return unexpected { (*chunk).error() };
    }
    net::get_lowest_layer(to).expires_after(timeout);
    tie(ec, _) = co_await async_write(
        to, make_chunk(net::const_buffer((*chunk)->data(), (*chunk)->size())));
    if (ec) {
      co_return unexpected { ec };
    }
    chunk = co_await from.async_read_next();
  }
  tie(ec, _) = co_await async_write(to, make_chunk_last());
  if (ec) {
    co_return unexpected { ec };
  }

  co_return expected<void, net::error_code>();
}
}

FITORIA_NAMESPACE_END

#endif
