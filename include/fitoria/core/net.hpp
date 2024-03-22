//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#ifndef FITORIA_CORE_NET_HPP
#define FITORIA_CORE_NET_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/core/detail/boost.hpp>

#include <fitoria/core/type_traits.hpp>

FITORIA_NAMESPACE_BEGIN

namespace net {

using namespace boost::asio;

using boost::beast::error_code;

constexpr auto use_ta = as_tuple(use_awaitable);

using ws_tcp_stream = boost::beast::websocket::stream<boost::beast::tcp_stream>;

template <typename Stream, bool HasSslCtx = false>
class basic_shared_stream {
  std::shared_ptr<Stream> stream_;

public:
  template <typename Arg>
    requires(!uncvref_same_as<Arg, basic_shared_stream>)
  basic_shared_stream(Arg&& arg)
      : stream_(std::make_shared<Stream>(std::forward<Arg>(arg)))
  {
  }

  auto operator*() const noexcept -> const Stream&
  {
    return *stream_;
  }

  auto operator*() noexcept -> Stream&
  {
    return *stream_;
  }

  auto operator->() const noexcept -> const Stream*
  {
    return stream_.get();
  }

  auto operator->() noexcept -> Stream*
  {
    return stream_.get();
  }
};

using shared_tcp_stream = basic_shared_stream<boost::beast::tcp_stream, false>;
using shared_ws_tcp_stream = basic_shared_stream<ws_tcp_stream, false>;

#if defined(FITORIA_HAS_OPENSSL)
using ssl_stream = boost::beast::ssl_stream<boost::beast::tcp_stream>;
using ws_ssl_stream = boost::beast::websocket::stream<ssl_stream>;

template <typename Stream>
class basic_shared_stream<Stream, true> {
  std::shared_ptr<ssl::context> ssl_ctx_;
  std::shared_ptr<Stream> stream_;

public:
  template <typename Arg>
  basic_shared_stream(Arg&& arg, std::shared_ptr<ssl::context> ssl_ctx)
      : ssl_ctx_(std::move(ssl_ctx))
      , stream_(std::make_shared<Stream>(std::forward<Arg>(arg), *ssl_ctx_))
  {
  }

  auto operator*() const noexcept -> const Stream&
  {
    return *stream_;
  }

  auto operator*() noexcept -> Stream&
  {
    return *stream_;
  }

  auto operator->() const noexcept -> const Stream*
  {
    return stream_.get();
  }

  auto operator->() noexcept -> Stream*
  {
    return stream_.get();
  }
};

using shared_ssl_stream = basic_shared_stream<ssl_stream, true>;
using shared_ws_ssl_stream = basic_shared_stream<ws_ssl_stream, true>;

#endif

template <typename Stream>
auto get_local_endpoint(Stream& stream)
{
  return boost::beast::get_lowest_layer(stream).socket().local_endpoint();
}

template <typename Stream>
auto get_remote_endpoint(Stream& stream)
{
  return boost::beast::get_lowest_layer(stream).socket().remote_endpoint();
}

template <typename Awaitable>
void sync_wait(Awaitable&& awaitable)
{
  io_context ioc;
  auto fut = co_spawn(ioc, std::forward<Awaitable>(awaitable), use_future);
  ioc.run();
  (void)fut.get();
}

}

FITORIA_NAMESPACE_END

#endif
