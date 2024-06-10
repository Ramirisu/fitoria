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

#include <fitoria/web/middleware/detail/brotli_error.hpp>
#include <fitoria/web/middleware/detail/brotli_params.hpp>

#include <fitoria/web/async_readable_stream_concept.hpp>

#include <brotli/decode.h>

FITORIA_NAMESPACE_BEGIN

namespace web::middleware::detail {

class brotli_decoder {
public:
  brotli_decoder()
      : handle_(BrotliDecoderCreateInstance(nullptr, nullptr, nullptr))
  {
    if (handle_ == nullptr) {
      FITORIA_THROW_OR(std::system_error(make_error_code(brotli_error::init)),
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

  auto write(broti_params& p) -> std::error_code
  {
    return from_native_error(BrotliDecoderDecompressStream(
        handle_, &p.avail_in, &p.next_in, &p.avail_out, &p.next_out, nullptr));
  }

private:
  auto from_native_error(BrotliDecoderResult result) -> std::error_code
  {
    switch (result) {
    case BROTLI_DECODER_RESULT_ERROR:
      return make_error_code(brotli_error::error);
    case BROTLI_DECODER_RESULT_SUCCESS:
      return {};
    case BROTLI_DECODER_RESULT_NEEDS_MORE_INPUT:
      return make_error_code(brotli_error::need_more_input);
    case BROTLI_DECODER_RESULT_NEEDS_MORE_OUTPUT:
      return make_error_code(brotli_error::need_more_output);
    default:
      break;
    }

    return make_error_code(brotli_error::unknown);
  }

  BrotliDecoderState* handle_ = nullptr;
};

template <async_readable_stream NextLayer>
class async_brotli_inflate_stream {
public:
  using is_async_readable_stream = void;

  template <async_readable_stream NextLayer2>
  async_brotli_inflate_stream(NextLayer2&& next)
      : next_(std::forward<NextLayer2>(next))
  {
  }

  auto
  async_read_some() -> awaitable<optional<expected<bytes, std::error_code>>>
  {
    auto data = co_await next_.async_read_some();
    if (!data) {
      co_return nullopt;
    }

    if (!*data) {
      co_return unexpected { data->error() };
    }
    if ((*data)->empty()) {
      co_return bytes();
    }

    auto readable = dynamic_buffer<bytes>(std::move(**data));

    auto buffer = dynamic_buffer<bytes>();

    for (;;) {
      auto writable
          = buffer.prepare(std::max(readable.size(), std::size_t(65536)));

      auto p = broti_params(readable.cdata().data(),
                            readable.cdata().size(),
                            writable.data(),
                            writable.size());

      auto ec = inflater_.write(p);
      if (ec == brotli_error::need_more_input
          || ec == brotli_error::need_more_output) {
        ec = {};
      }
      if (ec) {
        co_return unexpected { ec };
      }

      readable.consume(readable.size() - p.avail_in);
      buffer.commit(writable.size() - p.avail_out);

      if (readable.size() == 0 && p.avail_out > 0) {
        break;
      }
    }

    co_return buffer.release();
  }

private:
  NextLayer next_;
  brotli_decoder inflater_;
};

template <typename NextLayer>
async_brotli_inflate_stream(NextLayer&&)
    -> async_brotli_inflate_stream<std::decay_t<NextLayer>>;

}

FITORIA_NAMESPACE_END

#endif

#endif
