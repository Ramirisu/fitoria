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

#include <fitoria/core/dynamic_buffer.hpp>
#include <fitoria/core/expected.hpp>
#include <fitoria/core/net.hpp>

#include <fitoria/http/verb.hpp>

#include <fitoria/web/async_readable_stream_concept.hpp>

FITORIA_NAMESPACE_BEGIN

namespace web {

template <typename Stream, typename Parser>
class async_message_parser_stream {
  static_assert(std::is_move_constructible_v<Stream>);

public:
  using is_async_readable_stream = void;

  async_message_parser_stream(std::shared_ptr<flat_buffer> buffer,
                              Stream stream,
                              std::shared_ptr<Parser> parser)
      : buffer_(std::move(buffer))
      , stream_(std::move(stream))
      , parser_(std::move(parser))
  {
  }

  async_message_parser_stream(const async_message_parser_stream&) = delete;

  async_message_parser_stream& operator=(const async_message_parser_stream&)
      = delete;

  async_message_parser_stream(async_message_parser_stream&&) = default;

  async_message_parser_stream& operator=(async_message_parser_stream&&)
      = default;

  auto
  async_read_some() -> awaitable<optional<expected<bytes, std::error_code>>>
  {
    using boost::beast::http::async_read;

    if (parser_->is_done()) {
      co_return nullopt;
    }

    dynamic_buffer<bytes> buffer;
    boost::system::error_code ec;
    for (auto writable = buffer.prepare(65536);;
         writable = buffer.prepare(65536)) {
      parser_->get().body().data = writable.data();
      parser_->get().body().size = writable.size();

      auto size
          = co_await async_read(stream_, *buffer_, *parser_, use_awaitable);
      if (!size && size.error() != boost::beast::http::error::need_buffer) {
        ec = size.error();
        break;
      }

      const auto remaining = parser_->get().body().size;
      buffer.commit(writable.size() - remaining);
      if (remaining > 0) {
        break;
      }
    }

    if (ec) {
      co_return unexpected { ec };
    }
    co_return buffer.release();
  }

private:
  std::shared_ptr<flat_buffer> buffer_;
  Stream stream_;
  std::shared_ptr<Parser> parser_;
};

}

FITORIA_NAMESPACE_END

#endif
