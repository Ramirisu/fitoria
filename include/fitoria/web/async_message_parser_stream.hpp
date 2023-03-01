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
#include <functional>
#include <span>
#include <vector>

FITORIA_NAMESPACE_BEGIN

namespace web {

template <typename Stream, typename MessageParser>
class async_message_parser_stream {

  class impl {
  public:
    impl(std::unique_ptr<MessageParser> parser, net::flat_buffer buffer)
        : parser_(std::move(parser))
        , buffer_(std::move(buffer))
        , on_chunk_header_(std::bind_front(&impl::on_chunk_header, this))
        , on_chunk_body_(std::bind_front(&impl::on_chunk_body, this))
    {
      parser_->on_chunk_header(on_chunk_header_);
      parser_->on_chunk_body(on_chunk_body_);
    }

    impl(const impl&) = delete;

    impl& operator=(const impl&) = delete;

    impl(impl&&) = delete;

    impl& operator=(impl&&) = delete;

    MessageParser& parser() noexcept
    {
      return *parser_;
    }

    net::flat_buffer& buffer() noexcept
    {
      return buffer_;
    }

    std::vector<std::byte>& chunk() noexcept
    {
      return chunk_;
    }

  private:
    void on_chunk_header(std::uint64_t size,
                         boost::core::string_view,
                         net::error_code&)
    {
      chunk_.reserve(size);
      chunk_.clear();
    }

    std::size_t on_chunk_body(std::uint64_t remain,
                              boost::core::string_view body,
                              net::error_code& ec)
    {
      if (remain == body.size()) {
        ec = http::error::end_of_chunk;
      }
      auto s = std::as_bytes(std::span(body.data(), body.size()));
      chunk_.insert(chunk_.end(), s.begin(), s.end());

      return body.size();
    }

    std::unique_ptr<MessageParser> parser_;
    net::flat_buffer buffer_;
    std::vector<std::byte> chunk_;
    std::function<void(
        std::uint64_t, boost::core::string_view, net::error_code&)>
        on_chunk_header_;
    std::function<std::size_t(
        std::uint64_t, boost::core::string_view, net::error_code&)>
        on_chunk_body_;
  };

public:
  async_message_parser_stream(std::shared_ptr<Stream> stream,
                              std::unique_ptr<MessageParser> parser,
                              net::flat_buffer buffer,
                              std::chrono::milliseconds timeout)
      : stream_(stream)
      , impl_(std::make_unique<impl>(std::move(parser), std::move(buffer)))
      , timeout_(timeout)

  {
  }

  auto is_chunked() const noexcept -> bool
  {
    return true;
  }

  auto size_hint() const noexcept -> optional<std::size_t>
  {
    if (auto length = impl_->parser().content_length(); length) {
      return *length;
    }

    return nullopt;
  }

  auto async_read_next()
      -> lazy<optional<expected<std::vector<std::byte>, net::error_code>>>
  {
    using std::tie;
    auto _ = std::ignore;

    if (impl_->parser().is_done()) {
      co_return nullopt;
    }

    net::error_code ec;

    while (!impl_->parser().is_done() && !ec) {
      net::get_lowest_layer(*stream_).expires_after(timeout_);
      tie(ec, _) = co_await boost::beast::http::async_read(
          *stream_, impl_->buffer(), impl_->parser());
    }

    if (ec && ec != http::error::end_of_chunk) {
      co_return unexpected { ec };
    }

    co_return std::move(impl_->chunk());
  }

private:
  std::shared_ptr<Stream> stream_;
  std::unique_ptr<impl> impl_;
  std::chrono::milliseconds timeout_;
};

}

FITORIA_NAMESPACE_END

#endif
