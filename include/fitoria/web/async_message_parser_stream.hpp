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

template <typename Stream, typename MessageParser>
class async_message_parser_stream {
  net::flat_buffer buffer_;
  std::shared_ptr<Stream> stream_;
  std::unique_ptr<MessageParser> parser_;
  std::chrono::milliseconds timeout_;

public:
  async_message_parser_stream(net::flat_buffer buffer,
                              std::shared_ptr<Stream> stream,
                              std::unique_ptr<MessageParser> parser,
                              std::chrono::milliseconds timeout)
      : buffer_(std::move(buffer))
      , stream_(std::move(stream))
      , parser_(std::move(parser))
      , timeout_(timeout)
  {
  }

  auto size_hint() const noexcept -> optional<std::size_t>
  {
    if (auto length = parser_->content_length(); length) {
      return *length;
    }

    return nullopt;
  }

  auto async_read_next() -> net::awaitable<
      optional<expected<std::vector<std::byte>, net::error_code>>>
  {
    if (parser_->is_done()) {
      co_return nullopt;
    }

    std::vector<std::byte> buf;
    buf.resize(1024);
    parser_->get().body().data = buf.data();
    parser_->get().body().size = buf.size();

    net::get_lowest_layer(*stream_).expires_after(timeout_);
    auto [ec, size]
        = co_await boost::beast::http::async_read(*stream_, buffer_, *parser_);
    if (!ec || ec == http::error::need_buffer) {
      buf.resize(size);
      co_return buf;
    }
    co_return unexpected { ec };
  }
};

}

FITORIA_NAMESPACE_END

#endif
