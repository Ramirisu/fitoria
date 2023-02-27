//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef FITORIA_WEB_ASYNC_MESSAGE_PARSER_STREAM_HPP
#define FITORIA_WEB_ASYNC_MESSAGE_PARSER_STREAM_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/core/expected.hpp>
#include <fitoria/core/lazy.hpp>
#include <fitoria/core/net.hpp>

#include <fitoria/web/http/http.hpp>

#include <cstddef>
#include <vector>

FITORIA_NAMESPACE_BEGIN

namespace web {

template <typename Stream, typename MessageParser>
class async_message_parser_stream {

public:
  async_message_parser_stream(Stream& stream,
                              MessageParser& msg_parser,
                              net::flat_buffer& buffer,
                              std::vector<std::byte>& chunk,
                              std::chrono::milliseconds timeout)
      : stream_(stream)
      , msg_parser_(msg_parser)
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

    if (msg_parser_.is_done()) {
      co_return nullopt;
    }

    net::error_code ec;

    while (!msg_parser_.is_done() && !ec) {
      net::get_lowest_layer(stream_).expires_after(timeout_);
      tie(ec, _) = co_await boost::beast::http::async_read(
          stream_, buffer_, msg_parser_);
    }

    if (ec && ec != http::error::end_of_chunk) {
      co_return unexpected { ec };
    }

    co_return std::move(chunk_);
  }

private:
  Stream& stream_;
  MessageParser& msg_parser_;
  net::flat_buffer& buffer_;
  std::vector<std::byte>& chunk_;
  std::chrono::milliseconds timeout_;
};

}

FITORIA_NAMESPACE_END

#endif
