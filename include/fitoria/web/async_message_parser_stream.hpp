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
                              std::unique_ptr<parser_t> parser)
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

  auto size_hint() const noexcept -> optional<std::size_t>
  {
    if (parser_->chunked()) {
      return nullopt;
    }

    return parser_->content_length_remaining().value_or(0);
  }

  auto async_read_some(net::mutable_buffer buffer)
      -> awaitable<expected<std::size_t, std::error_code>>
  {
    using boost::beast::get_lowest_layer;
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

    auto [ec, _]
        = co_await async_read(*stream_, buffer_, *parser_, use_awaitable);
    if (!ec || ec == http::error::need_buffer) {
      const auto remaining = parser_->get().body().size;
      co_return buffer.size() - remaining;
    }

    co_return unexpected { ec };
  }

private:
  boost::beast::flat_buffer buffer_;
  std::shared_ptr<Stream> stream_;
  std::unique_ptr<parser_t> parser_;
  bool return_0_at_first_call_;
};

}

FITORIA_NAMESPACE_END

#endif
