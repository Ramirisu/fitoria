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

FITORIA_NAMESPACE_BEGIN

namespace web {

template <typename Stream, typename Parser>
class async_message_parser_stream {
  static_assert(std::is_move_constructible_v<Stream>);

public:
  using is_async_readable_stream = void;

  async_message_parser_stream(flat_buffer buffer,
                              Stream stream,
                              std::shared_ptr<Parser> parser)
      : buffer_(std::move(buffer))
      , stream_(std::move(stream))
      , parser_(std::move(parser))
      , return_0_at_first_call_(parser_->chunked() || parser_->content_length())
  {
  }

  async_message_parser_stream(const async_message_parser_stream&) = delete;

  async_message_parser_stream& operator=(const async_message_parser_stream&)
      = delete;

  async_message_parser_stream(async_message_parser_stream&&) = default;

  async_message_parser_stream& operator=(async_message_parser_stream&&)
      = default;

  auto async_read_some(net::mutable_buffer buffer)
      -> awaitable<expected<std::size_t, std::error_code>>
  {
    using boost::beast::http::async_read;

    if (parser_->is_done()) {
      // must return 0 for the first call to this function for following cases:
      //   1. chunked encoding with size of 0
      //   2. content-length is presented and with value of 0
      if (return_0_at_first_call_) {
        return_0_at_first_call_ = false;
        co_return std::size_t(0);
      }

      co_return unexpected { make_error_code(net::error::eof) };
    }
    return_0_at_first_call_ = false;

    parser_->get().body().data = buffer.data();
    parser_->get().body().size = buffer.size();

    auto bytes_read
        = co_await async_read(stream_, buffer_, *parser_, use_awaitable);
    if (!bytes_read && bytes_read.error() != http::error::need_buffer) {
      co_return unexpected { bytes_read.error() };
    }

    const auto remaining = parser_->get().body().size;
    co_return buffer.size() - remaining;
  }

private:
  flat_buffer buffer_;
  Stream stream_;
  std::shared_ptr<Parser> parser_;
  bool return_0_at_first_call_;
};

}

FITORIA_NAMESPACE_END

#endif
