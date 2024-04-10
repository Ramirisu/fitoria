//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#ifndef FITORIA_WEB_MIDDLEWARE_DETAIL_ASYNC_BROTLI_INFLATE_STREAM_HPP
#define FITORIA_WEB_MIDDLEWARE_DETAIL_ASYNC_BROTLI_INFLATE_STREAM_HPP

#include <fitoria/core/config.hpp>

#if defined(FITORIA_HAS_BROTLI)

#include <fitoria/core/net.hpp>

#include <fitoria/web/detail/brotli_error.hpp>
#include <fitoria/web/detail/brotli_params.hpp>

#include <fitoria/web/async_readable_stream_concept.hpp>

#include <brotli/decode.h>

FITORIA_NAMESPACE_BEGIN

namespace web::middleware::detail {

class brotli_decoder {
public:
  brotli_decoder()
      : handle_(BrotliDecoderCreateInstance(nullptr, nullptr, nullptr))
  {
    if (handle_ == 0) {
      FITORIA_THROW_OR(
          std::system_error(make_error_code(web::detail::brotli_error::init)),
          std::terminate());
    }
  }

  ~brotli_decoder()
  {
    if (handle_) {
      BrotliDecoderDestroyInstance(handle_);
    }
  }

  brotli_decoder(const brotli_decoder&) = delete;

  brotli_decoder& operator=(const brotli_decoder&) = delete;

  brotli_decoder(brotli_decoder&& other)
      : handle_(std::exchange(other.handle_, nullptr))
  {
  }

  brotli_decoder& operator=(brotli_decoder&& other)
  {
    if (this != &other) {
      std::swap(handle_, other.handle_);
    }

    return *this;
  }

  auto write(web::detail::broti_params& p) -> std::error_code
  {
    return from_native_error(BrotliDecoderDecompressStream(
        handle_, &p.avail_in, &p.next_in, &p.avail_out, &p.next_out, nullptr));
  }

private:
  auto from_native_error(BrotliDecoderResult result) -> std::error_code
  {
    switch (result) {
    case BROTLI_DECODER_RESULT_ERROR:
      return make_error_code(web::detail::brotli_error::error);
    case BROTLI_DECODER_RESULT_SUCCESS:
      return {};
    case BROTLI_DECODER_RESULT_NEEDS_MORE_INPUT:
      return make_error_code(web::detail::brotli_error::need_more_input);
    case BROTLI_DECODER_RESULT_NEEDS_MORE_OUTPUT:
      return make_error_code(web::detail::brotli_error::need_more_output);
    default:
      break;
    }

    return make_error_code(web::detail::brotli_error::unknown);
  }

  BrotliDecoderState* handle_ = nullptr;
};

template <async_readable_stream NextLayer>
class async_brotli_inflate_stream {
  enum state : std::uint32_t {
    none = 0,
    need_more_input_buffer = 1,
    need_more_output_buffer = 2,
  };

public:
  using is_async_readable_stream = void;

  template <async_readable_stream NextLayer2>
  async_brotli_inflate_stream(NextLayer2&& next)
      : next_(std::forward<NextLayer2>(next))
  {
  }

  auto is_sized() const noexcept -> bool
  {
    return false;
  }

  auto async_read_some(net::mutable_buffer buffer)
      -> awaitable<expected<std::size_t, std::error_code>>
  {
    FITORIA_ASSERT(buffer.size() > 0);

    if (state_ == none) {
      co_return unexpected { make_error_code(net::error::eof) };
    }

    if (state_ & need_more_input_buffer) {
      auto size
          = co_await next_.async_read_some(buffer_.prepare(buffer.size()));
      if (size) {
        buffer_.commit(*size);
      } else {
        co_return unexpected { size.error() };
      }
    }

    auto p = web::detail::broti_params();
    p.next_in = reinterpret_cast<const std::uint8_t*>(buffer_.cdata().data());
    p.avail_in = buffer_.cdata().size();
    p.next_out = reinterpret_cast<std::uint8_t*>(buffer.data());
    p.avail_out = buffer.size();

    auto ec = inflater_.write(p);

    if (ec) {
      if (ec == web::detail::brotli_error::need_more_input) {
        state_ |= need_more_input_buffer;
      } else if (ec == web::detail::brotli_error::need_more_output) {
        state_ |= need_more_output_buffer;
      } else {
        co_return unexpected { ec };
      }
    } else {
      state_ &= ~need_more_input_buffer;
      state_ &= ~need_more_output_buffer;
    }

    buffer_.consume(buffer_.size() - p.avail_in);
    co_return buffer.size() - p.avail_out;
  }

private:
  NextLayer next_;
  brotli_decoder inflater_;
  boost::beast::flat_buffer buffer_;
  std::uint32_t state_ = need_more_input_buffer;
};

template <typename NextLayer>
async_brotli_inflate_stream(NextLayer&&)
    -> async_brotli_inflate_stream<std::decay_t<NextLayer>>;

}

FITORIA_NAMESPACE_END

#endif

#endif
