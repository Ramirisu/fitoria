//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#ifndef FITORIA_WEB_MIDDLEWARE_DETAIL_ASYNC_BROTLI_DEFLATE_STREAM_HPP
#define FITORIA_WEB_MIDDLEWARE_DETAIL_ASYNC_BROTLI_DEFLATE_STREAM_HPP

#include <fitoria/core/config.hpp>

#if defined(FITORIA_HAS_BROTLI)

#include <fitoria/core/net.hpp>

#include <fitoria/web/detail/brotli_error.hpp>
#include <fitoria/web/detail/brotli_params.hpp>

#include <fitoria/web/async_readable_stream_concept.hpp>

#include <brotli/encode.h>

FITORIA_NAMESPACE_BEGIN

namespace web::middleware::detail {

enum class brotli_encoder_operation {
  process,
  flush,
  finish,
  emit_metadata,
};

class brotli_encoder {
public:
  brotli_encoder()
      : handle_(BrotliEncoderCreateInstance(nullptr, nullptr, nullptr))
  {
    if (handle_ == nullptr) {
      FITORIA_THROW_OR(
          std::system_error(make_error_code(web::detail::brotli_error::init)),
          std::terminate());
    }
  }

  ~brotli_encoder()
  {
    if (handle_) {
      BrotliEncoderDestroyInstance(handle_);
    }
  }

  brotli_encoder(const brotli_encoder&) = delete;

  brotli_encoder& operator=(const brotli_encoder&) = delete;

  brotli_encoder(brotli_encoder&& other)
      : handle_(std::exchange(other.handle_, nullptr))
  {
  }

  brotli_encoder& operator=(brotli_encoder&& other)
  {
    if (this != &other) {
      std::swap(handle_, other.handle_);
    }

    return *this;
  }

  auto write(web::detail::broti_params& p,
             brotli_encoder_operation op) noexcept -> std::error_code
  {
    return from_native_error(BrotliEncoderCompressStream(handle_,
                                                         to_native(op),
                                                         &p.avail_in,
                                                         &p.next_in,
                                                         &p.avail_out,
                                                         &p.next_out,
                                                         nullptr));
  }

  auto is_done() const noexcept -> bool
  {
    return BrotliEncoderIsFinished(handle_) == BROTLI_TRUE;
  }

private:
  auto from_native_error(BROTLI_BOOL result) -> std::error_code
  {
    if (result) {
      return {};
    }

    return make_error_code(web::detail::brotli_error::error);
  }

  auto to_native(brotli_encoder_operation op) -> BrotliEncoderOperation
  {
    switch (op) {
    case brotli_encoder_operation::process:
      return BROTLI_OPERATION_PROCESS;
    case brotli_encoder_operation::flush:
      return BROTLI_OPERATION_FLUSH;
    case brotli_encoder_operation::finish:
      return BROTLI_OPERATION_FINISH;
    case brotli_encoder_operation::emit_metadata:
      return BROTLI_OPERATION_EMIT_METADATA;
    default:
      break;
    }

    std::terminate();
  }

  BrotliEncoderState* handle_ = nullptr;
};

template <async_readable_stream NextLayer>
class async_brotli_deflate_stream {
  enum state : std::uint32_t {
    none = 0,
    input_is_not_eof = 1,
    need_more_output_buffer = 2,
  };

public:
  using is_async_readable_stream = void;

  template <async_readable_stream NextLayer2>
  async_brotli_deflate_stream(NextLayer2&& next)
      : next_(std::forward<NextLayer2>(next))
  {
  }

  auto async_read_some(net::mutable_buffer buffer)
      -> awaitable<expected<std::size_t, std::error_code>>
  {
    FITORIA_ASSERT(buffer.size() > 0);

    if (state_ == none) {
      co_return unexpected { make_error_code(net::error::eof) };
    }

    if (state_ & input_is_not_eof) {
      auto size
          = co_await next_.async_read_some(buffer_.prepare(buffer.size()));
      if (size) {
        buffer_.commit(*size);
      } else if (size.error() == make_error_code(net::error::eof)) {
        state_ &= ~input_is_not_eof;
      } else {
        co_return unexpected { size.error() };
      }
    }

    auto p = web::detail::broti_params();
    p.next_in = reinterpret_cast<const std::uint8_t*>(buffer_.cdata().data());
    p.avail_in = buffer_.cdata().size();
    p.next_out = reinterpret_cast<std::uint8_t*>(buffer.data());
    p.avail_out = buffer.size();

    if (state_ & input_is_not_eof) {
      if (auto ec = deflater_.write(p, brotli_encoder_operation::process); ec) {
        co_return unexpected { ec };
      }
    } else {
      if (auto ec = deflater_.write(p, brotli_encoder_operation::finish); ec) {
        co_return unexpected { ec };
      }

      if (deflater_.is_done()) {
        state_ &= ~need_more_output_buffer;
      } else {
        state_ |= need_more_output_buffer;
      }
    }

    buffer_.consume(buffer_.size() - p.avail_in);
    co_return buffer.size() - p.avail_out;
  }

private:
  NextLayer next_;
  brotli_encoder deflater_;
  flat_buffer buffer_;
  std::uint32_t state_ = input_is_not_eof;
};

template <typename NextLayer>
async_brotli_deflate_stream(NextLayer&&)
    -> async_brotli_deflate_stream<std::decay_t<NextLayer>>;

}

FITORIA_NAMESPACE_END

#endif

#endif
