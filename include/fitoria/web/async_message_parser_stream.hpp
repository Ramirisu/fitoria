//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#ifndef FITORIA_WEB_ASYNC_MESSAGE_PARSER_STREAM_HPP
#define FITORIA_WEB_ASYNC_MESSAGE_PARSER_STREAM_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/core/expected.hpp>
#include <fitoria/core/net.hpp>

#include <fitoria/web/http/http.hpp>

#include <cstddef>
#include <vector>

FITORIA_NAMESPACE_BEGIN

namespace web {

template <typename Stream, bool isRequest, typename Body, typename Allocator>
class async_message_parser_stream {
public:
  using is_async_readable_stream = void;

  using parser_t = boost::beast::http::parser<isRequest, Body, Allocator>;

  async_message_parser_stream(boost::beast::flat_buffer buffer,
                              std::shared_ptr<Stream> stream,
                              std::unique_ptr<parser_t> parser,
                              std::chrono::milliseconds timeout)
      : buffer_(std::move(buffer))
      , stream_(std::move(stream))
      , parser_(std::move(parser))
      , timeout_(timeout)
  {
  }

  async_message_parser_stream(const async_message_parser_stream&) = delete;

  async_message_parser_stream& operator=(const async_message_parser_stream&)
      = delete;

  async_message_parser_stream(async_message_parser_stream&&) = default;

  async_message_parser_stream& operator=(async_message_parser_stream&&)
      = default;

  auto size_hint() const noexcept -> optional<std::size_t>
  {
    if (auto length = parser_->content_length(); length) {
      return *length;
    }

    return nullopt;
  }

  auto async_read_next() -> net::awaitable<
      optional<expected<std::vector<std::byte>, std::error_code>>>
  {
    using boost::beast::get_lowest_layer;
    using boost::beast::http::async_read;

    if (parser_->is_done()) {
      co_return nullopt;
    }

    boost::system::error_code ec;

    // TODO: buffer size configurable ?
    const std::size_t bufsize = 1024;
    std::vector<std::byte> buf(bufsize);
    parser_->get().body().data = buf.data();
    parser_->get().body().size = buf.size();

    get_lowest_layer(*stream_).expires_after(timeout_);
    std::tie(ec, std::ignore)
        = co_await async_read(*stream_, buffer_, *parser_, net::use_ta);
    const auto remaining = parser_->get().body().size;
    if (!ec || ec == http::error::need_buffer) {
      buf.resize(bufsize - remaining);
      co_return buf;
    }
    co_return unexpected { ec };
  }

private:
  boost::beast::flat_buffer buffer_;
  std::shared_ptr<Stream> stream_;
  std::unique_ptr<parser_t> parser_;
  std::chrono::milliseconds timeout_;
};

}

FITORIA_NAMESPACE_END

#endif
