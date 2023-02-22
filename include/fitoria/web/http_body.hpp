//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef FITORIA_WEB_HTTP_BODY_HPP
#define FITORIA_WEB_HTTP_BODY_HPP

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
namespace request_body {

  class body_base {
  public:
    virtual ~body_base() = default;
    virtual auto read_next()
        -> lazy<expected<std::vector<std::byte>, net::error_code>>
        = 0;
    virtual auto read_all()
        -> lazy<expected<std::vector<std::byte>, net::error_code>>
        = 0;
  };

  class vector_body : public body_base {
  public:
    vector_body(std::vector<std::byte> data)
        : data_(std::move(data))
    {
    }

    template <typename T, std::size_t N>
    vector_body(std::span<T, N> s)
        : data_(std::vector<std::byte>(std::as_bytes(s).begin(),
                                       std::as_bytes(s).end()))
    {
    }

    ~vector_body() override = default;

    auto read_next()
        -> lazy<expected<std::vector<std::byte>, net::error_code>> override
    {
      if (data_) {
        auto data = std::move(*data_);
        data_.reset();
        co_return data;
      }

      co_return unexpected { net::error_code(http::error::end_of_stream) };
    }

    auto read_all()
        -> lazy<expected<std::vector<std::byte>, net::error_code>> override
    {
      return read_next();
    }

  private:
    optional<std::vector<std::byte>> data_;
  };

  template <typename Stream>
  class chunk_body : public body_base {
    using request_parser_type = boost::beast::http::request_parser<
        boost::beast::http::vector_body<std::byte>>;

  public:
    chunk_body(Stream& stream,
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

    ~chunk_body() override = default;

    auto read_next()
        -> lazy<expected<std::vector<std::byte>, net::error_code>> override
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

    auto read_all()
        -> lazy<expected<std::vector<std::byte>, net::error_code>> override
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
}

class http_request_body {
public:
  http_request_body(const http_request_body&) = delete;
  http_request_body& operator=(const http_request_body&) = delete;
  http_request_body(http_request_body&&) = default;
  http_request_body& operator=(http_request_body&&) = default;

  auto read_next() -> lazy<expected<std::vector<std::byte>, net::error_code>>
  {
    return body_->read_next();
  }

  auto read_all() -> lazy<expected<std::vector<std::byte>, net::error_code>>
  {
    return body_->read_all();
  }

  static auto new_vector_body(std::vector<std::byte> data) -> http_request_body
  {
    return http_request_body(
        std::make_shared<request_body::vector_body>(std::move(data)));
  }

  template <typename T, std::size_t N>
  static auto new_vector_body(std::span<T, N> s) -> http_request_body
  {
    return http_request_body(std::make_shared<request_body::vector_body>(s));
  }

  template <typename Stream>
  static auto
  new_chunk_body(Stream& stream,
                 boost::beast::http::request_parser<
                     boost::beast::http::vector_body<std::byte>>& req_parser,
                 net::flat_buffer& buffer,
                 std::vector<std::byte>& chunk,
                 std::chrono::milliseconds timeout) -> http_request_body
  {
    return http_request_body(std::make_shared<request_body::chunk_body<Stream>>(
        stream, req_parser, buffer, chunk, timeout));
  }

private:
  http_request_body(std::shared_ptr<request_body::body_base> body)
      : body_(std::move(body))
  {
  }

  // GCC 12 bug?
  // using unique_ptr causes segmentation fault while destuction
  std::shared_ptr<request_body::body_base> body_;
};

}

FITORIA_NAMESPACE_END

#endif
