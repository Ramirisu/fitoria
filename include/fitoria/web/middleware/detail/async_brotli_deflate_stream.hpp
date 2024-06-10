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

#include <fitoria/web/middleware/detail/brotli_error.hpp>
#include <fitoria/web/middleware/detail/brotli_params.hpp>

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
      FITORIA_THROW_OR(std::system_error(make_error_code(brotli_error::init)),
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

  auto write(broti_params& p,
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

    return make_error_code(brotli_error::error);
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
public:
  using is_async_readable_stream = void;

  template <async_readable_stream NextLayer2>
  async_brotli_deflate_stream(NextLayer2&& next)
      : next_(std::forward<NextLayer2>(next))
  {
  }

  auto
  async_read_some() -> awaitable<optional<expected<bytes, std::error_code>>>
  {
    auto data = co_await next_.async_read_some();
    if (!data) {
      if (finish_) {
        co_return nullopt;
      }

      finish_ = true;

      auto buffer = dynamic_buffer<bytes>();
      auto writable = buffer.prepare(65536);

      auto p = broti_params(nullptr, 0, writable.data(), writable.size());

      auto ec = deflater_.write(p, brotli_encoder_operation::finish);

      FITORIA_ASSERT(ec != brotli_error::need_more_output);
      if (ec) {
        co_return unexpected { ec };
      }

      buffer.commit(writable.size() - p.avail_out);
      co_return buffer.release();
    }

    auto& readable = *data;
    if (!readable) {
      co_return unexpected { readable.error() };
    }
    if (readable->empty()) {
      co_return bytes();
    }

    auto buffer = dynamic_buffer<bytes>();
    auto writable
        = buffer.prepare(std::max(readable->size(), std::size_t(65536)));

    auto p = broti_params(
        readable->data(), readable->size(), writable.data(), writable.size());

    auto ec = deflater_.write(p, brotli_encoder_operation::process);

    FITORIA_ASSERT(ec != brotli_error::need_more_output);
    if (ec) {
      co_return unexpected { ec };
    }

    buffer.commit(writable.size() - p.avail_out);
    co_return buffer.release();
  }

private:
  NextLayer next_;
  brotli_encoder deflater_;
  bool finish_ = false;
};

template <typename NextLayer>
async_brotli_deflate_stream(NextLayer&&)
    -> async_brotli_deflate_stream<std::decay_t<NextLayer>>;

}

FITORIA_NAMESPACE_END

#endif

#endif
